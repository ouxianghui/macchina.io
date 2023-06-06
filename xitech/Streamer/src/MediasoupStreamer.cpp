#include "MediasoupStreamer.h"
#include "broadcaster.hpp"
#include "mediasoupclient.hpp"
#include <cpr/cpr.h>
#include <cstdlib>
#include <iostream>
#include <string>
#include "engine.h"
#include "logger/u_logger.h"
#include "room_client.h"

using json = nlohmann::json;

namespace xi {

namespace streamer {

    MediasoupStreamer::MediasoupStreamer(const std::shared_ptr<PublishParams>& params) 
    : _activity(this, &MediasoupStreamer::runActivity)
    , _params(params)
    {

    }

    MediasoupStreamer::~MediasoupStreamer()
    {

    }
    
    void MediasoupStreamer::init() 
    {

    }

    void MediasoupStreamer::destroy() 
    {
        _activity.stop();
        _activity.wait();
    }

    void MediasoupStreamer::publish() 
    {
        _activity.start();
    }

    void MediasoupStreamer::unpublish() 
    {
        _activity.stop();
	    _activity.wait();
    }

    void MediasoupStreamer::runActivity() 
    {
        if (!_params) {
            return;
        }

        std::string baseUrl = _params->baseUrl;

        baseUrl.append("/rooms/").append(_params->roomId);

        auto r = cpr::GetAsync(cpr::Url{ baseUrl }, cpr::VerifySsl{ _params->verifySsl }).get();

        if (r.status_code != 200) {
            std::cerr << "[ERROR] unable to retrieve room info" << " [status code:" << r.status_code << ", body:\"" << r.text << "\"]" << std::endl;
            return;
        } 
        else {
            std::cout << "[INFO] found room" << _params->roomId << std::endl;
        }

        auto response = nlohmann::json::parse(r.text);

        auto broadcaster = getEngine()->createBroadcaster();
        
        broadcaster->Start(baseUrl, _params->videoUrl, _params->displayName, _params->deviceName, _params->enableAudio, _params->useSimulcast, _params->verifySsl, response);

        while (!_activity.isStopped()) {
		    Poco::Thread::sleep(2000);
	    }
    }

}

}