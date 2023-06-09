#pragma once

#include <stdlib.h>
#include <string.h>
#include <strstream>
#include <thread>
#include <chrono>
#include <memory>
#include "HCNetSDK.h"
#include "Poco/Activity.h"
#include "Poco/NotificationQueue.h"
#include "XDBotData.h"

namespace xi {
namespace XDBotThermometer {

class BundleActivator;

class ActivityThermometer {
public:
	ActivityThermometer(BundleActivator& activator, 
	Poco::UInt32 id, 
	Poco::UInt32 channel,
	Poco::UInt32 type, 
	const std::string& logo,
	const std::string& ip,
	Poco::UInt16 port, 
	std::string& userName, 
	const std::string& password, 
	Poco::Int32 interval,
	const std::string& siteName, 
	const std::string& objectName);

	Poco::UInt32 id();

	LONG lUserID();

	void start();

	void stop();

	int32_t status();

	void notify(std::shared_ptr<SensorData> data);

	void notify(std::shared_ptr<AlarmData> data);

	const std::string& getSiteName() { return _siteName; }

	const std::string& getObjectName() { return _objectName; }

private:
	void runActivity(); 

private:
	Poco::Activity<ActivityThermometer> _activity;

	BundleActivator& _activator;

	LONG _lUserID = -1;

	Poco::UInt32 _id;

	Poco::UInt32 _channel;

	Poco::UInt32 _type;

	std::string _logo;

	std::string _ip;

	Poco::UInt16 _port; 

	std::string _userName; 

	std::string _password;
	
	Poco::Int32 _interval;

	std::string _siteName;

	std::string _objectName;
};

}

}