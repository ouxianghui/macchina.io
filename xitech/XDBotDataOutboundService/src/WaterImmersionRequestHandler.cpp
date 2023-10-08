//
// WaterImmersionRequestHandler.cpp
//
// Copyright (c) 2017, Applied Informatics Software Engineering GmbH.
// All rights reserved.
//
// SPDX-License-Identifier: GPL-3.0-only
//


#include "WaterImmersionRequestHandler.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/HTMLForm.h"
#include "Poco/OSP/Web/WebSession.h"
#include "Poco/OSP/Web/WebSessionManager.h"
#include "Poco/OSP/ServiceRegistry.h"
#include "Poco/OSP/ServiceFinder.h"
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
#include "Poco/Timestamp.h"

namespace xi {
namespace XDBotDataOutboundService {

WaterImmersionRequestHandler::WaterImmersionRequestHandler(Poco::OSP::BundleContext::Ptr pContext)
	: _pContext(pContext)
{
	_pPrefs = Poco::OSP::ServiceFinder::find<Poco::OSP::PreferencesService>(_pContext);

	std::string deviceName("io.macchina.serialport#5");
	pContext->logger().information(Poco::format("Serial device '%s'.", deviceName));
	_pServiceRef = pContext->registry().findByName(deviceName);
	if (_pServiceRef) {
		_pSerialDevice = _pServiceRef->castedInstance<IoT::Devices::ISerialDevice>();
	}
	else {
		_pContext->logger().warning("No serial device found.");
	}
}

WaterImmersionRequestHandler::~WaterImmersionRequestHandler()
{

}

Poco::OSP::BundleContext::Ptr WaterImmersionRequestHandler::context() const
{
	return _pContext;
}

void WaterImmersionRequestHandler::handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
{
	if (request.getMethod() == "HEAD" || request.getMethod() == "GET" || request.getMethod() == "POST") {
		response.setContentType("application/json");

		if (_pSerialDevice) {
			// "01 03 00 05 00 02 D4 0A", "01 03 00 02 00 02 65 CB"
			std::string cmd;
			cmd.push_back(0x01);
			cmd.push_back(0x03);
			cmd.push_back(0x00);
			cmd.push_back(0x05);
			cmd.push_back(0x00);
			cmd.push_back(0x02);
			cmd.push_back(0xD4);
			cmd.push_back(0x0A);
			_pSerialDevice->writeString(cmd);
			std::vector<u_char> buf;
			for (int32_t i = 0; i < 11; ++i) {
				if (_pSerialDevice->poll(1.0)) {
					auto ch = _pSerialDevice->readByte();
					buf.emplace_back(ch);
				}
				else {
					std::cout << "_pSerialDevice->readString() timeout" << std::endl;
				}
			}
			int32_t len = 0;
			if (buf.size() >= 3) {
				len = buf[2];
			}
			short status = 0;
			u_char vbuf[2] = {0};
			for (int32_t i = 0; i < len; ++i) {
				vbuf[i] = buf[len+2-i];
			}
			memcpy(&status, vbuf, 2);

			Poco::JSON::Object messsage;
			messsage.set("status", status);
			std::ostringstream osstr;
			Poco::JSON::Stringifier::stringify(messsage, osstr);
			response.send() << osstr.str();
		}
		else {
			response.send()
			<< "{"
			<<   "\"error\":No serial device found"
			<< "}";
		}
	}
	else {
		response.setContentType("application/json");
		response.setChunkedTransferEncoding(true);
		response.setStatusAndReason(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
		response.send()
			<< "{"
			<<   "\"error\":" << Utility::jsonize("Bad request")
			<< "}";
	}
}

Poco::Net::HTTPRequestHandler* WaterImmersionRequestHandlerFactory::createRequestHandler(const Poco::Net::HTTPServerRequest& request)
{
	std::cout << "request.getURI(): " << request.getURI() << std::endl;
	return new WaterImmersionRequestHandler(context());
}

} } // namespace xi::XDBotDataOutboundService
