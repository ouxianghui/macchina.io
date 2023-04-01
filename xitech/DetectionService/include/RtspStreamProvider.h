#pragma once

#include <memory>
#include "Poco/Activity.h"
#include "rkmedia_api.h"
#include "IStreamProvider.h"

namespace xi {

	class RtspStreamProvider : public IStreamProvider, public std::enable_shared_from_this<RtspStreamProvider> {
	public:
		RtspStreamProvider(const MPP_CHN_S& chn);

		~RtspStreamProvider();

		void init() override;

		void destroy() override;

		void start() override;

		void stop() override;

	protected:
		void runActivity();

	private:
		const MPP_CHN_S& _vencCHN;

		Poco::Activity<RtspStreamProvider> _activity;
	};
}