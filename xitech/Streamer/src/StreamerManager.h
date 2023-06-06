#pragma once

#include <memory>
#include <mutex>
#include <vector>
#include "IStreamer.h"
#include "i_room_client_observer.h"
#include "options.h"

namespace xi {

namespace streamer {

    class StreamerManager : public vi::IRoomClientObserver, public std::enable_shared_from_this<StreamerManager> 
    {
    public:
        StreamerManager(std::string host, int32_t port, std::string roomId);

        ~StreamerManager();

        void init();

        void destroy();

        void start();

        void stop();

        void addStreamer(const std::shared_ptr<IStreamer>& streamer);

        void removeStreamer(const std::shared_ptr<IStreamer>& streamer);

    protected:
        void publish();

        void unpublish();

    private:
        void onRoomStateChanged(vi::RoomState state) override;

        void onCreateLocalVideoTrack(const std::string& tid, rtc::scoped_refptr<webrtc::MediaStreamTrackInterface>) override {}

        void onRemoveLocalVideoTrack(const std::string& tid, rtc::scoped_refptr<webrtc::MediaStreamTrackInterface>) override {}

        void onLocalAudioStateChanged(bool enabled, bool muted) override {}

        void onLocalVideoStateChanged(bool enabled) override {}

        void onLocalActiveSpeaker(int32_t volume) override {}

    private:
        std::mutex _mutex;

        std::vector<std::shared_ptr<IStreamer>> _streamers;

        std::string _clientId;

        std::string _host;

        int32_t _port;
        
        std::string _roomId;

        std::shared_ptr<vi::Options> _options;
    };

}

}

int run();