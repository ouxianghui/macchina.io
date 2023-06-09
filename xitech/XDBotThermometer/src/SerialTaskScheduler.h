#pragma once

#include <stdint.h>
#include <string>
#include "Poco/Thread.h"
#include "Poco/NotificationQueue.h"
#include "Poco/Runnable.h"
#include "TaskSchedulerImpl.h"

namespace xi {
namespace XDBotThermometer {

typedef void (*Callable)(void*);

class SerialTaskScheduler : public TaskSchedulerImpl {
public:
	SerialTaskScheduler();

	SerialTaskScheduler(const std::string& name);

	~SerialTaskScheduler();
};	

}
}