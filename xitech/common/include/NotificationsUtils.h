#pragma once 

#include <string>
#include <memory>
#include "Poco/NotificationQueue.h"
#include "Poco/BasicEvent.h"
#include "XDBotData.h"

namespace xi {
namespace utils {

class SensorDataNotification: public Poco::Notification
{
public:
	SensorDataNotification(std::shared_ptr<SensorData> data)
	: _data(data) {
	}
	
	~SensorDataNotification()
	{
	}
	
	std::shared_ptr<SensorData> data() {
		return _data;
	}
	
private:
	std::shared_ptr<SensorData> _data;
};

class SensorSingleAlarmNotification: public Poco::Notification
{
public:
	SensorSingleAlarmNotification(std::shared_ptr<AlarmData> data)
	: _data(data) {
	}
	
	~SensorSingleAlarmNotification()
	{
	}
	
	std::shared_ptr<AlarmData> data() {
		return _data;
	}
	
private:
	std::shared_ptr<AlarmData> _data;
};

class SensorMultipleAlarmNotification: public Poco::Notification
{
public:
	SensorMultipleAlarmNotification(const std::vector<std::shared_ptr<AlarmData>>& data)
	: _data(data) {
	}
	
	~SensorMultipleAlarmNotification()
	{
	}
	
	const std::vector<std::shared_ptr<AlarmData>>& data() {
		return _data;
	}
	
private:
	std::vector<std::shared_ptr<AlarmData>> _data;
};

class MessageNotification: public Poco::Notification
{
public:
	MessageNotification(const std::string& data)
	: _data(data) {
	}
	
	~MessageNotification() {
	}
	
	const std::string& data() {
		return _data;
	}
	
private:
	std::string _data;
};

class SensorStatusNotification: public Poco::Notification
{
public:
	SensorStatusNotification(const std::vector<std::shared_ptr<SensorStatus>>& statusList)
	: _statusList(statusList) {
	}
	
	~SensorStatusNotification()
	{
	}
	
	const std::vector<std::shared_ptr<SensorStatus>>& statusList() {
		return _statusList;
	}
	
private:
	std::vector<std::shared_ptr<SensorStatus>> _statusList;
};

class QueryActiveAlarmsNotification: public Poco::Notification
{
};

class ActiveAlarmsNotification: public Poco::Notification
{
public:
	ActiveAlarmsNotification(const std::vector<std::shared_ptr<AlarmData>>& data)
	: _data(data) {
	}
	
	~ActiveAlarmsNotification()
	{
	}
	
	const std::vector<std::shared_ptr<AlarmData>>& data() {
		return _data;
	}
	
private:
	std::vector<std::shared_ptr<AlarmData>> _data;
};

}
}