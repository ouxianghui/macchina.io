#pragma once

#include <memory>
#include "Poco/Activity.h"
#include "IStreamProvider.h"
#include "IFrameCapturer.h"
#include "rkmedia_api.h"

namespace xi {

	class RtspStreamProvider : public IStreamProvider, public ICapturerListener, public std::enable_shared_from_this<RtspStreamProvider> {
	public:
		RtspStreamProvider(const MPP_CHN_S& chn);

		~RtspStreamProvider();

		void init(std::shared_ptr<IFrameRenderer> renderer) override;

		void destroy() override;

		void start() override;

		void stop() override;

	protected:
		void runActivity();

		// ICapturerListener
		void onFrame(void* buffer, int32_t length) override;

	private:
	const MPP_CHN_S& _vencCHN;

		Poco::Activity<RtspStreamProvider> _activity;

		std::shared_ptr<IFrameRenderer> _renderer;
	};
}