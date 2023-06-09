#pragma once

#include "Poco/String.h"
#include "Poco/Format.h"
#include "Poco/Tuple.h"
#include "Poco/DateTime.h"
#include "Poco/Any.h"
#include "Poco/Exception.h"
#include "Poco/Data/LOB.h"
#include "Poco/Data/Date.h"
#include "Poco/Data/Time.h"
#include "Poco/Data/StatementImpl.h"
#include "Poco/Data/RecordSet.h"
#include "Poco/Data/Transaction.h"
#include "Poco/Data/MySQL/Connector.h"
#include "Poco/Data/MySQL/MySQLException.h"

#ifdef _WIN32
#include <Winsock2.h>
#endif

#include <mysql/mysql.h>
#include <iostream>
#include <limits>


using namespace Poco::Data;
using namespace Poco::Data::Keywords;
using Poco::Data::MySQL::ConnectionException;
using Poco::Data::MySQL::StatementException;
using Poco::format;
using Poco::Tuple;
using Poco::DateTime;
using Poco::Any;
using Poco::AnyCast;
using Poco::NotFoundException;
using Poco::InvalidAccessException;
using Poco::BadCastException;
using Poco::RangeException;

struct Site
{
	uint32_t siteId;
	std::string name;

	Site() : siteId(-1), name("") {}

	Site(uint32_t _siteId, const std::string& _name) : siteId(_siteId), name(_name) {

    }

	bool operator==(const Site& other) const {
		return siteId == other.siteId && name == other.name;
	}

	bool operator!=(const Site& other) const {
		return !(*this == other);
	}

	bool operator < (const Site& site) const {
		if (siteId < site.siteId) {
			return true;
		}

		return (name < site.name);
	}

	uint32_t operator () () {
		/// This method is required so we can extract data to a map!
		// we choose the lastName as examplary key
		return siteId;
	}
};

struct Object
{
	uint32_t objectId;
	std::string name;
	uint32_t siteId;

	Object() : objectId(-1), name(""), siteId(-1) {}

	Object(uint32_t _objectId, const std::string& _name, uint32_t _siteId) : objectId(_objectId), name(_name), siteId(_siteId) {

	}

	bool operator==(const Object& other) const {
		return objectId == other.objectId && name == other.name && siteId == other.siteId;
	}

	bool operator!=(const Object& other) const {
		return !(*this == other);
	}

	bool operator < (const Object& object) const {
		if (objectId < object.objectId) {
			return true;
		}

		return (name < object.name);
	}

	uint32_t operator () () { 
		/// This method is required so we can extract data to a map!
		// we choose the lastName as examplary key
		return objectId;
	}
};

struct Sensor
{
	uint32_t sensorId;
	std::string name;
	uint32_t type;
	std::string logo;
	uint32_t objectId;

	Sensor() 
	: sensorId(-1), name(""), type(-1), logo(""), objectId(-1) {}

	Sensor(uint32_t _sensorId, const std::string& _name, uint32_t _type, const std::string& _logo, uint32_t _objectId) 
	: sensorId(_sensorId), name(_name), type(_type), logo(_logo), objectId(_objectId) {
    }

	bool operator==(const Sensor& other) const {
		return sensorId == other.sensorId && name == other.name && type == other.type && logo == other.logo &&
		objectId == other.objectId;
	}

	bool operator!=(const Sensor& other) const {
		return !(*this == other);
	}

	bool operator < (const Sensor& sensor) const {
		if (sensorId < sensor.sensorId) {
			return true;
		}

		return (name < sensor.name);
	}

	uint32_t operator () () {
		/// This method is required so we can extract data to a map!
		// we choose the lastName as examplary key
		return sensorId;
	}
};

struct SensorData
{
	uint32_t sensorId;
	uint32_t channel;
	uint32_t ruleId;
	std::string ruleName;
	uint32_t ruleCalibType;
	float currVal = -100;
	float minVal = -100;
	float maxVal = -100;
	float avgVal = -100;
	float diffVal = -100;
	DateTime timestamp;
	uint64_t recordId;

	SensorData() {}

	bool operator==(const SensorData& other) const {
		return recordId == other.recordId && sensorId == other.sensorId && channel == other.channel && timestamp == other.timestamp && ruleId == other.ruleId;
	}

