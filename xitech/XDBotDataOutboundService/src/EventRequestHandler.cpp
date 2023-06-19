//
// EventRequestHandler.cpp
//
// Copyright (c) 2017, Applied Informatics Software Engineering GmbH.
// All rights reserved.
//
// SPDX-License-Identifier: GPL-3.0-only
//


#include "EventRequestHandler.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/HTMLForm.h"
#include "Poco/OSP/Web/WebSession.h"
#include "Poco/OSP/Web/WebSessionManager.h"
#include "Poco/OSP/ServiceRegistry.h"
#include "Poco/OSP/ServiceFinder.h"
#include "Poco/OSP/ServiceRef.h"
#include "Poco/OSP/PreferencesService.h"
#include "Poco/OSP/Auth/AuthService.h"
#include "Poco/Util/PropertyFileConfiguration.h"
#include "Poco/Util/JSONConfiguration.h"
#include "Poco/Util/Application.h"
#include "Poco/StreamCopier.h"
#include "Poco/Format.h"
#include "Poco/AutoPtr.h"
#include "Poco/JSON/Object.h"
#include "Poco/JSON/Array.h"
#include "Poco/JSON/JSONException.h"
#include "Poco/JSON/Stringifier.h"
#include "Poco/JSON/Parser.h"
#include "Poco/Dynamic/Struct.h"
#include "Poco/Dynamic/Var.h"
#include "Poco/NotificationCenter.h"
#include "Poco/Observer.h"
#include "Poco/NObserver.h"
#include <sstream>
#include "Utility.h"
#include "Poco/Net/WebSocket.h"
#include "Poco/Net/NetException.h"
#include "NotificationsUtils.h"


