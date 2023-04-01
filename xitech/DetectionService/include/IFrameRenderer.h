#pragma once

#include <memory>

namespace xi {

	class IFrameRenderer {
	public:
		virtual ~IFrameRenderer() = default;

		virtual void init() = 0;

		virtual void destroy() = 0;

		virtual void start() = 0;

		virtual void stop() = 0;

	};
}