	bool operator < (const SensorData& data) const {
		if (recordId < data.recordId) {
			return true;
		}

		if (sensorId < data.sensorId) {
			return true;
		}

		return (timestamp < data.timestamp);
	}

	uint64_t operator () () {
		/// This method is required so we can extract data to a map!
		// we choose the lastName as examplary key
		return recordId;
	}
};

struct AlarmData
{
	uint32_t sensorId;
	uint32_t type;
	uint32_t channel;
	uint32_t ruleId;
	uint32_t alarmType;
	uint32_t alarmLevel;
	uint32_t alarmRule;
	uint32_t unit;
	float ruleValue = -100;
	float currValue = -100;
	uint32_t suddenChangeCycle;
	float suddenChangeValue = -100;
	DateTime begin;
	DateTime end;
	uint32_t status;
	uint64_t recordId;

	// out of database
	std::string siteName;
	std::string objectName;

	AlarmData() {}

	bool operator==(const AlarmData& other) const {
		return recordId == other.recordId 
		&& sensorId == other.sensorId 
		&& type == other.type 
		&& channel == other.channel 
		&& ruleId == other.ruleId 
		&& alarmType == other.alarmType 
		&& alarmLevel == other.alarmLevel;
	}

	bool operator < (const AlarmData& data) const {
		if (recordId < data.recordId) {
			return true;
		}

		if (sensorId < data.sensorId) {
			return true;
		}

		return (begin < data.begin);
	}

	uint64_t operator () () {
		/// This method is required so we can extract data to a map!
		// we choose the lastName as examplary key
		return recordId;
	}
};

struct SensorStatus {
	uint32_t sensorId;
	uint32_t status;
};

namespace Poco {
namespace Data {

template <>
class TypeHandler<Site>
{
public:
	static void bind(std::size_t pos, const Site& obj, AbstractBinder::Ptr pBinder, AbstractBinder::Direction dir) {
		poco_assert_dbg (!pBinder.isNull());
		TypeHandler<uint32_t>::bind(pos++, obj.siteId, pBinder, dir);
		TypeHandler<std::string>::bind(pos++, obj.name, pBinder, dir);
	}

	static void prepare(std::size_t pos, const Site& obj, AbstractPreparator::Ptr pPrepare) {
		poco_assert_dbg (!pPrepare.isNull());
		TypeHandler<uint32_t>::prepare(pos++, obj.siteId, pPrepare);
		TypeHandler<std::string>::prepare(pos++, obj.name, pPrepare);
	}

	static std::size_t size() {
		return 2;
	}

	static void extract(std::size_t pos, Site& obj, const Site& defVal, AbstractExtractor::Ptr pExt) {
		poco_assert_dbg (!pExt.isNull());
		TypeHandler<uint32_t>::extract(pos++, obj.siteId, defVal.siteId, pExt);
		TypeHandler<std::string>::extract(pos++, obj.name, defVal.name, pExt);
	}

private:
	TypeHandler();
	~TypeHandler();
	TypeHandler(const TypeHandler&);
	TypeHandler& operator=(const TypeHandler&);
};

template <>
class TypeHandler<Object>
{
public:
	static void bind(std::size_t pos, const Object& obj, AbstractBinder::Ptr pBinder, AbstractBinder::Direction dir) {
		poco_assert_dbg (!pBinder.isNull());
		TypeHandler<uint32_t>::bind(pos++, obj.objectId, pBinder, dir);
		TypeHandler<std::string>::bind(pos++, obj.name, pBinder, dir);
		TypeHandler<uint32_t>::bind(pos++, obj.siteId, pBinder, dir);
	}

	static void prepare(std::size_t pos, const Object& obj, AbstractPreparator::Ptr pPrepare) {
		poco_assert_dbg (!pPrepare.isNull());
		TypeHandler<uint32_t>::prepare(pos++, obj.objectId, pPrepare);
		TypeHandler<std::string>::prepare(pos++, obj.name, pPrepare);
		TypeHandler<uint32_t>::prepare(pos++, obj.siteId, pPrepare);
	}

	static std::size_t size() {
		return 3;
	}

