#pragma once

#include <memory>

namespace xi {

	class IFrameRenderer {
	public:
		virtual ~IFrameRenderer() = default;

		virtual void init() = 0;

		virtual void destroy() = 0;

		virtual void renderFrame(void* buffer, int32_t length) = 0;

	};
}