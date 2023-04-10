#pragma once

#include "IFrameCapturer.h"
#include <memory>
#include <vector>
#include "Poco/Activity.h"
#include "Observable.h"
#include "rkmedia_common.h"
#include "rkmedia_api.h"
#include "ffrtsp.hh"

namespace xi {
	class FrameCapturer : public IFrameCapturer, public Observable, public std::enable_shared_from_this<FrameCapturer> {
	public:
		FrameCapturer(const MPP_CHN_S& chn, const std::string& rtspUrl);

		~FrameCapturer();

		void init() override;

		void destroy() override;

		void addListener(std::shared_ptr<ICapturerListener> listener) override;

		void removeListener(std::shared_ptr<ICapturerListener> listener) override;

		void start() override;

		void stop() override;

	protected:
		void runRtsp();

		void runCapture();

		static int FFRKMediaVdecSend(u_int8_t* frameBuff, unsigned frameSize, bool* quit, int chn);

	private:
		const MPP_CHN_S& _rgaNNCHN;

		Poco::Activity<FrameCapturer> _activityRtsp;

		Poco::Activity<FrameCapturer> _activityCapture;

		std::vector<std::weak_ptr<ICapturerListener>> _observers;

		struct FFRTSPGet _rtspClient;

		std::string _rtspUrl;
	};
}