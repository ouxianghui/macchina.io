#pragma once

#include "IFrameRenderer.h"
#include <memory>
#include <vector>
#include "Poco/Activity.h"
#include "rkmedia_common.h"
#include "rkmedia_api.h"


namespace xi {

	class RknnFrameRenderer : public IFrameRenderer, public std::enable_shared_from_this<RknnFrameRenderer> {
	public:
		RknnFrameRenderer(const MPP_CHN_S& vencCHN, const MPP_CHN_S& drawCHN);

		~RknnFrameRenderer();

		void init() override;

		void destroy() override;

		void start() override;

		void stop() override;

	protected:
		void runActivity();

	private:
		const MPP_CHN_S& _rgaVencCHN;

		const MPP_CHN_S& _rgaDrawCHN;

		Poco::Activity<RtspFrameCapturer> _activity;

	};

}