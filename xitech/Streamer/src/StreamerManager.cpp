#include "StreamerManager.h"
#include "engine.h"
#include "logger/u_logger.h"
#include "room_client.h"
#include "utils/thread_provider.h"

namespace xi {

namespace streamer {

    StreamerManager::StreamerManager(std::string host, int32_t port, std::string roomId)
    : _host(host)
    , _port(port)
    , _roomId(roomId)
    {
        _options = std::make_shared<vi::Options>();

        _options->useSimulcast = false;
        _options->useSharingSimulcast = false;
        _options->forceTcp = false;
        _options->produce = false;
        _options->consume = false;
        _options->datachannel = false;
    }

    StreamerManager::~StreamerManager()
    {

    }

    void StreamerManager::init()
    {
        if (auto client = getEngine()->createRoomClient()) {
            _clientId = client->getId();
            if (rtc::Thread* callbackThread = getThread("mediasoup")) {
                client->addObserver(shared_from_this(), callbackThread);
            }
        }
    }

    void StreamerManager::destroy()
    {
        stop();

        std::lock_guard<std::mutex> guard(_mutex);
        _streamers.clear();
    }

    void StreamerManager::start()
    {
        if (auto client = getRoomClient(_clientId)) {
            client->join(_host, _port, _roomId, "Host", _options);
        }

        //publish();
    }

    void StreamerManager::stop()
    {
        unpublish();

        if (auto client = getRoomClient(_clientId)) {
            client->leave();
        }
    }

    void StreamerManager::addStreamer(const std::shared_ptr<IStreamer>& streamer)
    {
        std::lock_guard<std::mutex> guard(_mutex);
        auto it = std::find_if(_streamers.begin(), _streamers.end(), [streamer](const auto& e) {
            return e == streamer;
        });

        if (it == _streamers.end()) {
            _streamers.emplace_back(streamer);
        }
    }

    void StreamerManager::removeStreamer(const std::shared_ptr<IStreamer>& streamer)
    {
        std::lock_guard<std::mutex> guard(_mutex);
        for (auto it = _streamers.begin(); it != _streamers.end(); ++it) {
            if (*it == streamer) {
                _streamers.erase(it);
                break;
            }
        }
    }

    void StreamerManager::publish()
    {
        std::lock_guard<std::mutex> guard(_mutex);
        for (auto it = _streamers.begin(); it != _streamers.end(); ++it) {
            (*it)->publish();
        }
    }

    void StreamerManager::unpublish()
    {
        std::lock_guard<std::mutex> guard(_mutex);
        for (auto it = _streamers.begin(); it != _streamers.end(); ++it) {
            (*it)->unpublish();
        }
    }

    void StreamerManager::onRoomStateChanged(vi::RoomState state)
    {
        if (state == vi::RoomState::CONNECTED) {
            std::cerr << " StreamerManager::onRoomStateChanged: connected" << std::endl;
            publish();
        }
        else if (state == vi::RoomState::CLOSED) {
            unpublish();
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
    }

}

}