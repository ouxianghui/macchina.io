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
#include "StreamerManager.h"
#include "MediasoupStreamer.h"

// make -s -j8 DEFAULT_TARGET=shared_release
// export LD_LIBRARY_PATH=~/Documents/dev/macchina.io/platform/lib/Linux/x86_64:~/Documents/dev/macchina.io/server/bin/Linux/x86_64/codeCache

namespace xi {

namespace streamer {

class BundleActivator: public Poco::OSP::BundleActivator
{
public:
	BundleActivator() 
	{
		auto dylib = std::make_shared<Poco::SharedLibrary>("libRTCStreamer.so");
		_dylibs.emplace_back(dylib);
	}

	~BundleActivator() 
	{
		for (const auto& lib : _dylibs) {
			lib->unload();
		}
	}

	void start(Poco::OSP::BundleContext::Ptr pContext) 
	{

		_pContext = pContext;

		_pPrefs = Poco::OSP::ServiceFinder::find<Poco::OSP::PreferencesService>(pContext);

    	vi::ULogger::init();

		auto logLevel = mediasoupclient::Logger::LogLevel::LOG_DEBUG;
		mediasoupclient::Logger::SetLogLevel(logLevel);
		mediasoupclient::Logger::SetDefaultHandler();

		// Initilize mediasoupclient.
		mediasoupclient::Initialize();

    	getEngine()->init();

		getEngine()->setRTCLoggingSeverity("error");

		std::string host = _pPrefs->configuration()->getString("xitech.sfu.host");
		int32_t port  = _pPrefs->configuration()->getInt("xitech.sfu.port", 4443);
		std::string roomId = _pPrefs->configuration()->getString("xitech.sfu.roomId");

		_streamerManager = std::make_shared<StreamerManager>(host, port, roomId);
		_streamerManager->init();

		std::string baseUrl = "https://" + host + ":" + std::to_string(port);

		_pContext->logger().information(Poco::format("baseUrl: %s, roomId: %s", baseUrl, roomId));

		Poco::Util::AbstractConfiguration::Keys keys;
		_pPrefs->configuration()->keys("xitech.streamer", keys);

		std::shared_ptr<PublishParams> params;
		for (std::vector<std::string>::const_iterator it = keys.begin(); it != keys.end(); ++it) {
			params = std::make_shared<PublishParams>();
			try {
				std::string baseKey = "xitech.streamer.";
				baseKey += *it;

				params->baseUrl = baseUrl;
				params->roomId = roomId;
				params->videoUrl = _pPrefs->configuration()->getString(baseKey + ".videoUrl");
				params->displayName = _pPrefs->configuration()->getString(baseKey + ".name", "broadcaster");
				params->deviceName = _pPrefs->configuration()->getString(baseKey + ".deviceName");
				params->enableAudio = _pPrefs->configuration()->getBool(baseKey + ".enableAudio", false);
				params->useSimulcast = _pPrefs->configuration()->getBool(baseKey + ".useSimulcast", true);
				params->verifySsl = _pPrefs->configuration()->getBool(baseKey + ".verifySsl", true);

				auto streamer = std::make_shared<MediasoupStreamer>(params);
				streamer->init();
				_streamerManager->addStreamer(streamer);

			}
			catch (Poco::Exception& exc) {
				_pContext->logger().error(Poco::format("Invalid configuration, name: %s,  %s", params->roomId, exc.displayText()));
			}
		}
		_streamerManager->start();
	}

	void stop(Poco::OSP::BundleContext::Ptr pContext) 
	{
		_streamerManager->destroy();

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

	std::shared_ptr<StreamerManager> _streamerManager;
};


} // namespace xi
} // namespace streamer


POCO_BEGIN_MANIFEST(Poco::OSP::BundleActivator)
	POCO_EXPORT_CLASS(xi::streamer::BundleActivator)
POCO_END_MANIFEST
