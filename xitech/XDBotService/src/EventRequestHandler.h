//
// EventRequestHandler.h
//
// Copyright (c) 2017, Applied Informatics Software Engineering GmbH.
// All rights reserved.
//
// SPDX-License-Identifier: GPL-3.0-only
//


#ifndef XDBotService_EventRequestHandler_INCLUDED
#define XDBotService_EventRequestHandler_INCLUDED

#include <memory>
#include <unordered_map>
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/OSP/Web/WebRequestHandlerFactory.h"
#include "Poco/OSP/BundleContext.h"
#include "Poco/OSP/PreferencesService.h"
#include "Poco/Mutex.h"
#include "Poco/NotificationQueue.h"
#include "Poco/Runnable.h"
#include "Poco/Timer.h"
#include "XDBotData.h"
#include "NotificationsUtils.h"

namespace Poco {
	namespace Net {
		class WebSocket;
	}
}

namespace xi {
namespace XDBotService {


class EventRequestHandler: public Poco::Net::HTTPRequestHandler, public Poco::Runnable
{
public:
	EventRequestHandler(Poco::OSP::BundleContext::Ptr pContext);
	/// Creates the EventRequestHandler using the given bundle context.

	~EventRequestHandler();

	// Poco::Net::HTTPRequestHandler
	void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response);

	void run() override;

protected:
	Poco::OSP::BundleContext::Ptr context() const;

	void shutdown();

	void send(const std::string& buffer);	

	void messagesLoop();

	void onAlarm(xi::utils::SensorMultipleAlarmNotification* nf);

	void onActiveAlarms(xi::utils::ActiveAlarmsNotification* nf);

	void onStatus(xi::utils::SensorStatusNotification* nf);

	void onSendPing(Poco::Timer& timer);

	void processAlarmData(const std::vector<std::shared_ptr<AlarmData>>& data);

private:

	Poco::OSP::BundleContext::Ptr _pContext;
	
	Poco::OSP::PreferencesService::Ptr _pPrefs;

	std::shared_ptr<Poco::Net::WebSocket> _pWS;

	int _flags;

	Poco::NotificationQueue _queue;

	Poco::SharedPtr<Poco::Timer> _pingTimer;

	Poco::TimerCallback<EventRequestHandler> _sendPing;

	uint64_t _seq = 0;

	std::atomic_bool _requested {false};
};


class EventRequestHandlerFactory: public Poco::OSP::Web::WebRequestHandlerFactory
{
public:
	Poco::Net::HTTPRequestHandler* createRequestHandler(const Poco::Net::HTTPServerRequest& request) override;
};


} }  // xi::XDBotService


#endif // XDBotService_EventRequestHandler_INCLUDED
