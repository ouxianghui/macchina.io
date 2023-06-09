#pragma once

#include <memory>
#include <unordered_map>
#include "Poco/NotificationQueue.h"
#include "Poco/Runnable.h"
#include "Poco/Data/Session.h"
#include "Poco/UniqueAccessExpireCache.h"
#include "Poco/AccessExpirationDecorator.h"
#include "Poco/Util/Timer.h"
#include "Poco/Util/TimerTask.h"
#include "Poco/Util/TimerTaskAdapter.h"
#include "Poco/Thread.h"
#include "NotificationsUtils.h"
#include "XDBotData.h"
#include "SerialTaskScheduler.h"
#include "ConcurrentTaskScheduler.h"

namespace xi {
namespace XDBotThermometer {

typedef Poco::AccessExpirationDecorator<std::shared_ptr<AlarmData>> AlarmDataVal;

class SensorDataHandler : public Poco::Runnable, public std::enable_shared_from_this<SensorDataHandler>
{
public:
	SensorDataHandler(Poco::NotificationQueue& queue, Poco::SharedPtr<Poco::Data::Session> pSession);

	~SensorDataHandler();

	void run();

	void stop();

private:
	void processSensorDataNf(xi::utils::SensorDataNotification* nf);

	void processSensorAlarmNf(xi::utils::SensorSingleAlarmNotification* nf);

	void processSensorStatusNf(xi::utils::SensorStatusNotification* nf);

	void onQueryActiveAlarmsNf(xi::utils::QueryActiveAlarmsNotification* nf);

	void insertSensorData(std::shared_ptr<SensorData> data);

	std::string getKey(std::shared_ptr<AlarmData> data);

	void insertAlarmData(std::shared_ptr<AlarmData> data);

	void updateAlarmData(std::shared_ptr<AlarmData> data);
	
	void onTimer(Poco::Util::TimerTask& task);

private:
	Poco::NotificationQueue& _queue;

	Poco::SharedPtr<Poco::Data::Session> _pSession;

	std::unordered_map<std::string, std::shared_ptr<AlarmData>> _activeAlarms;

	Poco::UniqueAccessExpireCache<std::string, AlarmDataVal> _alarmsCache;

	Poco::Util::Timer _timer;

	Poco::FastMutex _mutex;

	// only for database operations
	SerialTaskScheduler _scheduler;

	//ConcurrentTaskScheduler _concurrentScheduler;
};

}
}
