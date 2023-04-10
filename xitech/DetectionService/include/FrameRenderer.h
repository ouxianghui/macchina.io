#pragma once

#include "IFrameRenderer.h"
#include <memory>


namespace xi {

	class FrameRenderer : public IFrameRenderer, public std::enable_shared_from_this<FrameRenderer> {
	public:
		FrameRenderer();

		~FrameRenderer();

		void init() override;

		void destroy() override;

		void renderFrame(void* buffer, int32_t length) override;

	};

}