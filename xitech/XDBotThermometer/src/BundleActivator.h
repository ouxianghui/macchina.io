#pragma once

#include <stdlib.h>
#include <string.h>
#include <strstream>
#include <thread>
#include <chrono>
#include <unordered_map>
#include "Poco/OSP/BundleActivator.h"
#include "Poco/OSP/BundleContext.h"
#include "Poco/OSP/ServiceRegistry.h"
#include "Poco/OSP/ServiceFinder.h"
#include "Poco/OSP/ServiceRef.h"
#include "Poco/OSP/PreferencesService.h"
#include "Poco/Timer.h"
#include "Poco/Format.h"
#include "Poco/ClassLibrary.h"
#include "Poco/Exception.h"
#include "Poco/Data/LOB.h"
#include "Poco/Data/StatementImpl.h"
#include "Poco/Data/MySQL/Connector.h"
#include "Poco/Data/MySQL/Utility.h"
#include "Poco/Data/MySQL/MySQLException.h"
#include "Poco/Nullable.h"
#include "Poco/Data/DataException.h"
#include "Poco/SharedLibrary.h"
#include "Poco/Activity.h"
#include "Poco/Thread.h"
#include "Poco/SharedMemory.h"
#include "Poco/JSON/Object.h"
#include "Poco/JSON/Array.h"
#include "Poco/JSON/JSONException.h"
#include "Poco/JSON/Stringifier.h"
#include "Poco/Dynamic/Struct.h"
#include "Poco/Dynamic/Var.h"
#include "Poco/UTF8Encoding.h"
#include "Poco/TextConverter.h"
#include "Poco/Latin1Encoding.h"
#include "HCNetSDK.h"
#include "XDBotData.h"
#include "ActivityThermometer.h"
#include "Poco/NotificationQueue.h"
#include "SensorDataHandler.h"

namespace xi {
namespace XDBotThermometer {

using namespace Poco::Data;
using namespace Poco::Data::Keywords;
using Poco::Data::MySQL::ConnectionException;
using Poco::Data::MySQL::Utility;
using Poco::Data::MySQL::StatementException;


class BundleActivator: public Poco::OSP::BundleActivator
{
public:
	BundleActivator();

	~BundleActivator();

	void start(Poco::OSP::BundleContext::Ptr pContext);

	void stop(Poco::OSP::BundleContext::Ptr pContext);

	Poco::NotificationQueue& queue();

	const std::vector<std::shared_ptr<ActivityThermometer>>& sensors();

private:
	void dbInfo(Session& session);

	void connectNoDB();

	void setupSensorDataHandler();

	void onReportStatus(Poco::Timer& timer);

	void setupDatabase();

	void updateSites();

	void updateObjects();

	void updateSensors();

	void updateConfigs2Database();

	void startupSensors();

private:
	Poco::OSP::BundleContext::Ptr _pContext;

	Poco::OSP::PreferencesService::Ptr _pPrefs;

	std::shared_ptr<Poco::Timer> _statusTimer;

 	std::string _dbUser;

	std::string _dbPassword;

	std::string _dbHost;

	Poco::UInt16 _dbPort;

	std::string _dbName;

	std::string _dbConnString;

	static Poco::SharedPtr<Poco::Data::Session> _pSession;

	std::shared_ptr<Poco::SharedLibrary> _hcNetSDK;

	Poco::FastMutex _mutex;
	
	std::vector<std::shared_ptr<ActivityThermometer>> _sensors;

	Poco::NotificationQueue _queue;

	std::shared_ptr<SensorDataHandler> _sensorDataHandler;

	std::unordered_map<int32_t, Site> _sitesMap;

	std::unordered_map<int32_t, Object> _objectsMap;
};

}
}