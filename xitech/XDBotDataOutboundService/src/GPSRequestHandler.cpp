//
// GPSRequestHandler.cpp
//
// Copyright (c) 2017, Applied Informatics Software Engineering GmbH.
// All rights reserved.
//
// SPDX-License-Identifier: GPL-3.0-only
//


#include "GPSRequestHandler.h"
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

GPSRequestHandler::GPSRequestHandler(Poco::OSP::BundleContext::Ptr pContext)
	: _pContext(pContext)
{
	_pPrefs = Poco::OSP::ServiceFinder::find<Poco::OSP::PreferencesService>(_pContext);

	std::string deviceName("io.macchina.serialport#2");
	pContext->logger().information(Poco::format("Serial device '%s'.", deviceName));
	_pServiceRef = pContext->registry().findByName(deviceName);
	if (_pServiceRef) {
		_pSerialDevice = _pServiceRef->castedInstance<IoT::Devices::ISerialDevice>();
	}
	else {
		_pContext->logger().warning("No serial device found.");
	}
}

GPSRequestHandler::~GPSRequestHandler()
{

}

Poco::OSP::BundleContext::Ptr GPSRequestHandler::context() const
{
	return _pContext;
}

void GPSRequestHandler::handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
{
	Poco::Path p(request.getURI(), Poco::Path::PATH_UNIX);
	p.makeFile();
	std::string resource = p.getBaseName();

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
			for (int32_t i = 0; i < 9; ++i) {
				if (_pSerialDevice->poll(1.0)) {
					auto ch = _pSerialDevice->readByte();
					buf.emplace_back(ch);
					std::cout << i << ": " << (int)ch << std::endl;
				}
				else {
					std::cout << "_pSerialDevice->readString() timeout" << std::endl;
				}
			}
			int32_t len = 0;
			if (buf.size() >= 3) {
				len = buf[2];
			}
			std::cout << "len: " << len << std::endl;
			float lat = 0.0;
			u_char vbuf[4] = {0};
			for (int32_t i = 0; i < len; ++i) {
				vbuf[i] = buf[len+2-i];
			}
			memcpy(&lat, vbuf, 4);
			std::cout << "lat: " << lat << std::endl;

			cmd.clear();
			cmd.push_back(0x01);
			cmd.push_back(0x03);
			cmd.push_back(0x00);
			cmd.push_back(0x02);
			cmd.push_back(0x00);
			cmd.push_back(0x02);
			cmd.push_back(0x65);
			cmd.push_back(0xCB);
			_pSerialDevice->writeString(cmd);
			buf.clear();
			for (int32_t i = 0; i < 9; ++i) {
				if (_pSerialDevice->poll(1.0)) {
					auto ch = _pSerialDevice->readByte();
					buf.emplace_back(ch);
					std::cout << i << ": " << (int)ch << std::endl;
				}
				else {
					std::cout << "_pSerialDevice->readString() timeout" << std::endl;
				}
			}
			len = 0;
			if (buf.size() >= 3) {
				len = buf[2];
			}
			std::cout << "len: " << len << std::endl;
			float lon = 0.0;
			vbuf[4] = {0};
			for (int32_t i = 0; i < len; ++i) {
				vbuf[i] = buf[len+2-i];
			}
			memcpy(&lon, vbuf, 4);
			std::cout << "lon: " << lon << std::endl;

			Poco::JSON::Object messsage;
			messsage.set("longtitude", lon);
			messsage.set("latitude", lat);
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

Poco::Net::HTTPRequestHandler* GPSRequestHandlerFactory::createRequestHandler(const Poco::Net::HTTPServerRequest& request)
{
	std::cout << "request.getURI(): " << request.getURI() << std::endl;
	return new GPSRequestHandler(context());
}

} } // namespace xi::XDBotDataOutboundService
