#pragma once

#include "Poco/OSP/Service.h"
#include "Poco/AutoPtr.h"

namespace xi {
	class IDetectionService: public Poco::OSP::Service
	{
	public:
		using Ptr = Poco::AutoPtr<IDetectionService>;

		virtual void init() = 0;

		virtual void destroy() = 0;
	};
}
