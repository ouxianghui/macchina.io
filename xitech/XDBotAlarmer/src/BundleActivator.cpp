//
// BundleActivator.cpp
//
// Copyright (c) 2019, Applied Informatics Software Engineering GmbH.
// All rights reserved.
//
// SPDX-License-Identifier: GPL-3.0-only
//

#include <iostream>
#include <memory>
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
#include "IoT/Devices/ISerialDevice.h"
#include "AlarmHandler.h"

namespace xi {
namespace XDBotAlarmer {

class BundleActivator: public Poco::OSP::BundleActivator
{
public:
	void start(Poco::OSP::BundleContext::Ptr pContext)
	{
		_pContext = pContext;
		_pPrefs = Poco::OSP::ServiceFinder::find<Poco::OSP::PreferencesService>(pContext);

		std::string deviceName("io.macchina.serialport#1");
		try {
			pContext->logger().information(Poco::format("Serial device '%s'.", deviceName));
			_pServiceRef = pContext->registry().findByName(deviceName);
			if (_pServiceRef) {
				_pSerialDevice = _pServiceRef->castedInstance<IoT::Devices::ISerialDevice>();
				_alarmHandler = std::make_unique<AlarmHandler>(_pSerialDevice);
				_alarmHandler->init();
			}
			else {
				_pContext->logger().warning("No serial device found.");
			}
		}
		catch (Poco::Exception& exc) {
			pContext->logger().error(Poco::format("serial device '%s': %s", deviceName, exc.displayText()));
		}
	}

	void stop(Poco::OSP::BundleContext::Ptr pContext)
	{
		_pSerialDevice.reset();
		_pPrefs.reset();
		_pContext.reset();
		if (_alarmHandler) {
			_alarmHandler->destroy();
		}
	}

private:
	Poco::OSP::BundleContext::Ptr _pContext;
	Poco::OSP::PreferencesService::Ptr _pPrefs;
	Poco::OSP::ServiceRef::Ptr _pServiceRef;
	IoT::Devices::ISerialDevice::Ptr _pSerialDevice;
	std::unique_ptr<AlarmHandler> _alarmHandler;
};

}
} // namespace xi


POCO_BEGIN_MANIFEST(Poco::OSP::BundleActivator)
	POCO_EXPORT_CLASS(xi::XDBotAlarmer::BundleActivator)
POCO_END_MANIFEST
