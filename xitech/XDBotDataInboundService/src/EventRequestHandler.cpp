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
#include "Poco/JSON/Parser.h"
#include "Poco/JSON/ParseHandler.h"
#include "Poco/JSON/JSONException.h"
#include "Poco/JSON/Object.h"
#include "Poco/JSON/Array.h"
#include "Poco/JSON/Stringifier.h"
#include "Poco/StreamCopier.h"
#include "Poco/Dynamic/Struct.h"
#include "Poco/Dynamic/Var.h"
#include "Poco/DateTime.h"
#include "Poco/DateTimeFormat.h"
#include "Poco/Timezone.h"
#include "Poco/StringTokenizer.h"
#include "Poco/String.h"


namespace xi {
namespace XDBotDataInboundService {

EventRequestHandler::EventRequestHandler(Poco::OSP::BundleContext::Ptr pContext)
	: _pContext(pContext)
	, _pWS(0)
	, _flags(0)
	, _sendPing(*this, &EventRequestHandler::onSendPing)
{
	_pPrefs = Poco::OSP::ServiceFinder::find<Poco::OSP::PreferencesService>(_pContext);

	_scheduler.start();

	setupDatabase();
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
	if (_pSession) {
		_pSession->close();
	}
	
	_scheduler.stop();
}

void EventRequestHandler::send(const std::string& buffer)
{
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
		if (!_pingTimer) {
			_pingTimer = new Poco::Timer(1000, 1000*10);
			_pingTimer->start(_sendPing);
		}

		if (!_pWS){
			_pWS = std::make_shared<Poco::Net::WebSocket>(request, response);
		}

		char buffer[4096] = {0};
		int n = 0;
		do {

			// FRAME_OP_CONT    = 0x00, /// Continuation frame.
			// FRAME_OP_TEXT    = 0x01, /// Text frame.
			// FRAME_OP_BINARY  = 0x02, /// Binary frame.
			// FRAME_OP_CLOSE   = 0x08, /// Close connection.
			// FRAME_OP_PING    = 0x09, /// Ping frame.
			// FRAME_OP_PONG    = 0x0a, /// Pong frame.
			// FRAME_OP_BITMASK = 0x0f, /// Bit mask for opcodes.
			// FRAME_OP_SETRAW  = 0x100 /// Set raw flags (for use with sendBytes() and FRAME_OP_CONT).

			//if ((_flags & Poco::Net::WebSocket::FRAME_OP_BITMASK) == Poco::Net::WebSocket::FRAME_OP_CONT) {
			//	std::cout << "FRAME_OP_CONT" << std::endl;
			//} 
			//else if ((_flags & Poco::Net::WebSocket::FRAME_OP_BITMASK) == Poco::Net::WebSocket::FRAME_OP_TEXT) {
			//	std::cout << "FRAME_OP_TEXT" << std::endl;
			//}
			//else if ((_flags & Poco::Net::WebSocket::FRAME_OP_BITMASK) == Poco::Net::WebSocket::FRAME_OP_CLOSE) {
			//	std::cout << "FRAME_OP_CLOSE" << std::endl;
			//}
			//else if ((_flags & Poco::Net::WebSocket::FRAME_OP_BITMASK) == Poco::Net::WebSocket::FRAME_OP_PING) {
			//	std::cout << "FRAME_OP_PING" << std::endl;
			//}
			//else if ((_flags & Poco::Net::WebSocket::FRAME_OP_BITMASK) == Poco::Net::WebSocket::FRAME_OP_PONG) {
			//	std::cout << "FRAME_OP_PONG" << std::endl;
			//}
			//else if ((_flags & Poco::Net::WebSocket::FRAME_OP_BITMASK) == Poco::Net::WebSocket::FRAME_OP_SETRAW) {
			//	std::cout << "FRAME_OP_SETRAW" << std::endl;
			//}

			n = _pWS->receiveFrame(buffer, sizeof(buffer), _flags);
			//_pContext->logger().information(Poco::format("Frame received (length=%d, flags=0x%x).", n, unsigned(_flags)));
			if ((_flags & Poco::Net::WebSocket::FRAME_OP_BITMASK) == Poco::Net::WebSocket::FRAME_OP_TEXT) {
				std::string msg(buffer, n);
				onMessage(msg);
			}
		} while (n > 0 && (_flags & Poco::Net::WebSocket::FRAME_OP_BITMASK) != Poco::Net::WebSocket::FRAME_OP_CLOSE);
		_pContext->logger().information("websocket connection closed.");
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
			_pWS->sendFrame(sstr.str().c_str(), sstr.str().length(), Poco::Net::WebSocket::FRAME_TEXT | Poco::Net::WebSocket::FRAME_OP_PING);
		}
	}
	catch (Poco::Net::WebSocketException& exc) {
		_pContext->logger().log(exc);
		if (_pWS) {
			_pWS->close();
		}
	}
}

