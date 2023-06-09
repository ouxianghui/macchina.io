#include "SerialTaskScheduler.h"
#include <iostream>

namespace xi {
namespace XDBotThermometer {	


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
}