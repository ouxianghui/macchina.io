#include "Poco/ConcurrentTaskScheduler.h"
#include <iostream>

namespace xi {
	
ConcurrentTaskScheduler::ConcurrentTaskScheduler() : TaskSchedulerImpl(5, 5)
{

}

ConcurrentTaskScheduler::ConcurrentTaskScheduler(int32_t workerNum) : TaskSchedulerImpl(workerNum, workerNum)
{

}

ConcurrentTaskScheduler::ConcurrentTaskScheduler(const std::string& name, int32_t workerNum) : TaskSchedulerImpl(name, workerNum, workerNum)
{
	
}

ConcurrentTaskScheduler::~ConcurrentTaskScheduler()
{

}

}