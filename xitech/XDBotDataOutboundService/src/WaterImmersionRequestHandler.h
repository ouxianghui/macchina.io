//
// WaterImmersionRequestHandler.h
//
// Copyright (c) 2017, Applied Informatics Software Engineering GmbH.
// All rights reserved.
//
// SPDX-License-Identifier: GPL-3.0-only
//


#ifndef XDBotDataOutboundService_WaterImmersionRequestHandler_INCLUDED
#define XDBotDataOutboundService_WaterImmersionRequestHandler_INCLUDED

#include <memory>
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/OSP/Web/WebRequestHandlerFactory.h"
#include "Poco/OSP/BundleContext.h"
#include "Poco/OSP/PreferencesService.h"
#include "Poco/OSP/ServiceRef.h"
#include "Poco/Mutex.h"
#include "Poco/NotificationQueue.h"
#include "Poco/Runnable.h"
#include "Poco/Timer.h"
#include "IoT/Devices/ISerialDevice.h"

namespace Poco {
	namespace Net {
		class WebSocket;
	}
}

namespace xi {
namespace XDBotDataOutboundService {


class WaterImmersionRequestHandler: public Poco::Net::HTTPRequestHandler
{
public:
	WaterImmersionRequestHandler(Poco::OSP::BundleContext::Ptr pContext);
	/// Creates the WaterImmersionRequestHandler using the given bundle context.

	~WaterImmersionRequestHandler();

	// Poco::Net::HTTPRequestHandler
	void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response);

protected:
	Poco::OSP::BundleContext::Ptr context() const;

private:
	Poco::OSP::BundleContext::Ptr _pContext;
	
	Poco::OSP::PreferencesService::Ptr _pPrefs;

	Poco::OSP::ServiceRef::Ptr _pServiceRef;

	IoT::Devices::ISerialDevice::Ptr _pSerialDevice;
};


class WaterImmersionRequestHandlerFactory: public Poco::OSP::Web::WebRequestHandlerFactory
{
public:
	Poco::Net::HTTPRequestHandler* createRequestHandler(const Poco::Net::HTTPServerRequest& request) override;
};


} }  // xi::XDBotDataOutboundService


#endif // XDBotDataOutboundService_WaterImmersionRequestHandler_INCLUDED
