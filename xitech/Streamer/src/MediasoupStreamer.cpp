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
        if (!_broadcaster) {
            _broadcaster = getEngine()->createBroadcaster();
            if (auto thread = getThread("mediasoup")) {
                _broadcaster->addListener(shared_from_this(), thread);
            }
        }
    }

    void MediasoupStreamer::destroy() 
    {
        if (_activity.isRunning()) {
            _activity.stop();
            _activity.wait();
        }

        if (!_broadcaster) {
            _broadcaster = getEngine()->createBroadcaster();
            _broadcaster->removeListener(shared_from_this());
        }
    }

    void MediasoupStreamer::publish() 
    {
        if (_activity.isStopped()) {
            _activity.start();
        }
    }

    void MediasoupStreamer::unpublish() 
    {
        if (_activity.isRunning()) {
            _activity.stop();
            _activity.wait();
        }
    }

    void MediasoupStreamer::republish()
    {
        Poco::Timestamp tsNow;
        _timer.schedule(Poco::Util::Timer::func([wself = weak_from_this()]() {
            if (auto self = wself.lock()) {
                self->publish();
            }
	    }), tsNow + 1000 * 1000 * 5);

        //if (auto thread = getThread("mediasoup")) {
        //    thread->PostDelayedTask(RTC_FROM_HERE, [wself = weak_from_this()](){
        //        if (auto self = wself.lock()) {
        //            self->publish();
        //        }
        //    }, 5000);
        //}
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
            republish();
            return;
        } 
        else {
            std::cout << "[INFO] found room" << _params->roomId << std::endl;
        }

        auto response = nlohmann::json::parse(r.text);
        
        _broadcaster->Start(baseUrl, _params->videoUrl, _params->displayName, _params->deviceName, _params->enableAudio, _params->useSimulcast, _params->verifySsl, response);

        while (!_activity.isStopped()) {
		    Poco::Thread::sleep(2000);
	    }

        _broadcaster->Stop();
    }

    void MediasoupStreamer::onStatusChanged(BroadcasterStatus status)
    {
        if (status == BroadcasterStatus::Closed) {
            republish();
        }
    }

}

}