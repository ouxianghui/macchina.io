#include "ActivityThermometer.h"

#include <chrono>
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
#include "Poco/TextConverter.h"
#include "Poco/Latin1Encoding.h"
#include "XDBotData.h"
#include "NotificationsUtils.h"
#include "Poco/DateTime.h"
#include "Poco/LocalDateTime.h"
#include "utility.h"
#include "BundleActivator.h"

namespace xi {
namespace XDBotThermometer {

void RealtimeThermometryCallback(DWORD dwType, void* lpBuffer, DWORD dwBufLen, void* pUserData);
BOOL AlarmDataCallback(LONG lCommand, NET_DVR_ALARMER *pAlarmer, char *pAlarmInfo, DWORD dwBufLen, void* pUser);

ActivityThermometer::ActivityThermometer(BundleActivator& activator, 
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
const std::string& objectName)
: _activity(this, &ActivityThermometer::runActivity) 
, _activator(activator)
, _id(id)
, _channel(channel)
, _type(type)
, _logo(logo)
, _ip(ip)
, _port(port)
, _userName(userName)
, _password(password)
, _interval(interval) 
, _siteName(siteName)
, _objectName(objectName) {

}

Poco::UInt32 ActivityThermometer::id() 
{
	return _id;
}

LONG ActivityThermometer::lUserID() 
{
	return _lUserID;
}

void ActivityThermometer::start() 
{
	_activity.start();
}

void ActivityThermometer::stop() 
{
	_activity.stop();
	_activity.wait();
}

void ActivityThermometer::notify(std::shared_ptr<SensorData> data) 
{
	data->sensorId = _id;
	Poco::LocalDateTime now;
	Poco::Timestamp ts = now.timestamp();
	data->timestamp = Poco::DateTime(ts);
	data->recordId = 0;

	_activator.queue().enqueueNotification(new xi::utils::SensorDataNotification(data));
}

void ActivityThermometer::notify(std::shared_ptr<AlarmData> data)
{
	data->sensorId = _id;
	Poco::LocalDateTime now;
	Poco::Timestamp ts = now.timestamp();
	data->begin = Poco::DateTime(ts);
	data->recordId = 0;

	_activator.queue().enqueueNotification(new xi::utils::SensorSingleAlarmNotification(data));
}

int32_t ActivityThermometer::status() 
{
	if (_lUserID < 0) {
		return 0;
	}
	bool online = NET_DVR_RemoteControl(_lUserID, NET_DVR_CHECK_USER_STATUS, NULL, 0);
	return online ? 1 : 0;
}

void ActivityThermometer::runActivity() 
{
	// 初始化
	NET_DVR_Init();

	// 设置连接时间与重连时间
	NET_DVR_SetConnectTime(2000, 1);
	NET_DVR_SetReconnect(10000, true);

	//---------------------------------------
	// 注册设备
	// 登录参数，包括设备地址、登录用户、密码等
	NET_DVR_USER_LOGIN_INFO struLoginInfo = {0};
	struLoginInfo.bUseAsynLogin = 0; //同步登录方式
	strcpy(struLoginInfo.sDeviceAddress, _ip.c_str()); //设备 IP 地址
	struLoginInfo.wPort = _port; //设备服务端口
	strcpy(struLoginInfo.sUserName, _userName.c_str()); //设备登录用户名
	strcpy(struLoginInfo.sPassword, _password.c_str()); //设备登录密码

	// 设备信息, 输出参数
	NET_DVR_DEVICEINFO_V40 struDeviceInfoV40 = {0};
	_lUserID = NET_DVR_Login_V40(&struLoginInfo, &struDeviceInfoV40);
	if (_lUserID < 0) {
		printf("Login failed, error code: %d\n", NET_DVR_GetLastError());
		NET_DVR_Cleanup();
		return;
	}
	
	//printf("lUserID: %d\n", _lUserID);

	// 设置报警回调函数
    NET_DVR_SetDVRMessageCallBack_V31(AlarmDataCallback, static_cast<void *>(&_activator));

    // 启用布防
    LONG lHandleAlarm;
    NET_DVR_SETUPALARM_PARAM struAlarmParam = {0};
    struAlarmParam.dwSize = sizeof(struAlarmParam);

    // 温度或者温差报警不需要设置其他报警布防参数，不支持
    lHandleAlarm = NET_DVR_SetupAlarmChan_V41(_lUserID, &struAlarmParam);
    if (lHandleAlarm < 0) {
       printf("NET_DVR_SetupAlarmChan_V41 error, %d\n", NET_DVR_GetLastError());
       NET_DVR_Logout(_lUserID);
       NET_DVR_Cleanup();
       return;
    }

	NET_DVR_REALTIME_THERMOMETRY_COND cond = {0};
	cond.dwSize = sizeof(cond);
	cond.dwChan = _channel;
	cond.byRuleID = 0;
	cond.byMode = 1;
	cond.wInterval = _interval;
	LONG lHandleConfig;
	lHandleConfig = NET_DVR_StartRemoteConfig(_lUserID, NET_DVR_GET_REALTIME_THERMOMETRY, &cond, sizeof(NET_DVR_REALTIME_THERMOMETRY_COND), RealtimeThermometryCallback, this);
	if (lHandleConfig < 0) {
		printf("NET_DVR_StartRemoteConfig error, %d\n", NET_DVR_GetLastError());
		NET_DVR_Logout(_lUserID);
		NET_DVR_Cleanup();
		return;
	}

	while (!_activity.isStopped()) {
		Poco::Thread::sleep(1000);
	}

	if (!NET_DVR_StopRemoteConfig(lHandleConfig)) {
		printf("NET_DVR_StopRemoteConfig error, %d\n", NET_DVR_GetLastError());
		NET_DVR_Logout(_lUserID);
		NET_DVR_Cleanup();
		return;
	}

	// 撤销布防上传通道
    if (!NET_DVR_CloseAlarmChan_V30(lHandleAlarm)) {
       printf("NET_DVR_CloseAlarmChan_V30 error, %d\n", NET_DVR_GetLastError());
       NET_DVR_Logout(_lUserID);
       NET_DVR_Cleanup();
       return;
    }

	// 注销用户
	NET_DVR_Logout(_lUserID);

	// 释放 SDK 资源
	//NET_DVR_Cleanup();

	_lUserID = -1;
}

BOOL AlarmDataCallback(LONG lCommand, NET_DVR_ALARMER *pAlarmer, char *pAlarmInfo, DWORD dwBufLen, void* pUser)
{
	BundleActivator* activator = static_cast<BundleActivator *>(pUser);
	if (!activator) {
		std::cerr << "activator is nullptr!" << std::endl;
		return 0;
	}

	if (!pAlarmer) {
		std::cerr << "pAlarmer is nullptr!" << std::endl;
		return 0;
	}

	if ((int32_t)pAlarmer->byUserIDValid != 1) {
		std::cerr << "byUserIDValid != 1" << std::endl;
		return 0;
	}

	std::shared_ptr<ActivityThermometer> thermometer;
	std::vector<std::shared_ptr<ActivityThermometer>> sensors = activator->sensors();
	for ( auto the : sensors) {
		if (the->lUserID() == (int32_t)pAlarmer->lUserID) {
			thermometer = the;
			break;
		}
	}

	if (!thermometer) {
		std::cerr << "thermometer is nullptr" << std::endl;
		return 0;
	}

	switch(lCommand)
	{
	
	case COMM_THERMOMETRY_ALARM: //温度报警
	{
		//return 0;
		NET_DVR_THERMOMETRY_ALARM struThermometryAlarm = {0};
		memcpy(&struThermometryAlarm, pAlarmInfo, sizeof(NET_DVR_THERMOMETRY_ALARM));
		if (0 == struThermometryAlarm.byRuleCalibType)
		{
			// printf("sensorId: %d\n", thermometer->id());
			// printf("温度报警: Channel:%d\n, RuleID:%d\n, ThermometryUnit:%d\n, PresetNo:%d\n, RuleTemperature:%.1f\n, CurrTemperature:%.1f\n, suddenChangeCycle:%d\n, suddenChangeValue:%.1f\n, PTZ Info[Pan:%f, Tilt:%f, Zoom:%f]\n, AlarmLevel:%d\n, AlarmType:%d\n, AlarmRule:%d\n, RuleCalibType:%d\n, Point[x:%f, y:%f]\n, PicLen:%d, ThermalPicLen:%d\n, ThermalInfoLen:%d\n\n",
			// 	struThermometryAlarm.dwChannel,
			// 	struThermometryAlarm.byRuleID,
			// 	struThermometryAlarm.byThermometryUnit,
			// 	struThermometryAlarm.wPresetNo,
			// 	struThermometryAlarm.fRuleTemperature,
			// 	struThermometryAlarm.fCurrTemperature,
			// 	struThermometryAlarm.dwTemperatureSuddenChangeCycle,
			// 	struThermometryAlarm.fTemperatureSuddenChangeValue,
			// 	struThermometryAlarm.struPtzInfo.fPan,
			// 	struThermometryAlarm.struPtzInfo.fTilt,
			// 	struThermometryAlarm.struPtzInfo.fZoom,
			// 	struThermometryAlarm.byAlarmLevel,
			// 	struThermometryAlarm.byAlarmType,
			// 	struThermometryAlarm.byAlarmRule,
			// 	struThermometryAlarm.byRuleCalibType,
			// 	struThermometryAlarm.struPoint.fX,
			// 	struThermometryAlarm.struPoint.fY,
			// 	struThermometryAlarm.dwPicLen,
			// 	struThermometryAlarm.dwThermalPicLen,
			// 	struThermometryAlarm.dwThermalInfoLen);
		}
		else if (1 == struThermometryAlarm.byRuleCalibType || 2 == struThermometryAlarm.byRuleCalibType) {
			// int iPointNum = struThermometryAlarm.struRegion.dwPointNum;
			// for (int i = 0; i < iPointNum; i++) {
			// 	float fX = struThermometryAlarm.struRegion.struPos[i].fX;
			// 	float fY = struThermometryAlarm.struRegion.struPos[i].fY; 
			// 	printf("测温区域坐标点:X%d:%f,Y%d:%f;", iPointNum + 1, fX, iPointNum + 1, fY);
			// }
			// printf("sensorId: %d\n", thermometer->id());
			// printf("温度报警: Channel:%d\n, RuleID:%d\n, HighestPoint[x:%f, y:%f]\n, ThermometryUnit:%d\n, PresetNo:%d\n, RuleTemperature:%.1f\n, CurrTemperature:%.1f\n, suddenChangeCycle:%d\n, suddenChangeValue:%.1f\n, PTZ Info[Pan:%f, Tilt:%f, Zoom:%f]\n, AlarmLevel:%d\n, AlarmType:%d\n, AlarmRule:%d\n, RuleCalibType:%d\n, PicLen:%d, ThermalPicLen:%d, ThermalInfoLen:%d\n\n",
			// 	struThermometryAlarm.dwChannel,
			// 	struThermometryAlarm.byRuleID,
			// 	struThermometryAlarm.struHighestPoint.fX,
			// 	struThermometryAlarm.struHighestPoint.fY,
			// 	struThermometryAlarm.byThermometryUnit,
			// 	struThermometryAlarm.wPresetNo,
			// 	struThermometryAlarm.fRuleTemperature,
			// 	struThermometryAlarm.fCurrTemperature,
			// 	struThermometryAlarm.dwTemperatureSuddenChangeCycle,
			// 	struThermometryAlarm.fTemperatureSuddenChangeValue,
			// 	struThermometryAlarm.struPtzInfo.fPan,
			// 	struThermometryAlarm.struPtzInfo.fTilt,
			// 	struThermometryAlarm.struPtzInfo.fZoom,
			// 	struThermometryAlarm.byAlarmLevel,
			// 	struThermometryAlarm.byAlarmType,
			// 	struThermometryAlarm.byAlarmRule,
			// 	struThermometryAlarm.byRuleCalibType,
			// 	struThermometryAlarm.dwPicLen,
			// 	struThermometryAlarm.dwThermalPicLen,
			// 	struThermometryAlarm.dwThermalInfoLen);
		}
		auto value = std::make_shared<AlarmData>();
		value->type = 0;
		value->channel = struThermometryAlarm.dwChannel;
		value->ruleId = struThermometryAlarm.byRuleID;
		value->alarmType = struThermometryAlarm.byAlarmType;
		value->alarmLevel = struThermometryAlarm.byAlarmLevel;
		value->alarmRule = struThermometryAlarm.byAlarmRule;
		value->unit = struThermometryAlarm.byThermometryUnit;
		value->ruleValue = struThermometryAlarm.fRuleTemperature;
		value->currValue = struThermometryAlarm.fCurrTemperature;
		value->suddenChangeCycle = struThermometryAlarm.dwTemperatureSuddenChangeCycle;
		value->suddenChangeValue = struThermometryAlarm.fTemperatureSuddenChangeValue;
		value->siteName = thermometer->getSiteName();
		value->objectName = thermometer->getObjectName();
		thermometer->notify(value);
	}
		break;
	case COMM_THERMOMETRY_DIFF_ALARM: //温差报警
	{
		NET_DVR_THERMOMETRY_DIFF_ALARM struThermometryDiffAlarm = {0};
		memcpy(&struThermometryDiffAlarm, pAlarmInfo, sizeof(NET_DVR_THERMOMETRY_DIFF_ALARM));
		if (0 == struThermometryDiffAlarm.byRuleCalibType)
		{
			// printf("温差报警: Channel:%d\n, AlarmID1:%d\n, AlarmID2:%d\n, PresetNo:%d\n, RuleTemperatureDiff:%.1f\n, CurTemperatureDiff:%.1f\n, AlarmLevel:%d\n, AlarmType:%d\n, AlarmRule:%d\n, RuleCalibType:%d\n, Point1[x:%f, y:%f]\n, point2[x:%f, y:%f]\n, PTZ Info[Pan:%f, Tilt:%f, Zoom:%f]\n, PicLen:%d, ThermalPicLen:%d, ThermalInfoLen:%d\n, ThermometryUnit:%d\n\n",
			// 	struThermometryDiffAlarm.dwChannel,
			// 	struThermometryDiffAlarm.byAlarmID1,
			// 	struThermometryDiffAlarm.byAlarmID2,
			// 	struThermometryDiffAlarm.wPresetNo,
			// 	struThermometryDiffAlarm.fRuleTemperatureDiff,
			// 	struThermometryDiffAlarm.fCurTemperatureDiff,
			// 	struThermometryDiffAlarm.byAlarmLevel,
			// 	struThermometryDiffAlarm.byAlarmType,
			// 	struThermometryDiffAlarm.byAlarmRule,
			// 	struThermometryDiffAlarm.byRuleCalibType,
			// 	struThermometryDiffAlarm.struPoint[0].fX,
			// 	struThermometryDiffAlarm.struPoint[0].fY,
			// 	struThermometryDiffAlarm.struPoint[1].fX,
			// 	struThermometryDiffAlarm.struPoint[1].fY,
			// 	struThermometryDiffAlarm.struPtzInfo.fPan,
			// 	struThermometryDiffAlarm.struPtzInfo.fTilt,
			// 	struThermometryDiffAlarm.struPtzInfo.fZoom,
			// 	struThermometryDiffAlarm.dwPicLen,
			// 	struThermometryDiffAlarm.dwThermalPicLen,
			// 	struThermometryDiffAlarm.dwThermalInfoLen,
			// 	struThermometryDiffAlarm.byThermometryUnit);
		}
		else if (1 == struThermometryDiffAlarm.byRuleCalibType || 2 == struThermometryDiffAlarm.byRuleCalibType) {
			//int i = 0;
			// int iPointNum = struThermometryDiffAlarm.struRegion[0].dwPointNum;
			// for (i = 0; i < iPointNum; i++) {
			// 	float fX = struThermometryDiffAlarm.struRegion[0].struPos[i].fX;
			// 	float fY = struThermometryDiffAlarm.struRegion[0].struPos[i].fY;
			// 	printf("测温区域 1 坐标点: X%d:%f,Y%d:%f;", iPointNum + 1, fX, iPointNum + 1, fY);
			// }
			// iPointNum = struThermometryDiffAlarm.struRegion[1].dwPointNum;
			// for (i = 0; i < iPointNum; i++) {
			// 	float fX = struThermometryDiffAlarm.struRegion[1].struPos[i].fX;
			// 	float fY = struThermometryDiffAlarm.struRegion[1].struPos[i].fY;
			// 	printf("测温区域 2 坐标点: X%d:%f,Y%d:%f;", iPointNum + 1, fX, iPointNum + 1, fY);
			// }
			// printf("温差报警: Channel:%d\n, AlarmID1:%d\n, AlarmID2:%d\n, PresetNo:%d\n, RuleTemperatureDiff:%.1f\n, CurTemperatureDiff:%.1f\n, AlarmLevel:%d\n, AlarmType:%d\n, AlarmRule:%d\n, RuleCalibType:%d\n, PTZ Info[Pan:%f, Tilt:%f, Zoom:%f]\n, PicLen:%d, ThermalPicLen:%d, ThermalInfoLen:%d\n, ThermometryUnit:%d\n\n",
			// 	struThermometryDiffAlarm.dwChannel,
			// 	struThermometryDiffAlarm.byAlarmID1,
			// 	struThermometryDiffAlarm.byAlarmID2,
			// 	struThermometryDiffAlarm.wPresetNo,
			// 	struThermometryDiffAlarm.fRuleTemperatureDiff,
			// 	struThermometryDiffAlarm.fCurTemperatureDiff,
			// 	struThermometryDiffAlarm.byAlarmLevel,
			// 	struThermometryDiffAlarm.byAlarmType,
			// 	struThermometryDiffAlarm.byAlarmRule,
			// 	struThermometryDiffAlarm.byRuleCalibType,
			// 	struThermometryDiffAlarm.struPtzInfo.fPan,
			// 	struThermometryDiffAlarm.struPtzInfo.fTilt,
			// 	struThermometryDiffAlarm.struPtzInfo.fZoom,
			// 	struThermometryDiffAlarm.dwPicLen,
			// 	struThermometryDiffAlarm.dwThermalPicLen,
			// 	struThermometryDiffAlarm.dwThermalInfoLen,
			// 	struThermometryDiffAlarm.byThermometryUnit);
		}
		auto value = std::make_shared<AlarmData>();
		value->type = 1;
		value->channel = struThermometryDiffAlarm.dwChannel;
		value->ruleId = struThermometryDiffAlarm.byAlarmID1;
		value->alarmType = struThermometryDiffAlarm.byAlarmType;
		value->alarmLevel = struThermometryDiffAlarm.byAlarmLevel;
		value->alarmRule = struThermometryDiffAlarm.byAlarmRule;
		value->unit = struThermometryDiffAlarm.byThermometryUnit;
		value->ruleValue = struThermometryDiffAlarm.fRuleTemperatureDiff;
		value->currValue = struThermometryDiffAlarm.fCurTemperatureDiff;
		value->siteName = thermometer->getSiteName();
		value->objectName = thermometer->getObjectName();
		thermometer->notify(value);
	}
		break;
	default:
		printf("其他报警，报警信息类型: %d\n", lCommand);
		break;
	}
	return TRUE;
}

void RealtimeThermometryCallback(DWORD dwType, void* lpBuffer, DWORD dwBufLen, void* pUserData) {
	switch(dwType) {
	case NET_SDK_CALLBACK_TYPE_STATUS:
		break;
	case NET_SDK_CALLBACK_TYPE_PROGRESS:
		break;   
	case NET_SDK_CALLBACK_TYPE_DATA:
	{
		ActivityThermometer* thermometer = static_cast<ActivityThermometer *>(pUserData);
		if (!thermometer) {
			std::cerr << "thermometer is nullptr!" << std::endl;
			return;
		}

		NET_DVR_THERMOMETRY_UPLOAD data = {0};
		memcpy(&data, lpBuffer, sizeof(NET_DVR_THERMOMETRY_UPLOAD));

		std::string ruleName = std::string(data.szRuleName);

		// Poco::Latin1Encoding latin;
		// Poco::UTF8Encoding utf8;
		// std::string flag;
		// Poco::TextConverter converter(latin, utf8);
		// converter.convert(ruleName, flag);

		std::string flag = xi::utils::gbkToUtf8(ruleName);

		// std::cout << "dwAbsTime: " << (int64_t)data.dwAbsTime << std::endl
		// << ", dwRelativeTime: " << (int64_t)data.dwRelativeTime << std::endl
		// << ", szRuleName: " << flag << std::endl
		// << ", byRuleID: " << (int32_t)data.byRuleID << std::endl
		// << ", byRuleCalibType: " << (int32_t)data.byRuleCalibType << std::endl
		// << ", wPresetNo: " << data.wPresetNo << std::endl
		// << ", DVR_POINT_THERM_CFG.fTemperature: " << data.struPointThermCfg.fTemperature << std::endl
		// << ", DVR_POINT_THERM_CFG.NET_VCA_POINT.fX: " << data.struPointThermCfg.struPoint.fX << std::endl
		// << ", DVR_POINT_THERM_CFG.NET_VCA_POINT.fY: " << data.struPointThermCfg.struPoint.fY << std::endl
		// << ", NET_DVR_LINEPOLYGON_THERM_CFG.fMaxTemperature: " << data.struLinePolygonThermCfg.fMaxTemperature << std::endl
		// << ", NET_DVR_LINEPOLYGON_THERM_CFG.fMinTemperature: " << data.struLinePolygonThermCfg.fMinTemperature << std::endl
		// << ", NET_DVR_LINEPOLYGON_THERM_CFG.fAverageTemperature: " << data.struLinePolygonThermCfg.fAverageTemperature << std::endl
		// << ", NET_DVR_LINEPOLYGON_THERM_CFG.fTemperatureDiff: " << data.struLinePolygonThermCfg.fTemperatureDiff << std::endl
		// << ", byThermometryUnit: " << (int32_t)data.byThermometryUnit << std::endl
		// << ", byDataType: " << (int32_t)data.byDataType << std::endl
		// << ", bySpecialPointThermType: " << data.bySpecialPointThermType << std::endl
		// << ", fCenterPointTemperature: " << data.fCenterPointTemperature << std::endl
		// << ", fHighestPointTemperature: " << data.fHighestPointTemperature << std::endl
		// << ", fLowestPointTemperature: " << data.fLowestPointTemperature << std::endl
		// << std::endl;

		auto value = std::make_shared<SensorData>();
		value->channel = data.dwChan;
		value->ruleId = (int32_t)data.byRuleID;
		value->ruleName = flag;
		value->ruleCalibType = (int32_t)data.byRuleCalibType;

		if (value->ruleCalibType == 1 || value->ruleCalibType == 2) {
			value->minVal = data.struLinePolygonThermCfg.fMinTemperature;
			value->maxVal = data.struLinePolygonThermCfg.fMaxTemperature;
			value->avgVal =  data.struLinePolygonThermCfg.fAverageTemperature;
			value->diffVal =  data.struLinePolygonThermCfg.fTemperatureDiff;
		}
		else if (value->ruleCalibType == 0) {
			value->currVal = data.struPointThermCfg.fTemperature;
		}

		thermometer->notify(value);
	}
		break;
	default:
		break;
	}
	return;
}

}
}