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

class ConcurrentTaskScheduler : public TaskSchedulerImpl {
public:
	ConcurrentTaskScheduler();

	ConcurrentTaskScheduler(int32_t workerNum);

	ConcurrentTaskScheduler(const std::string& name, int32_t workerNum);

	~ConcurrentTaskScheduler();
};	

}
}