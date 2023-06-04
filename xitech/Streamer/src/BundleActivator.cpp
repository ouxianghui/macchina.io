//
// BundleActivator.cpp
//
// Copyright (c) 2018, Applied Informatics Software Engineering GmbH.
// All rights reserved.
//
// SPDX-License-Identifier: GPL-3.0-only
//


#include "Poco/OSP/BundleActivator.h"
#include "Poco/OSP/BundleContext.h"
#include "Poco/OSP/ServiceRegistry.h"
#include "Poco/OSP/ServiceRef.h"
#include "Poco/ClassLibrary.h"
#include "Poco/SharedLibrary.h"
#include "Poco/OSP/PreferencesService.h"
#include "Poco/OSP/ServiceFinder.h"

#include "broadcaster.hpp"
#include "mediasoupclient.hpp"
#include "engine.h"
#include "logger/u_logger.h"
#include "room_client.h"
#include "WebrtcStreamer.h"

// export LD_LIBRARY_PATH=~/Documents/dev/macchina.io/platform/lib/Linux/x86_64:~/Documents/dev/macchina.io/server/bin/Linux/x86_64/codeCache

namespace xi {

namespace streamer {

class BundleActivator: public Poco::OSP::BundleActivator
{
public:
	BundleActivator() {
		auto dylib = std::make_shared<Poco::SharedLibrary>("libRTCStreamer.so");
		_dylibs.emplace_back(dylib);
	}

	~BundleActivator() {
		for (const auto& lib : _dylibs) {
			lib->unload();
		}
	}

	void start(Poco::OSP::BundleContext::Ptr pContext) {

		_pContext = pContext;

		_pPrefs = Poco::OSP::ServiceFinder::find<Poco::OSP::PreferencesService>(pContext);

    	vi::ULogger::init();

		std::string level = "error";
		// Set RTC logging severity.
		if (level== "info") {
			rtc::LogMessage::LogToDebug(rtc::LoggingSeverity::LS_INFO);
		} else if (level == "warn") {
			rtc::LogMessage::LogToDebug(rtc::LoggingSeverity::LS_WARNING);
		} else if (level == "error") {
			rtc::LogMessage::LogToDebug(rtc::LoggingSeverity::LS_ERROR);
		}

		auto logLevel = mediasoupclient::Logger::LogLevel::LOG_DEBUG;
		mediasoupclient::Logger::SetLogLevel(logLevel);
		mediasoupclient::Logger::SetDefaultHandler();

		// Initilize mediasoupclient.
		mediasoupclient::Initialize();

    	getEngine()->init();

		Poco::Util::AbstractConfiguration::Keys keys;
		_pPrefs->configuration()->keys("xitech.streamer", keys);

		std::string serverUrl;
		std::string roomId;
		std::string name;
		bool enableAudio = false;
		bool useSimulcast = true;
		bool verifySsl = true;
		for (std::vector<std::string>::const_iterator it = keys.begin(); it != keys.end(); ++it) {
			try {
				std::string baseKey = "xitech.streamer.";
				baseKey += *it;

				serverUrl = _pPrefs->configuration()->getString(baseKey + ".serverUrl");
				roomId = _pPrefs->configuration()->getString(baseKey + ".roomId");
				name = _pPrefs->configuration()->getString(baseKey + ".name", "broadcaster");
				enableAudio = _pPrefs->configuration()->getBool(baseKey + ".enableAudio", false);
				useSimulcast = _pPrefs->configuration()->getBool(baseKey + ".useSimulcast", true);
				verifySsl = _pPrefs->configuration()->getBool(baseKey + ".verifySsl", true);
			}
			catch (Poco::Exception& exc) {
				_pContext->logger().error(Poco::format("Invalid configuration, name: %s,  %s", verifySsl, exc.displayText()));
			}
		}

		// xitech.streamer.0.serverUrl = "https://www.wevisit.cn:4443"
		// xitech.streamer.0.roomId = "jmucv-001"
		// xitech.streamer.0.name = "T0"
		// xitech.streamer.0.enableAudio = false
		// xitech.streamer.0.useSimulcast = true
		// xitech.streamer.0.verifySsl = true

		run();

	}

	void stop(Poco::OSP::BundleContext::Ptr pContext) {
		getEngine()->destroy();

		mediasoupclient::Cleanup();

		vi::ULogger::destroy();

		_pPrefs.reset();

		_pContext.reset();
	}

private:
	Poco::OSP::BundleContext::Ptr _pContext;

	Poco::OSP::PreferencesService::Ptr _pPrefs;

	std::vector<std::shared_ptr<Poco::SharedLibrary>> _dylibs;
};


} // namespace xi
} // namespace streamer


POCO_BEGIN_MANIFEST(Poco::OSP::BundleActivator)
	POCO_EXPORT_CLASS(xi::streamer::BundleActivator)
POCO_END_MANIFEST
