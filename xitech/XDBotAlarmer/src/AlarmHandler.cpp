#include "AlarmHandler.h"
#include <sstream>
#include <chrono>
#include <locale>
#include <codecvt>
#include <iconv.h>
#include "Poco/OSP/BundleActivator.h"
#include "Poco/OSP/BundleContext.h"
#include "Poco/OSP/ServiceRegistry.h"
#include "Poco/OSP/ServiceFinder.h"
#include "Poco/OSP/ServiceRef.h"
#include "Poco/OSP/PreferencesService.h"
#include "Poco/Util/Timer.h"
#include "Poco/Util/TimerTask.h"
#include "Poco/Format.h"
#include "Poco/ClassLibrary.h"
#include "Poco/Exception.h"
#include "Poco/Nullable.h"
#include "Poco/SharedLibrary.h"
#include "Poco/Activity.h"
#include "Poco/Thread.h"
#include "Poco/UTF8Encoding.h"
#include "Poco/UTF16Encoding.h"
#include "Poco/UTF32Encoding.h"
#include "Poco/TextConverter.h"
#include "Poco/Latin1Encoding.h"
#include "Poco/DateTime.h"
#include "Poco/LocalDateTime.h"
#include "Poco/DateTimeFormat.h"
#include "Poco/DateTimeFormatter.h"
#include "Poco/NotificationCenter.h"
#include "Poco/Observer.h"
#include "Poco/NObserver.h"
#include "Poco/JSON/Object.h"
#include "Poco/JSON/Array.h"
#include "Poco/JSON/JSONException.h"
#include "Poco/JSON/Stringifier.h"
#include "Poco/JSON/Parser.h"
#include "Poco/Dynamic/Struct.h"
#include "Poco/Dynamic/Var.h"
#include "Poco/Net/WebSocket.h"
#include "Poco/Net/NetException.h"
#include "NotificationsUtils.h"
#include "XDBotData.h"
#include "utility.h"


namespace xi {
namespace XDBotAlarmer {

AlarmHandler::AlarmHandler(IoT::Devices::ISerialDevice::Ptr pSerialDevice)
 : _activity(this, &AlarmHandler::runActivity)
 , _pSerialDevice(pSerialDevice)
{

}

void AlarmHandler::init() 
{
	Poco::NotificationCenter::defaultCenter().addObserver(Poco::Observer<AlarmHandler, xi::utils::SensorMultipleAlarmNotification>(*this, &AlarmHandler::onAlarm));
	
	_activity.start();
}

void AlarmHandler::destroy() 
{
	_queue.wakeUpAll();

	_activity.stop();
	_activity.wait();

	Poco::NotificationCenter::defaultCenter().removeObserver(Poco::Observer<AlarmHandler, xi::utils::SensorMultipleAlarmNotification>(*this, &AlarmHandler::onAlarm));
}

void AlarmHandler::onAlarm(xi::utils::SensorMultipleAlarmNotification* nf)
{
	std::vector<std::shared_ptr<AlarmData>> vecData = nf->data();
	if (vecData.empty()) {
		return;
	}

	for (auto data : vecData) {
		std::string alarmLevel;
		if (data->alarmLevel == 0) {
			alarmLevel = xi::utils::utf8ToGBK("预警");
		}
		else if (data->alarmLevel == 1) {
			alarmLevel = xi::utils::utf8ToGBK("报警");
		}

		std::string siteName = xi::utils::utf8ToGBK(data->siteName);
		std::string objectName = xi::utils::utf8ToGBK(data->objectName);
		std::string status = data->status == 1 ? xi::utils::utf8ToGBK("开始") : xi::utils::utf8ToGBK("结束");
		std::string dt = Poco::DateTimeFormatter::format(data->begin, Poco::DateTimeFormat::SORTABLE_FORMAT);
		std::stringstream sms;
		sms << "[" << status << "]"
		<< "[" << dt << "] " 
		<< (siteName) << "-" 
		<< (objectName) << "-" 
		<< alarmLevel << "-";
		if (data->alarmType == 0 || data->alarmType == 1 || data->alarmType == 2) {
			sms << xi::utils::utf8ToGBK("当前温度") << data->currValue << xi::utils::utf8ToGBK("度") << "-" 
			<< xi::utils::utf8ToGBK("报警阈值") << data->ruleValue << xi::utils::utf8ToGBK("度") << std::endl; 
		} else if (data->alarmType == 4) {
			sms << xi::utils::utf8ToGBK("温度突升") << data->suddenChangeValue << xi::utils::utf8ToGBK("度") << std::endl;
		} else if (data->alarmType == 5) {
			sms << xi::utils::utf8ToGBK("温度突降") << data->suddenChangeValue << xi::utils::utf8ToGBK("度") << std::endl; 
		} else {
			continue;
		}
		//std::cout << "alarm: " << sms.str() << std::endl;
		_queue.enqueueNotification(new xi::utils::MessageNotification(sms.str()));
	}
}

void AlarmHandler::runActivity()
{
	try {
		Poco::AutoPtr<Poco::Notification> pNf = _queue.waitDequeueNotification();
		while (pNf) {
			if (auto pMsgNf = pNf.cast<xi::utils::MessageNotification>()) {
				std::string json(pMsgNf->data());
				process(json);
			}
			pNf = _queue.waitDequeueNotification();
		}
	}
	catch (Poco::Exception&) {

	}
}

void AlarmHandler::process(const std::string& json)
{
	if (_pSerialDevice) {
		_pSerialDevice->writeString(json);
	}
}

}
}