void EventRequestHandler::connectNoDB() 
{
	std::string dbConnString = "host=" + _dbHost + ";port=" + std::to_string(_dbPort) + ";user=" + _dbUser + ";password=" + _dbPassword + ";db=" + _dbName + ";compress=true;auto-reconnect=true;secure-auth=true;protocol=tcp";

	try {
		Session session(MySQL::Connector::KEY, dbConnString);

		_pContext->logger().information("*** Connected to [MySQL] without database.");

		session << "CREATE DATABASE IF NOT EXISTS " + _dbName + ";", now;

		_pContext->logger().information("Disconnecting ...");

		session.close();

		_pContext->logger().information("Disconnected");
	}
	catch (ConnectionFailedException& exc) {
		_pContext->logger().error(Poco::format("Connection failed exception: %s", exc.displayText()));
	}
}

void EventRequestHandler::setupDatabase() 
{
	MySQL::Connector::registerConnector();

	_dbUser = _pPrefs->configuration()->getString("xitech.xdbot.db.user", "");
	_dbPassword = _pPrefs->configuration()->getString("xitech.xdbot.db.password", "");
	_dbPort = _pPrefs->configuration()->getInt("xitech.xdbot.db.port", 3306);
	_dbHost = _pPrefs->configuration()->getString("xitech.xdbot.db.host", "");
	_dbName = _pPrefs->configuration()->getString("xitech.xdbot.db.name", "");

	_dbConnString = "host=" + _dbHost + ";port=" + std::to_string(_dbPort) + ";user=" + _dbUser + ";password=" + _dbPassword + ";db=" + _dbName + ";compress=true;auto-reconnect=true;secure-auth=true;protocol=tcp";
	_pContext->logger().information(Poco::format("database connect string: %s", _dbConnString));

	try {
		_pSession = new Session(MySQL::Connector::KEY, _dbConnString);
	}
	catch (ConnectionFailedException& exc) {
		_pContext->logger().error(Poco::format("Connection failed exception: %s", exc.displayText()));
		_pContext->logger().error("Trying to connect without DB and create one ...");
		connectNoDB();
		try {
			_pSession = new Session(MySQL::Connector::KEY, _dbConnString);
		}
		catch (ConnectionFailedException& ex) {
			_pContext->logger().error(Poco::format("Connection failed exception: %s", exc.displayText()));
			return;
		}
	}

	_pContext->logger().debug(Poco::format("*** Connected to [MySQL] '%s' database.", _dbName));
}

void EventRequestHandler::onMessage(const std::string& json)
{
    if (json.empty()) {
        return;
    }

    Poco::JSON::Parser parser;
    Poco::Dynamic::Var result;
	int32_t req = -1;
    try {
        result = parser.parse(json);

		Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();

		if (!object) {
			return;
		}

		const Poco::DynamicStruct& ds = *object;

		if (ds.size() <= 0) {
			return;
		}

		Poco::Dynamic::Var reqVar = ds["req"];
		if (reqVar.isInteger()) {
			req = reqVar;
		}

		int32_t type = -1;
		Poco::Dynamic::Var typeVar = ds["type"];
		if (typeVar.isInteger()) {
			type = typeVar;
		}

		// type, 0: detection alarm
		if (type == 0) {
			_scheduler.dispatchFunc([req, result, this](){
				auto models = parseDetectionAlarmData(result);
				if (!models.empty()) {
					if (processDetectionAlarm(models)) {
						response(req, 0, "success");
					}
					else {
						response(req, 1, "Save to database failed");
					}
				}					
				else {
					response(req, 2, "Invalid data content or format");
				}
			});
		}
    } 
	catch (Poco::JSON::JSONException& jsone) {
		std::cerr << jsone.what() << ": " << jsone.message() << std::endl;
		response(req, 3, "JSON parse exception");
    }
}