	static void extract(std::size_t pos, Object& obj, const Object& defVal, AbstractExtractor::Ptr pExt) {
		poco_assert_dbg (!pExt.isNull());
		TypeHandler<uint32_t>::extract(pos++, obj.objectId, defVal.objectId, pExt);
		TypeHandler<std::string>::extract(pos++, obj.name, defVal.name, pExt);
		TypeHandler<uint32_t>::extract(pos++, obj.siteId, defVal.siteId, pExt);
	}

private:
	TypeHandler();
	~TypeHandler();
	TypeHandler(const TypeHandler&);
	TypeHandler& operator=(const TypeHandler&);
};

template <>
class TypeHandler<Sensor>
{
public:
	static void bind(std::size_t pos, const Sensor& obj, AbstractBinder::Ptr pBinder, AbstractBinder::Direction dir) {
		poco_assert_dbg (!pBinder.isNull());
		TypeHandler<uint32_t>::bind(pos++, obj.sensorId, pBinder, dir);
		TypeHandler<std::string>::bind(pos++, obj.name, pBinder, dir);
		TypeHandler<uint32_t>::bind(pos++, obj.type, pBinder, dir);
		TypeHandler<std::string>::bind(pos++, obj.logo, pBinder, dir);
        TypeHandler<uint32_t>::bind(pos++, obj.objectId, pBinder, dir);
	}

	static void prepare(std::size_t pos, const Sensor& obj, AbstractPreparator::Ptr pPrepare) {
		poco_assert_dbg (!pPrepare.isNull());
		TypeHandler<uint32_t>::prepare(pos++, obj.sensorId, pPrepare);
		TypeHandler<std::string>::prepare(pos++, obj.name, pPrepare);
		TypeHandler<uint32_t>::prepare(pos++, obj.type, pPrepare);
		TypeHandler<std::string>::prepare(pos++, obj.logo, pPrepare);
        TypeHandler<uint32_t>::prepare(pos++, obj.objectId, pPrepare);
	}

	static std::size_t size() {
		return 5;
	}

	static void extract(std::size_t pos, Sensor& obj, const Sensor& defVal, AbstractExtractor::Ptr pExt) {
		poco_assert_dbg (!pExt.isNull());
		TypeHandler<uint32_t>::extract(pos++, obj.sensorId, defVal.sensorId, pExt);
        TypeHandler<std::string>::extract(pos++, obj.name, defVal.name, pExt);
		TypeHandler<uint32_t>::extract(pos++, obj.type, defVal.type, pExt);
		TypeHandler<std::string>::extract(pos++, obj.logo, defVal.logo, pExt);
		TypeHandler<uint32_t>::extract(pos++, obj.objectId, defVal.objectId, pExt);
	}

private:
	TypeHandler();
	~TypeHandler();
	TypeHandler(const TypeHandler&);
	TypeHandler& operator=(const TypeHandler&);
};

template <>
class TypeHandler<SensorData>
{
public:
	static std::size_t size() {
		return 12;
	}

	static void bind(std::size_t pos, const SensorData& obj, AbstractBinder::Ptr pBinder, AbstractBinder::Direction dir) {
		poco_assert_dbg (!pBinder.isNull());
		TypeHandler<uint32_t>::bind(pos++, obj.sensorId, pBinder, dir);
		TypeHandler<uint32_t>::bind(pos++, obj.channel, pBinder, dir);
		TypeHandler<uint32_t>::bind(pos++, obj.ruleId, pBinder, dir);
		TypeHandler<std::string>::bind(pos++, obj.ruleName, pBinder, dir);
		TypeHandler<uint32_t>::bind(pos++, obj.ruleCalibType, pBinder, dir);
		TypeHandler<float>::bind(pos++, obj.currVal, pBinder, dir);
		TypeHandler<float>::bind(pos++, obj.minVal, pBinder, dir);
        TypeHandler<float>::bind(pos++, obj.maxVal, pBinder, dir);
		TypeHandler<float>::bind(pos++, obj.avgVal, pBinder, dir);
        TypeHandler<float>::bind(pos++, obj.diffVal, pBinder, dir);
        TypeHandler<Poco::DateTime>::bind(pos++, obj.timestamp, pBinder, dir);
		TypeHandler<uint64_t>::bind(pos++, obj.recordId, pBinder, dir);
	}

