#pragma once

#include <memory>

namespace xi {

	class IFrameRenderer;
	class IStreamProvider {
	public:
		virtual ~IStreamProvider() = default;

		virtual void init(std::shared_ptr<IFrameRenderer> renderer) = 0;

		virtual void destroy() = 0;

		virtual void start() = 0;

		virtual void stop() = 0;

	};
}