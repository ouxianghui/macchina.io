#pragma once

#include <memory>

namespace xi {

	class ICapturerListener {
	public:
		virtual ~ICapturerListener() = default;

		virtual void onFrame(void* buffer, int32_t length) = 0;
	};

	class IFrameCapturer {
	public:
		virtual ~IFrameCapturer() = default;

		virtual void init() = 0;

		virtual void destroy() = 0;

		virtual void addListener(std::shared_ptr<ICapturerListener> listener) = 0;

		virtual void removeListener(std::shared_ptr<ICapturerListener> listener) = 0;

		virtual void start() = 0;

		virtual void stop() = 0;

	};
}