	static void prepare(std::size_t pos, const SensorData& obj, AbstractPreparator::Ptr pPrepare) {
		poco_assert_dbg (!pPrepare.isNull());
		TypeHandler<uint32_t>::prepare(pos++, obj.sensorId, pPrepare);
		TypeHandler<uint32_t>::prepare(pos++, obj.channel, pPrepare);
		TypeHandler<uint32_t>::prepare(pos++, obj.ruleId, pPrepare);
		TypeHandler<std::string>::prepare(pos++, obj.ruleName, pPrepare);
		TypeHandler<uint32_t>::prepare(pos++, obj.ruleCalibType, pPrepare);
		TypeHandler<float>::prepare(pos++, obj.currVal, pPrepare);
		TypeHandler<float>::prepare(pos++, obj.minVal, pPrepare);
        TypeHandler<float>::prepare(pos++, obj.maxVal, pPrepare);
		TypeHandler<float>::prepare(pos++, obj.avgVal, pPrepare);
        TypeHandler<float>::prepare(pos++, obj.diffVal, pPrepare);
        TypeHandler<Poco::DateTime>::prepare(pos++, obj.timestamp, pPrepare);
		TypeHandler<uint64_t>::prepare(pos++, obj.recordId, pPrepare);
	}

	static void extract(std::size_t pos, SensorData& obj, const SensorData& defVal, AbstractExtractor::Ptr pExt) {
		poco_assert_dbg (!pExt.isNull());

		TypeHandler<uint32_t>::extract(pos++, obj.sensorId, defVal.sensorId, pExt);
		TypeHandler<uint32_t>::extract(pos++, obj.channel, defVal.channel, pExt);
		TypeHandler<uint32_t>::extract(pos++, obj.ruleId, defVal.ruleId, pExt);
		TypeHandler<std::string>::extract(pos++, obj.ruleName, defVal.ruleName, pExt);
		TypeHandler<uint32_t>::extract(pos++, obj.ruleCalibType, defVal.ruleCalibType, pExt);
		TypeHandler<float>::extract(pos++, obj.currVal, defVal.currVal, pExt);
		TypeHandler<float>::extract(pos++, obj.minVal, defVal.minVal, pExt);
		TypeHandler<float>::extract(pos++, obj.maxVal, defVal.maxVal, pExt);
		TypeHandler<float>::extract(pos++, obj.avgVal, defVal.avgVal, pExt);
		TypeHandler<float>::extract(pos++, obj.diffVal, defVal.diffVal, pExt);
		TypeHandler<Poco::DateTime>::extract(pos++, obj.timestamp, defVal.timestamp, pExt);
		TypeHandler<uint64_t>::extract(pos++, obj.recordId, defVal.recordId, pExt);

	}

private:
	TypeHandler();
	~TypeHandler();
	TypeHandler(const TypeHandler&);
	TypeHandler& operator=(const TypeHandler&);
};

template <>
class TypeHandler<AlarmData>
{
public:
	static std::size_t size() {
		return 15;
	}

	static void bind(std::size_t pos, const AlarmData& obj, AbstractBinder::Ptr pBinder, AbstractBinder::Direction dir) {
		poco_assert_dbg (!pBinder.isNull());
		TypeHandler<uint32_t>::bind(pos++, obj.sensorId, pBinder, dir);
		TypeHandler<uint32_t>::bind(pos++, obj.type, pBinder, dir);
		TypeHandler<uint32_t>::bind(pos++, obj.channel, pBinder, dir);
		TypeHandler<uint32_t>::bind(pos++, obj.ruleId, pBinder, dir);
        TypeHandler<uint32_t>::bind(pos++, obj.alarmType, pBinder, dir);
		TypeHandler<uint32_t>::bind(pos++, obj.alarmLevel, pBinder, dir);
		TypeHandler<uint32_t>::bind(pos++, obj.alarmRule, pBinder, dir);
        TypeHandler<uint32_t>::bind(pos++, obj.unit, pBinder, dir);
		TypeHandler<float>::bind(pos++, obj.ruleValue, pBinder, dir);
		TypeHandler<float>::bind(pos++, obj.currValue, pBinder, dir);
		TypeHandler<uint32_t>::bind(pos++, obj.suddenChangeCycle, pBinder, dir);
		TypeHandler<float>::bind(pos++, obj.suddenChangeValue, pBinder, dir);
        TypeHandler<Poco::DateTime>::bind(pos++, obj.begin, pBinder, dir);
		TypeHandler<Poco::DateTime>::bind(pos++, obj.end, pBinder, dir);
		TypeHandler<uint64_t>::bind(pos++, obj.recordId, pBinder, dir);
	}

