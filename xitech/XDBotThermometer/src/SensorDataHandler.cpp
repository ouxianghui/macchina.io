#include "SensorDataHandler.h"
#include "Poco/NotificationCenter.h"
#include "Poco/Observer.h"
#include "Poco/Data/MySQL/Utility.h"
#include "Poco/Data/MySQL/MySQLException.h"
#include <thread>
#include "Poco/DateTime.h"

namespace xi {
namespace XDBotThermometer {

SensorDataHandler::SensorDataHandler(Poco::NotificationQueue& queue, Poco::SharedPtr<Poco::Data::Session> pSession)
: _queue(queue), _pSession(pSession) 
{
	_scheduler.start();

	//_concurrentScheduler.start();

	Poco::NotificationCenter::defaultCenter().addObserver(Poco::Observer<SensorDataHandler, xi::utils::QueryActiveAlarmsNotification>(*this, &SensorDataHandler::onQueryActiveAlarmsNf));

	Poco::Util::TimerTask::Ptr pTask = new Poco::Util::TimerTaskAdapter<SensorDataHandler>(*this, &SensorDataHandler::onTimer);
	_timer.schedule(pTask, 5000, 3000);
}

SensorDataHandler::~SensorDataHandler() 
{
	Poco::NotificationCenter::defaultCenter().removeObserver(Poco::Observer<SensorDataHandler, xi::utils::QueryActiveAlarmsNotification>(*this, &SensorDataHandler::onQueryActiveAlarmsNf));

	if (_pSession) {
		_pSession->close();
	}
}

void SensorDataHandler::run()
{
	Poco::AutoPtr<Poco::Notification> pNf(_queue.waitDequeueNotification());
	while (pNf)
	{
		if (xi::utils::SensorDataNotification* pSDNf = dynamic_cast<xi::utils::SensorDataNotification*>(pNf.get())) {
			processSensorDataNf(pSDNf);
		}
		else if (xi::utils::SensorSingleAlarmNotification* pSANf = dynamic_cast<xi::utils::SensorSingleAlarmNotification*>(pNf.get())) {
			processSensorAlarmNf(pSANf);
		}
		else if (xi::utils::SensorStatusNotification* pSSNf = dynamic_cast<xi::utils::SensorStatusNotification*>(pNf.get())) {
			processSensorStatusNf(pSSNf);
		}
		pNf = _queue.waitDequeueNotification();
	}
}

void SensorDataHandler::stop()
{
	_timer.cancel();
	_queue.wakeUpAll();

	_scheduler.stop();
}

void SensorDataHandler::insertSensorData(std::shared_ptr<SensorData> data)
{
	if (!data) {
		return;
	}

 	if  (!_pSession) {
		return;
	}

	try {
		Statement stmt = (
		*_pSession << "INSERT INTO SensorData(sensorId, channel, ruleId, ruleName, ruleCalibType, currVal, minVal, maxVal, avgVal, diffVal, timestamp) VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?) "
		, use(data->sensorId)
		, use(data->channel)
		, use(data->ruleId)
		, use(data->ruleName)
		, use(data->ruleCalibType)
		, use(data->currVal)
		, use(data->minVal)
		, use(data->maxVal)
		, use(data->avgVal)
		, use(data->diffVal)
		, use(data->timestamp)
		);

		stmt.execute();
		poco_assert (stmt.done());
	}
	catch(Poco::Exception& exc) {
		std::cerr << exc.what() << ": " << exc.message() << std::endl;
	}
}

void SensorDataHandler::processSensorDataNf(xi::utils::SensorDataNotification* nf) 
{
	if (!nf){
		return;
	}	

	// static std::atomic_bool flag = {false};
	// if (flag) {
	// 	return;
	// }
	// flag = true;
	// static std::atomic_int32_t counter = {0};
	// for (int64_t i = 0; i < 9999; ++i) {
	// 	_concurrentScheduler.dispatchFunc([wself = std::weak_ptr<SensorDataHandler>(shared_from_this()), data = nf->data(), &counter](){
	// 		auto self = wself.lock();
	// 		if (!self) {
	// 			return;
	// 		}
	// 		std::cout << "concurrent task scheduler::run(), tid: " << std::this_thread::get_id() << " counter: " << counter++ << std::endl;
	// 	});
	// }

	_scheduler.dispatchFunc([wself = std::weak_ptr<SensorDataHandler>(shared_from_this()), data = nf->data()](){
		auto self = wself.lock();
		if (!self) {
			return;
		}
		self->insertSensorData(data);
	});
}

void SensorDataHandler::processSensorAlarmNf(xi::utils::SensorSingleAlarmNotification* nf) 
{
	if (!nf){
		return;
	}	

	auto key = getKey(nf->data());
	bool isNewAlarm = false;
	{
		Poco::FastMutex::ScopedLock lock(_mutex);
		if (_activeAlarms.find(key) == _activeAlarms.end()) {
			isNewAlarm = true;
			_activeAlarms[key] = nf->data();
			_alarmsCache.add(key, AlarmDataVal(nf->data(), Poco::Timespan(15, 0)));
			std::cerr << "add active alarm: " << key << std::endl;
		}
		else {
			_alarmsCache.get(key);
			std::cerr << "update active alarm: " << key << std::endl;
		}
	}
	if (isNewAlarm) {
		nf->data()->status = 1;

		// save new alarm to db
		_scheduler.dispatchFunc([wself = std::weak_ptr<SensorDataHandler>(shared_from_this()), data = nf->data()](){
			auto self = wself.lock();
			if (!self) {
				return;
			}
			self->insertAlarmData(data);
		});

		// broadcast all active alarms
		std::vector<std::shared_ptr<AlarmData>> data;
		Poco::FastMutex::ScopedLock lock(_mutex);
		for (auto item : _activeAlarms) {
			data.emplace_back(item.second);
		}
		if (!data.empty()) {
			Poco::NotificationCenter::defaultCenter().postNotification(new xi::utils::SensorMultipleAlarmNotification(data));
		}
	}
}

void SensorDataHandler::processSensorStatusNf(xi::utils::SensorStatusNotification* nf)
{
	if (!nf){
		return;
	}	

	Poco::NotificationCenter::defaultCenter().postNotification(new xi::utils::SensorStatusNotification(nf->statusList()));	
}

std::string SensorDataHandler::getKey(std::shared_ptr<AlarmData> data)
{
	std::stringstream key;
	//key << data->sensorId << data->channel << data->type << data->ruleId << data->alarmType << data->alarmLevel;
	key << data->sensorId << data->ruleId;
	return key.str();
}

void SensorDataHandler::insertAlarmData(std::shared_ptr<AlarmData> data)
{
	if (!data) {
		return;
	}

 	if  (!_pSession) {
		return;
	}

	try {
		Statement stmt = (
		*_pSession << "INSERT INTO AlarmData(sensorId, type, channel, ruleId, alarmType, alarmLevel, alarmRule, unit, ruleValue, currValue, suddenChangeCycle, suddenChangeValue, begin) VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?) "
		, use(data->sensorId)
		, use(data->type)
		, use(data->channel)
		, use(data->ruleId)
		, use(data->alarmType)
		, use(data->alarmLevel)
		, use(data->alarmRule)
		, use(data->unit)
		, use(data->ruleValue)
		, use(data->currValue)
		, use(data->suddenChangeCycle)
		, use(data->suddenChangeValue)
		, use(data->begin)
		);
		stmt.execute();
		poco_assert (stmt.done());
	}
	catch(Poco::Exception& exc) {
		std::cerr << exc.what() << ": " << exc.message() << std::endl;
	}
}

void SensorDataHandler::updateAlarmData(std::shared_ptr<AlarmData> data)
{
	if (!data) {
		return;
	}

	if  (!_pSession) {
		return;
	}

	try {
		Statement stmt = (
		*_pSession << "UPDATE AlarmData SET end = ? WHERE sensorId = ? and type = ? and ruleId = ?"
		, use(data->end)
		, use(data->sensorId)
		, use(data->type)
		, use(data->ruleId)
		);
		stmt.execute();
		poco_assert (stmt.done());
	}
	catch(Poco::Exception& exc) {
		std::cerr << exc.what() << ": " << exc.message() << std::endl;
	}
}

void SensorDataHandler::onTimer(Poco::Util::TimerTask& task)
{
	Poco::FastMutex::ScopedLock lock(_mutex);

	std::vector<std::shared_ptr<AlarmData>> canceledAlarms;
	for  (auto it = _activeAlarms.begin(); it != _activeAlarms.end(); ++it) {
		auto key = it->first;
		if (!_alarmsCache.has(key)) {
			Poco::LocalDateTime now;
			Poco::Timestamp ts = now.timestamp();
			it->second->end = Poco::DateTime(ts);
			it->second->status = 0;

			_scheduler.dispatchFunc([wself = std::weak_ptr<SensorDataHandler>(shared_from_this()), data = it->second](){
				auto self = wself.lock();
				if (!self) {
					return;
				}
				self->updateAlarmData(data);
			});

			canceledAlarms.emplace_back(it->second);
			std::cerr << "remove active alarm: " << key << std::endl;
		}
	}

	if (!canceledAlarms.empty()) {
		Poco::NotificationCenter::defaultCenter().postNotification(new xi::utils::SensorMultipleAlarmNotification(canceledAlarms));
	}

	for  (auto it = _activeAlarms.begin(); it != _activeAlarms.end();) {
		auto key = it->first;
		if (!_alarmsCache.has(key)) {
			_activeAlarms.erase(it++);
		} else {
			++it;
		}
	}
}

void SensorDataHandler::onQueryActiveAlarmsNf(xi::utils::QueryActiveAlarmsNotification* nf)
{
	std::vector<std::shared_ptr<AlarmData>> data;
	for (auto item : _activeAlarms) {
		data.emplace_back(item.second);
	}
	if (!data.empty()) {
		Poco::NotificationCenter::defaultCenter().postNotification(new xi::utils::ActiveAlarmsNotification(data));
	}
}

}}