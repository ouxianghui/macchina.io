DROP DATABASE IF EXISTS xdbot_db;

CREATE DATABASE IF NOT EXISTS xdbot_db DEFAULT CHARACTER SET UTF8 COLLATE UTF8_GENERAL_CI;

use xdbot_db;

create table Site
(
    siteId INT UNSIGNED PRIMARY KEY KEY NOT NULL,
    name VARCHAR(128) NOT NULL
);

create table Object
(
    objectId INT UNSIGNED PRIMARY KEY NOT NULL,
    name VARCHAR(128) NOT NULL,
    siteId INT UNSIGNED NOT NULL,
    FOREIGN KEY(siteId) REFERENCES Site(siteId) ON UPDATE CASCADE
);

create table Sensor
(
    sensorId INT UNSIGNED PRIMARY KEY NOT NULL,
    name VARCHAR(128) NOT NULL,
    type INT UNSIGNED NOT NULL,
    logo VARCHAR(64) NOT NULL,
    objectId INT UNSIGNED NOT NULL,
    FOREIGN KEY(objectId) REFERENCES Object(objectId) ON UPDATE CASCADE
);

create table SensorData
(
    sensorId INT UNSIGNED NOT NULL,
    channel INT UNSIGNED NOT NULL,
    ruleId INT UNSIGNED,
    ruleName VARCHAR(64),
    ruleCalibType INT UNSIGNED,
    currVal float,
    minVal float,
    maxVal float,
    avgVal float,
    diffVal float,
    timestamp DATETIME,
    recordId BIGINT UNSIGNED PRIMARY KEY NOT NULL AUTO_INCREMENT,
    FOREIGN KEY(sensorId) REFERENCES Sensor(sensorId) ON UPDATE CASCADE
);

create index index_1 on SensorData(timestamp, sensorId);

create table AlarmData
(
    sensorId INT UNSIGNED NOT NULL,    
    type INT UNSIGNED NOT NULL,
    channel INT UNSIGNED NOT NULL,
    ruleId INT UNSIGNED,
    alarmType INT UNSIGNED,
    alarmLevel INT UNSIGNED,
    alarmRule INT UNSIGNED,
    unit INT UNSIGNED,
    ruleValue float,
    currValue float,
    suddenChangeCycle INT UNSIGNED,
    suddenChangeValue float,
    begin DATETIME,
    end DATETIME,
    recordId BIGINT UNSIGNED PRIMARY KEY NOT NULL AUTO_INCREMENT,
    FOREIGN KEY(sensorId) REFERENCES Sensor(sensorId) ON UPDATE CASCADE
);

create index index_1 on AlarmData(begin, sensorId, alarmType, alarmLevel);

create table DetectionData
(
    devCode VARCHAR(16) NOT NULL,   
    devName VARCHAR(64), 
    valueType VARCHAR(16), 
    value VARCHAR(16), 
    valueCN VARCHAR(64), 
    recResult INT UNSIGNED,
    recReason VARCHAR(64),
    imageUrl VARCHAR(256),
    eventTime DATETIME,
    recordId BIGINT UNSIGNED PRIMARY KEY NOT NULL AUTO_INCREMENT
);

create index index_1 on DetectionData(devCode, devName);
