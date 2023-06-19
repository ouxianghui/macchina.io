#include "Poco/SerialTaskScheduler.h"
#include <iostream>

namespace xi {

SerialTaskScheduler::SerialTaskScheduler() : TaskSchedulerImpl(1, 1)
{

}

SerialTaskScheduler::SerialTaskScheduler(const std::string& name) : TaskSchedulerImpl(name, 1, 1)
{
	
}

SerialTaskScheduler::~SerialTaskScheduler()
{
}

}