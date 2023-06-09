#pragma once

#include <stdlib.h>
#include <string.h>
#include <strstream>
#include <thread>
#include <chrono>
#include <memory>
#include "Poco/Activity.h"
#include "Poco/NotificationQueue.h"
#include "IoT/Devices/ISerialDevice.h"
#include "NotificationsUtils.h"

namespace xi {
namespace XDBotAlarmer {

class AlarmHandler {
public:
	AlarmHandler(IoT::Devices::ISerialDevice::Ptr pSerialDevice);

	void init();

	void destroy();

protected:
	void onAlarm(xi::utils::SensorMultipleAlarmNotification* nf);

private:
	void runActivity(); 

	void process(const std::string& json);

private:
	Poco::Activity<AlarmHandler> _activity;

	IoT::Devices::ISerialDevice::Ptr _pSerialDevice;

	Poco::NotificationQueue _queue;
};

}
}