	static void prepare(std::size_t pos, const AlarmData& obj, AbstractPreparator::Ptr pPrepare) {
		poco_assert_dbg (!pPrepare.isNull());
		TypeHandler<uint32_t>::prepare(pos++, obj.sensorId, pPrepare);
		TypeHandler<uint32_t>::prepare(pos++, obj.type, pPrepare);
		TypeHandler<uint32_t>::prepare(pos++, obj.channel, pPrepare);
		TypeHandler<uint32_t>::prepare(pos++, obj.ruleId, pPrepare);
        TypeHandler<uint32_t>::prepare(pos++, obj.alarmType, pPrepare);
		TypeHandler<uint32_t>::prepare(pos++, obj.alarmLevel, pPrepare);
		TypeHandler<uint32_t>::prepare(pos++, obj.alarmRule, pPrepare);
		TypeHandler<uint32_t>::prepare(pos++, obj.unit, pPrepare);
        TypeHandler<float>::prepare(pos++, obj.ruleValue, pPrepare);
		TypeHandler<float>::prepare(pos++, obj.currValue, pPrepare);
		TypeHandler<uint32_t>::prepare(pos++, obj.suddenChangeCycle, pPrepare);
        TypeHandler<float>::prepare(pos++, obj.suddenChangeValue, pPrepare);
        TypeHandler<Poco::DateTime>::prepare(pos++, obj.begin, pPrepare);
		TypeHandler<Poco::DateTime>::prepare(pos++, obj.end, pPrepare);
		TypeHandler<uint64_t>::prepare(pos++, obj.recordId, pPrepare);
	}

	static void extract(std::size_t pos, AlarmData& obj, const AlarmData& defVal, AbstractExtractor::Ptr pExt) {
		poco_assert_dbg (!pExt.isNull());

		TypeHandler<uint32_t>::extract(pos++, obj.sensorId, defVal.sensorId, pExt);
		TypeHandler<uint32_t>::extract(pos++, obj.type, defVal.type, pExt);
		TypeHandler<uint32_t>::extract(pos++, obj.channel, defVal.channel, pExt);
		TypeHandler<uint32_t>::extract(pos++, obj.ruleId, defVal.ruleId, pExt);
		TypeHandler<uint32_t>::extract(pos++, obj.alarmType, defVal.alarmType, pExt);
		TypeHandler<uint32_t>::extract(pos++, obj.alarmLevel, defVal.alarmLevel, pExt);
		TypeHandler<uint32_t>::extract(pos++, obj.alarmRule, defVal.alarmRule, pExt);
		TypeHandler<uint32_t>::extract(pos++, obj.unit, defVal.unit, pExt);
		TypeHandler<float>::extract(pos++, obj.ruleValue, defVal.ruleValue, pExt);
		TypeHandler<float>::extract(pos++, obj.currValue, defVal.currValue, pExt);
		TypeHandler<uint32_t>::extract(pos++, obj.suddenChangeCycle, defVal.suddenChangeCycle, pExt);
		TypeHandler<float>::extract(pos++, obj.suddenChangeValue, defVal.suddenChangeValue, pExt);
		TypeHandler<Poco::DateTime>::extract(pos++, obj.begin, defVal.begin, pExt);
		TypeHandler<Poco::DateTime>::extract(pos++, obj.end, defVal.end, pExt);
		TypeHandler<uint64_t>::extract(pos++, obj.recordId, defVal.recordId, pExt);

	}

private:
	TypeHandler();
	~TypeHandler();
	TypeHandler(const TypeHandler&);
	TypeHandler& operator=(const TypeHandler&);
};

} } // namespace Poco::Data
