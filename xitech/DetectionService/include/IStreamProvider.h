#pragma once

namespace xi {

	class IStreamProvider {
	public:
		virtual ~IStreamProvider() = default;

		virtual void init() = 0;

		virtual void destroy() = 0;

		virtual void start() = 0;

		virtual void stop() = 0;

	};
}