//
// BundleActivator.cpp
//
// Copyright (c) 2019, Applied Informatics Software Engineering GmbH.
// All rights reserved.
//
// SPDX-License-Identifier: GPL-3.0-only
//

#include "BundleActivator.h"

namespace xi {
namespace XDBotThermometer {

Poco::SharedPtr<Poco::Data::Session> BundleActivator::_pSession = 0;

BundleActivator::BundleActivator() 
{
	_hcNetSDK = std::make_shared<Poco::SharedLibrary>("libhcnetsdk.so");
}

BundleActivator::~BundleActivator() 
{
	_hcNetSDK->unload();
	if (_pSession) {
		_pSession->close();
	}
}

void BundleActivator::start(Poco::OSP::BundleContext::Ptr pContext) 
{
	// 初始化
	// NET_DVR_Init();

	// 设置连接时间与重连时间
	NET_DVR_SetConnectTime(2000, 1);
	NET_DVR_SetReconnect(10000, true);

	_pContext = pContext;

	_pPrefs = Poco::OSP::ServiceFinder::find<Poco::OSP::PreferencesService>(pContext);

	setupDatabase();

	updateConfigs2Database();

	setupSensorDataHandler();

	startupSensors();

	_statusTimer = std::make_shared<Poco::Timer>(1000, 1000 * 10);
	_statusTimer->start(Poco::TimerCallback<BundleActivator>(*this, &BundleActivator::onReportStatus));
}

void BundleActivator::stop(Poco::OSP::BundleContext::Ptr pContext) 
{
	if (_statusTimer) {
		_statusTimer->stop();
	}

	for (const auto& device : _sensors) {
		device->stop();
	}

	_pPrefs.reset();

	_pContext.reset();

	if (_sensorDataHandler) {
		_sensorDataHandler->stop();
	}

	_sitesMap.clear();
	
	_objectsMap.clear();

	// 释放 SDK 资源
	NET_DVR_Cleanup();
}

Poco::NotificationQueue& BundleActivator::queue()
{
	return _queue;
}

const std::vector<std::shared_ptr<ActivityThermometer>>& BundleActivator::sensors()
{
	Poco::FastMutex::ScopedLock lock(_mutex);
	return _sensors;
}

void BundleActivator::dbInfo(Session& session) 
{
	_pContext->logger().information(Poco::format("Server Info: %s", Utility::serverInfo(session)));
	_pContext->logger().information(Poco::format("Server Version: %s", Utility::serverVersion(session)));
	_pContext->logger().information(Poco::format("Host Info: %s", Utility::hostInfo(session)));
}

void BundleActivator::connectNoDB() 
{
	std::string dbConnString = "host=" + _dbHost + ";port=" + std::to_string(_dbPort) + ";user=" + _dbUser + ";password=" + _dbPassword + ";db=" + _dbName + ";compress=true;auto-reconnect=true;secure-auth=true;protocol=tcp";

	try {
		Session session(MySQL::Connector::KEY, dbConnString);

		_pContext->logger().information("*** Connected to [MySQL] without database.");

		dbInfo(session);

		session << "CREATE DATABASE IF NOT EXISTS " + _dbName + ";", now;

		_pContext->logger().information("Disconnecting ...");

		session.close();

		_pContext->logger().information("Disconnected");
	}
	catch (ConnectionFailedException& exc) {
		_pContext->logger().error(Poco::format("Connection failed exception: %s", exc.displayText()));
	}
}

void BundleActivator::setupDatabase() 
{
	MySQL::Connector::registerConnector();

	_dbUser = _pPrefs->configuration()->getString("xitech.xdbot.db.user", "");
	_dbPassword = _pPrefs->configuration()->getString("xitech.xdbot.db.password", "");
	_dbPort = _pPrefs->configuration()->getInt("xitech.xdbot.db.port", 3306);
	_dbHost = _pPrefs->configuration()->getString("xitech.xdbot.db.host", "");
	_dbName = _pPrefs->configuration()->getString("xitech.xdbot.db.name", "");

	_dbConnString = "host=" + _dbHost + ";port=" + std::to_string(_dbPort) + ";user=" + _dbUser + ";password=" + _dbPassword + ";db=" + _dbName + ";compress=true;auto-reconnect=true;secure-auth=true;protocol=tcp";
	_pContext->logger().information(Poco::format("database connect string: %s", _dbConnString));

	try {
		_pSession = new Session(MySQL::Connector::KEY, _dbConnString);
	}
	catch (ConnectionFailedException& exc) {
		_pContext->logger().error(Poco::format("Connection failed exception: %s", exc.displayText()));
		_pContext->logger().error("Trying to connect without DB and create one ...");
		connectNoDB();
		try {
			_pSession = new Session(MySQL::Connector::KEY, _dbConnString);
		}
		catch (ConnectionFailedException& ex) {
			_pContext->logger().error(Poco::format("Connection failed exception: %s", exc.displayText()));
			return;
		}
	}
	_pContext->logger().debug(Poco::format("*** Connected to [MySQL] '%s' database.", _dbName));
	dbInfo(*_pSession);
}

void BundleActivator::updateSites() 
{
	std::vector<Site> oldSites;
	try {
		*_pSession << "SELECT siteId, name FROM Site", into(oldSites), now;
	}
	catch (Poco::Exception& exc) {
		_pContext->logger().error(Poco::format("Fetch site, %s", exc.displayText()));
	}

	std::vector<Site> newSites;
	Poco::Util::AbstractConfiguration::Keys keys;
	_pPrefs->configuration()->keys("xitech.xdbot.site", keys);

	Poco::UInt32 id = -1;
	std::string name;
	for (std::vector<std::string>::const_iterator it = keys.begin(); it != keys.end(); ++it) {
		try {
			std::string baseKey = "xitech.xdbot.site.";
			baseKey += *it;

			id = _pPrefs->configuration()->getUInt(baseKey + ".id", -1);
			name = _pPrefs->configuration()->getString(baseKey + ".name", "");

			newSites.emplace_back(Site(id, name));
		}
		catch (Poco::Exception& exc) {
			_pContext->logger().error(Poco::format("Invalid site configuration, id: %s,  %s", id, exc.displayText()));
		}
	}

	std::map<uint32_t, std::string> oldSitesMap;
	for (const auto& site : oldSites) {
		oldSitesMap[site.siteId] = site.name;
	}

	for (const auto& site : newSites) {
		_sitesMap[site.siteId] = site;
	}

	for (auto site : newSites) {
		if (oldSitesMap.find(site.siteId) == oldSitesMap.end()) {
			// insert
			try {
				*_pSession << "INSERT INTO Site VALUES(?, ?)", use(site), now;
			}
			catch (Poco::Exception& exc) {
				_pContext->logger().error(Poco::format("Insert Site, id: %d,  %s", site.siteId, exc.displayText()));
			}
		}
		else {
			auto name = oldSitesMap[site.siteId];
			if (site.name != name) {
				// update
				try {
					*_pSession << "UPDATE Site SET name = ? WHERE siteId = ?", use(site.name), use(site.siteId), now;
				}
				catch (Poco::Exception& exc) {
					_pContext->logger().error(Poco::format("Update Site, id: %s,  %s", site.siteId, exc.displayText()));
				}
			}
		}
	}
}

void BundleActivator::updateObjects() 
{
	std::vector<Object> oldObjects;
	try {
		*_pSession << "SELECT * FROM Object", into(oldObjects), now;
	}
	catch (Poco::Exception& exc) {
		_pContext->logger().error(Poco::format("Fetch object, %s", exc.displayText()));
	}

	std::vector<Object> newObjects;
	Poco::Util::AbstractConfiguration::Keys keys;
	_pPrefs->configuration()->keys("xitech.xdbot.object", keys);

	Poco::UInt32 id = -1;
	std::string name;
	Poco::UInt32 siteId = -1;
	for (std::vector<std::string>::const_iterator it = keys.begin(); it != keys.end(); ++it) {
		try {
			std::string baseKey = "xitech.xdbot.object.";
			baseKey += *it;

			id = _pPrefs->configuration()->getUInt(baseKey + ".id", -1);
			name = _pPrefs->configuration()->getString(baseKey + ".name", "");
			siteId = _pPrefs->configuration()->getUInt(baseKey + ".siteId", -1);

			newObjects.emplace_back(Object(id, name, siteId));
		}
		catch (Poco::Exception& exc) {
			_pContext->logger().error(Poco::format("Invalid object configuration, id: %s,  %s", id, exc.displayText()));
		}
	}

	std::map<uint32_t, Object> oldObjectsMap;
	for (const auto& object : oldObjects) {
		oldObjectsMap[object.objectId] = object;
	}

	for (const auto& object : newObjects) {
		_objectsMap[object.objectId] = object;
	}

	for (auto object : newObjects) {
		if (oldObjectsMap.find(object.objectId) == oldObjectsMap.end()) {
			// insert
			try {
				*_pSession << "INSERT INTO Object VALUES(?, ?, ?)", use(object), now;
			}
			catch (Poco::Exception& exc) {
				_pContext->logger().error(Poco::format("Insert Object, id: %d,  %s", object.objectId, exc.displayText()));
			}
		}
		else {
			auto old = oldObjectsMap[object.objectId];
			if (object != old) {
				// update
				try {
					*_pSession << "UPDATE Object SET name = ?, siteId = ? WHERE objectId = ?", use(object.name), use(object.siteId), use(object.objectId), now;
				}
				catch (Poco::Exception& exc) {
					_pContext->logger().error(Poco::format("Update Object, id: %s,  %s", object.objectId, exc.displayText()));
				}
			}
		}
	}
}

void BundleActivator::updateSensors() 
{
	std::vector<Sensor> oldSensors;
	try {
		*_pSession << "SELECT * FROM Sensor", into(oldSensors), now;
	}
	catch (Poco::Exception& exc) {
		_pContext->logger().error(Poco::format("Fetch sensor, %s", exc.displayText()));
	}

	std::vector<Sensor> newSensors;
	Poco::Util::AbstractConfiguration::Keys keys;
	_pPrefs->configuration()->keys("xitech.xdbot.sensor", keys);

	Poco::UInt32 id = -1;
	std::string name;
	Poco::UInt32 type = -1;
	std::string logo;
	Poco::UInt32 objectId = -1;
	for (std::vector<std::string>::const_iterator it = keys.begin(); it != keys.end(); ++it) {
		try {
			std::string baseKey = "xitech.xdbot.sensor.";
			baseKey += *it;

			id = _pPrefs->configuration()->getUInt(baseKey + ".id", -1);
			name = _pPrefs->configuration()->getString(baseKey + ".name", "");
			type = _pPrefs->configuration()->getUInt(baseKey + ".type", -1);
			logo = _pPrefs->configuration()->getString(baseKey + ".logo", "");
			objectId = _pPrefs->configuration()->getUInt(baseKey + ".objectId", -1);

			newSensors.emplace_back(Sensor(id, name, type, logo, objectId));
		}
		catch (Poco::Exception& exc) {
			_pContext->logger().error(Poco::format("Invalid sensor configuration, id: %s,  %s", id, exc.displayText()));
		}
	}

	std::map<uint32_t, Sensor> oldSensorMap;
	for (const auto& sensor : oldSensors) {
		oldSensorMap[sensor.sensorId] = sensor;
	}

	for (auto sensor : newSensors) {
		if (oldSensorMap.find(sensor.sensorId) == oldSensorMap.end()) {
			// insert
			try {
				*_pSession << "INSERT INTO Sensor VALUES(?, ?, ?, ?, ?)", use(sensor), now;
			}
			catch (Poco::Exception& exc) {
				_pContext->logger().error(Poco::format("Insert Sensor, id: %d,  %s", sensor.sensorId, exc.displayText()));
			}
		}
		else {
			auto old = oldSensorMap[sensor.sensorId];
			if (sensor != old) {
				// update
				try {
					*_pSession << "UPDATE Sensor SET name = ?, type = ?, logo = ?, objectId = ? WHERE sensorId = ?", use(sensor.name), use(sensor.type), use(sensor.logo), use(sensor.objectId), use(sensor.sensorId), now;
				}
				catch (Poco::Exception& exc) {
					_pContext->logger().error(Poco::format("Update Sensor, id: %s,  %s", sensor.sensorId, exc.displayText()));
				}
			}
		}
	}
}

void BundleActivator::updateConfigs2Database() 
{
	updateSites();

	updateObjects();

	updateSensors();
}

void BundleActivator::startupSensors() 
{
	Poco::Util::AbstractConfiguration::Keys keys;
	_pPrefs->configuration()->keys("xitech.xdbot.sensor", keys);
	for (std::vector<std::string>::const_iterator it = keys.begin(); it != keys.end(); ++it) {
		Poco::UInt32 id = -1;
		try {
			std::string baseKey = "xitech.xdbot.sensor.";
			baseKey += *it;

			Poco::UInt32 type = _pPrefs->configuration()->getInt(baseKey + ".type", -1);
			if (type != 1) {
				continue;
			}

			id = _pPrefs->configuration()->getUInt(baseKey + ".id");
			Poco::UInt32 channel = _pPrefs->configuration()->getUInt(baseKey + ".channel", 1);
			std::string name = _pPrefs->configuration()->getString(baseKey + ".name", "");
			std::string logo = _pPrefs->configuration()->getString(baseKey + ".logo", "");
			Poco::UInt32 objectId = _pPrefs->configuration()->getUInt(baseKey + ".objectId", -1);

			std::string ip = _pPrefs->configuration()->getString(baseKey + ".ip", "");
			Poco::UInt16 port = _pPrefs->configuration()->getUInt(baseKey + ".port", 8000);
			std::string userName = _pPrefs->configuration()->getString(baseKey + ".userName", "");
			std::string password = _pPrefs->configuration()->getString(baseKey + ".password", "");
			Poco::UInt32 interval = _pPrefs->configuration()->getUInt(baseKey + ".interval", 1);

			//Poco::SharedPtr<Poco::Data::Session> pSession = new Session(MySQL::Connector::KEY, _dbConnString);

			std::string siteName;
			std::string objectName;
			if (_objectsMap.find(objectId) != _objectsMap.end()) {
				auto object = _objectsMap[objectId];
				auto siteId = object.siteId;
				objectName = object.name;
				if (_sitesMap.find(siteId) != _sitesMap.end()) {
					auto site = _sitesMap[siteId];
					siteName = site.name;
				}
			}
			auto device = std::make_shared<ActivityThermometer>(*this, id, channel, type, logo, ip, port, userName, password, interval, siteName, objectName);
			device->start();
			Poco::FastMutex::ScopedLock lock(_mutex);
			_sensors.emplace_back(device);
		}
		catch (Poco::Exception& exc) {
			_pContext->logger().error(Poco::format("Running thermometer device, id: '%s': %s", id, exc.displayText()));
		}
	}
}

void BundleActivator::setupSensorDataHandler()
{
	Poco::SharedPtr<Poco::Data::Session> pSession = new Session(MySQL::Connector::KEY, _dbConnString);
	_sensorDataHandler = std::make_shared<SensorDataHandler>(_queue, pSession);
	Poco::ThreadPool::defaultPool().start(*_sensorDataHandler);
}

void BundleActivator::onReportStatus(Poco::Timer& timer)
{
	std::vector<std::shared_ptr<SensorStatus>> statusList;
	for (const auto& device : _sensors) {
		auto data = std::make_shared<SensorStatus>();
		data->sensorId = device->id();
		data->status = device->status();
		statusList.emplace_back(data);
	}

	_queue.enqueueNotification(new xi::utils::SensorStatusNotification(statusList));
}

} // namespace XDBotThermometer
} // namespace xi


POCO_BEGIN_MANIFEST(Poco::OSP::BundleActivator)
	POCO_EXPORT_CLASS(xi::XDBotThermometer::BundleActivator)
POCO_END_MANIFEST