bool EventRequestHandler::processDetectionAlarm(const std::vector<Poco::SharedPtr<DetectionData>>& models)
{
	if (models.empty()) {
		return false;
	}

 	if  (!_pSession) {
		return false;
	}

	try {
		for (auto data : models) {
			Statement stmt = (
			*_pSession << "INSERT INTO DetectionData(devCode, devName, valueType, value, valueCN, recResult, recReason, imageUrl, eventTime) VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?) "
			, use(data->devCode)
			, use(data->devName)
			, use(data->valueType)
			, use(data->value)
			, use(data->valueCN)
			, use(data->recResult)
			, use(data->recReason)
			, use(data->imageUrl)
			, use(data->eventTime)
			);
			stmt.execute();
			poco_assert (stmt.done());
		}
		return true;
	}
	catch(Poco::Exception& exc) {
		std::cerr << exc.what() << ": " << exc.message() << std::endl;
		return false;
	}
}

std::vector<Poco::SharedPtr<DetectionData>> EventRequestHandler::parseDetectionAlarmData(const Poco::Dynamic::Var& var) 
{
    Poco::JSON::Object::Ptr object = var.extract<Poco::JSON::Object::Ptr>();

    if (!object) {
        return {};
    }

    const Poco::DynamicStruct& ds = *object;

    if (ds.size() <= 0) {
        return {};
    }

	std::string imgUrl;
    Poco::Dynamic::Var img = ds["imgUrl"];
    if (img.isString()) {
        imgUrl = img.convert<std::string>();
    }

	std::vector<Poco::SharedPtr<DetectionData>> modelVec;

	Poco::Dynamic::Var datas = ds["devList"];
    if (datas.isArray()) {
        for (const auto& elem : datas) {
            Poco::SharedPtr<DetectionData> model = new DetectionData();
            Poco::Dynamic::Var devCode = elem["devCode"];
            if (devCode.isString()) {
                model->devCode = devCode.convert<std::string>();
            }
			Poco::Dynamic::Var devName = elem["devName"];
            if (devName.isString()) {
                model->devName = devName.convert<std::string>();
            }
			Poco::Dynamic::Var valueType = elem["valueType"];
            if (valueType.isString()) {
                model->valueType = valueType.convert<std::string>();
            }
			Poco::Dynamic::Var value = elem["value"];
            if (value.isString()) {
                model->value = value.convert<std::string>();
            }
			Poco::Dynamic::Var valueCN = elem["valueCN"];
            if (valueCN.isString()) {
                model->valueCN = valueCN.convert<std::string>();
            }
            Poco::Dynamic::Var recResult = elem["recResult"];
			if (recResult.isInteger()) {
				model->recResult = recResult;
			}
			Poco::Dynamic::Var recReason = elem["recReason"];
            if (recReason.isString()) {
                model->recReason = recReason.convert<std::string>();
            }
            model->imageUrl = imgUrl;
			Poco::Dynamic::Var time = elem["time"];
            if (time.isString()) {
                std::string timeString = time.convert<std::string>();
                Poco::DateTime dt;
                int tz = -1;
                if (Poco::DateTimeParser::tryParse(Poco::DateTimeFormat::SORTABLE_FORMAT, timeString, dt, tz)) {
                    model->eventTime = dt;
                }
            }
            modelVec.emplace_back(model);
        }
    }

    return modelVec;
}

void EventRequestHandler::response(int64_t req, int32_t code, const std::string& reason) 
{
	Poco::JSON::Object resp;
	resp.set("resp", req);
	resp.set("code", code);
	resp.set("reason", reason);

	std::ostringstream osstr;
	Poco::JSON::Stringifier::stringify(resp, osstr);
	std::string str = osstr.str();
	send(str);
}

Poco::Net::HTTPRequestHandler* EventRequestHandlerFactory::createRequestHandler(const Poco::Net::HTTPServerRequest& request)
{
	return new EventRequestHandler(context());
}

} } // namespace xi::XDBotDataInboundService


// test json string:
// {
// "req":1,
// "type":0,
// "devList":[
// {
// "devName":"T1",
// "devCode":"1234567890",
// "valueType":"int",
// "value":"1",
// "valueCN":"opened",
// "recResult":1,
// "recReason":"daily",
// "time":"2023-06-20 19:36:34"
// },
// {
// "devName":"T2",
// "devCode":"1234567891",
// "valueType":"int",
// "value":"0",
// "valueCN":"closed",
// "recResult":1,
// "recReason":"daily",
// "time":"2023-06-20 19:36:54"
// }
// ],
// "imgUrl":"https://192.168.1.3/group1/M00/00/test.jpg"
// }