namespace xi {
namespace XDBotDataOutboundService {

EventRequestHandler::EventRequestHandler(Poco::OSP::BundleContext::Ptr pContext)
	: _pContext(pContext)
	, _pWS(0)
	, _flags(0)
	, _sendPing(*this, &EventRequestHandler::onSendPing)
{
	_pPrefs = Poco::OSP::ServiceFinder::find<Poco::OSP::PreferencesService>(_pContext);

	Poco::NotificationCenter::defaultCenter().addObserver(Poco::Observer<EventRequestHandler, xi::utils::SensorMultipleAlarmNotification>(*this, &EventRequestHandler::onAlarm));

	Poco::NotificationCenter::defaultCenter().addObserver(Poco::Observer<EventRequestHandler, xi::utils::ActiveAlarmsNotification>(*this, &EventRequestHandler::onActiveAlarms));

	Poco::NotificationCenter::defaultCenter().addObserver(Poco::Observer<EventRequestHandler, xi::utils::SensorStatusNotification>(*this, &EventRequestHandler::onStatus));
}

EventRequestHandler::~EventRequestHandler()
{
	shutdown();
}

Poco::OSP::BundleContext::Ptr EventRequestHandler::context() const
{
	return _pContext;
}

void EventRequestHandler::shutdown()
{
	Poco::NotificationCenter::defaultCenter().removeObserver(Poco::Observer<EventRequestHandler, xi::utils::SensorMultipleAlarmNotification>(*this, &EventRequestHandler::onAlarm));

	Poco::NotificationCenter::defaultCenter().removeObserver(Poco::Observer<EventRequestHandler, xi::utils::ActiveAlarmsNotification>(*this, &EventRequestHandler::onActiveAlarms));

	Poco::NotificationCenter::defaultCenter().removeObserver(Poco::Observer<EventRequestHandler, xi::utils::SensorStatusNotification>(*this, &EventRequestHandler::onStatus));

	_queue.wakeUpAll();
}

void EventRequestHandler::send(const std::string& buffer)
{
	//return;
	if (!_pWS || (_flags & Poco::Net::WebSocket::FRAME_OP_BITMASK) == Poco::Net::WebSocket::FRAME_OP_CLOSE) {
		return;
	}
	try {
		if (_pWS) {
			int32_t n = 0;
			int32_t ret = 0;
			do {
				ret = _pWS->sendFrame(buffer.c_str(), buffer.length(), Poco::Net::WebSocket::FRAME_TEXT);
				n += ret;
			} while(ret > 0 && n < buffer.length());
		}
	}
	catch (Poco::Net::WebSocketException& exc) {
		_pContext->logger().log(exc);
		if (_pWS) {
			_pWS->close();
		}
	}
}

void EventRequestHandler::handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
{
	try {
		Poco::ThreadPool::defaultPool().start(*this);

		if (!_pingTimer) {
			_pingTimer = new Poco::Timer(1000, 1000*10);
			_pingTimer->start(_sendPing);
		}

		if (!_pWS){
			_pWS = std::make_shared<Poco::Net::WebSocket>(request, response);
		}

		Poco::NotificationCenter::defaultCenter().postNotification(new xi::utils::QueryActiveAlarmsNotification());

		char buffer[1024] = {0};
		int n = 0;
		do {
			n = _pWS->receiveFrame(buffer, sizeof(buffer), _flags);
			if ((_flags & Poco::Net::WebSocket::FRAME_OP_BITMASK) != Poco::Net::WebSocket::FRAME_OP_PING) {
				std::string msg(buffer, n);
				_pWS->sendFrame(buffer, n, _flags);
			}
			else if ((_flags & Poco::Net::WebSocket::FRAME_OP_BITMASK) != Poco::Net::WebSocket::FRAME_OP_TEXT) {

			}
		} while (n > 0 || (_flags & Poco::Net::WebSocket::FRAME_OP_BITMASK) != Poco::Net::WebSocket::FRAME_OP_CLOSE);

		_queue.wakeUpAll();

		std::cout << std::endl << "WebSocket connection established." << std::endl;
	}
	catch (Poco::Net::WebSocketException& exc) {
		_pContext->logger().log(exc);
		switch (exc.code())
		{
		case Poco::Net::WebSocket::WS_ERR_HANDSHAKE_UNSUPPORTED_VERSION:
			response.set("Sec-WebSocket-Version", Poco::Net::WebSocket::WEBSOCKET_VERSION);
			// fallthrough
		case Poco::Net::WebSocket::WS_ERR_NO_HANDSHAKE:
		case Poco::Net::WebSocket::WS_ERR_HANDSHAKE_NO_VERSION:
		case Poco::Net::WebSocket::WS_ERR_HANDSHAKE_NO_KEY:
			response.setStatusAndReason(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
			response.setContentLength(0);
			response.send();
			break;
		}
	}
}

void EventRequestHandler::onSendPing(Poco::Timer& timer)
{
	try {
		if (_pWS) {
			std::stringstream sstr;
			sstr << "ping: " << ++_seq;
			_pWS->sendFrame(sstr.str().c_str(), sstr.str().length(), Poco::Net::WebSocket::FRAME_TEXT |  Poco::Net::WebSocket::FRAME_OP_PING);
		}
	}
	catch (Poco::Net::WebSocketException& exc) {
		_pContext->logger().log(exc);
		if (_pWS) {
			_pWS->close();
		}
	}
}

void EventRequestHandler::run() 
{
	messagesLoop();
}

void EventRequestHandler::messagesLoop()
{
	try {
		Poco::AutoPtr<Poco::Notification> pNf = _queue.waitDequeueNotification();
		while (pNf) {
			if (auto pMsgNf = pNf.cast<xi::utils::MessageNotification>()) {
				std::string json(pMsgNf->data());
				// if (_pWS && (_flags & Poco::Net::WebSocket::FRAME_OP_BITMASK) != Poco::Net::WebSocket::FRAME_OP_CLOSE) {
				// 	_pWS->sendFrame(json.data(), static_cast<int>(json.size()), _flags);				
				// }
				this->send(json);
			}
			pNf = _queue.waitDequeueNotification();
		}
	}
	catch (Poco::Exception&) {

	}
}

void EventRequestHandler::processAlarmData(const std::vector<std::shared_ptr<AlarmData>>& vecData)
{
	Poco::JSON::Object messsage;
	messsage.set("event", std::string("alarm"));
	Poco::JSON::Array payload;
	for (auto data : vecData) {
		Poco::JSON::Object item;
		item.set("sensorId", data->sensorId);
		item.set("type", data->type);
		item.set("channel", data->channel);
		item.set("ruleId", data->ruleId);
		item.set("alarmType", data->alarmType);
		item.set("alarmLevel", data->alarmLevel);
		item.set("alarmRule", data->alarmRule);
		item.set("unit", data->unit);
		item.set("ruleValue", data->ruleValue);
		item.set("currValue", data->currValue);
		item.set("suddenChangeCycle", data->suddenChangeCycle);
		item.set("suddenChangeValue", data->suddenChangeValue);
		item.set("begin", (data->begin.timestamp().epochTime() - Poco::Timestamp::TimeDiff(60 * 60 * 8)));
		// item.set("end", data->end.timestamp().epochTime());
		item.set("status", data->status);
		payload.add(item);
	}
	messsage.set("payload", payload);
	std::ostringstream osstr;
	Poco::JSON::Stringifier::stringify(messsage, osstr);
	DetectionData data;
	_queue.enqueueNotification(new xi::utils::MessageNotification(osstr.str()));
}

void EventRequestHandler::onAlarm(xi::utils::SensorMultipleAlarmNotification* nf)
{
	std::vector<std::shared_ptr<AlarmData>> vecData = nf->data();
	if (vecData.empty()) {
		return;
	}

	processAlarmData(vecData);
}

void EventRequestHandler::onStatus(xi::utils::SensorStatusNotification* nf)
{
	std::vector<std::shared_ptr<SensorStatus>> statusList = nf->statusList();
	if (statusList.empty()) {
		return;
	}
	Poco::JSON::Object messsage;
	
	messsage.set("event", std::string("status"));
	Poco::JSON::Array payload;
	for (const auto& status : statusList) {
		Poco::JSON::Object item;
		item.set("sensorId", status->sensorId);
		item.set("status", status->status);
		payload.add(item);
	}
	messsage.set("payload", payload);

	std::ostringstream osstr;
	Poco::JSON::Stringifier::stringify(messsage, osstr);

	_queue.enqueueNotification(new xi::utils::MessageNotification(osstr.str()));
}

void EventRequestHandler::onActiveAlarms(xi::utils::ActiveAlarmsNotification* nf)
{
	if (!_requested) {
		_requested = true;

		std::vector<std::shared_ptr<AlarmData>> vecData = nf->data();
		if (vecData.empty()) {
			return;
		}

		processAlarmData(vecData);
	}
}

Poco::Net::HTTPRequestHandler* EventRequestHandlerFactory::createRequestHandler(const Poco::Net::HTTPServerRequest& request)
{
	return new EventRequestHandler(context());
}

} } // namespace xi::XDBotDataOutboundService
