#pragma once

#include "IStreamer.h"
#include "Poco/Activity.h"

namespace xi {

namespace streamer {

    class MediasoupStreamer : public IStreamer {
    public:
        MediasoupStreamer(const std::shared_ptr<PublishParams>& params);

        ~MediasoupStreamer();

        void init() override;

        void destroy() override;

        void publish() override;

        void unpublish() override;

    private:
	    void runActivity(); 

    private:
        Poco::Activity<MediasoupStreamer> _activity;

        std::shared_ptr<PublishParams> _params;
    };

}

}