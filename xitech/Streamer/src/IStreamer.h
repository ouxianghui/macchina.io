#pragma once

#include <string>
#include <memory>

namespace xi {

namespace streamer {

    struct PublishParams {
        /* data */
        std::string baseUrl;
        std::string videoUrl;
        std::string roomId;
        std::string displayName;
        std::string deviceName;
        bool enableAudio;
        bool useSimulcast;
        bool verifySsl;
    };
    
    class IStreamer {
    public:
        virtual ~IStreamer() = default;

        virtual void init() = 0;

        virtual void destroy() = 0;

        virtual void publish() = 0;

        virtual void unpublish() = 0;
    };

}

}