//
// EventRequestHandler.h
//
// Copyright (c) 2017, Applied Informatics Software Engineering GmbH.
// All rights reserved.
//
// SPDX-License-Identifier: GPL-3.0-only
//


#ifndef XDBotDataInboundService_EventRequestHandler_INCLUDED
#define XDBotDataInboundService_EventRequestHandler_INCLUDED

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
#include "Poco/Data/LOB.h"
#include "Poco/Data/StatementImpl.h"
#include "Poco/Data/MySQL/Connector.h"
#include "Poco/Data/MySQL/Utility.h"
#include "Poco/Data/MySQL/MySQLException.h"
#include "Poco/Data/DataException.h"
#include "XDBotData.h"
#include "Poco/SerialTaskScheduler.h"

namespace Poco {
	namespace Net {
		class WebSocket;
	}
}

using namespace Poco::Data;
using namespace Poco::Data::Keywords;
using Poco::Data::MySQL::ConnectionException;
using Poco::Data::MySQL::Utility;
using Poco::Data::MySQL::StatementException;

namespace xi {
namespace XDBotDataInboundService {


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

	void onSendPing(Poco::Timer& timer);

	void connectNoDB();

	void setupDatabase();

	void onMessage(const std::string& buffer);

private:

	Poco::OSP::BundleContext::Ptr _pContext;
	
	Poco::OSP::PreferencesService::Ptr _pPrefs;

	std::shared_ptr<Poco::Net::WebSocket> _pWS;

	int _flags;

	Poco::SharedPtr<Poco::Timer> _pingTimer;

	Poco::TimerCallback<EventRequestHandler> _sendPing;

	uint64_t _seq = 0;


 	std::string _dbUser;

	std::string _dbPassword;

	std::string _dbHost;

	Poco::UInt16 _dbPort;

	std::string _dbName;

	std::string _dbConnString;

	Poco::SharedPtr<Poco::Data::Session> _pSession;

	// only for database operations
	SerialTaskScheduler _scheduler;
};


class EventRequestHandlerFactory: public Poco::OSP::Web::WebRequestHandlerFactory
{
public:
	Poco::Net::HTTPRequestHandler* createRequestHandler(const Poco::Net::HTTPServerRequest& request) override;
};


} }  // xi::XDBotDataInboundService


#endif // XDBotDataInboundService_EventRequestHandler_INCLUDED
