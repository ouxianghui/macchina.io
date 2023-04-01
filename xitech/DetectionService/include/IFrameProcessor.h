#pragma once

#include <memory>

namespace xi {
	class IProcessorListener {
	public:
		virtual ~IProcessorListener() = default;

		virtual void onFrameProcessed() = 0;
	};

	class IFrameProcessor {
	public:
		virtual ~IFrameProcessor() = default;

		virtual void init() = 0;

		virtual void destroy() = 0;

		virtual void addListener(std::shared_ptr<IProcessorListener> listener) = 0;

		virtual void removeListener(std::shared_ptr<IProcessorListener> listener) = 0;

		virtual void start() = 0;

		virtual void stop() = 0;

	};
}