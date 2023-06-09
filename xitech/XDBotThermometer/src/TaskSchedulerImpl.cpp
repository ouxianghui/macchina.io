#include "TaskSchedulerImpl.h"
#include <iostream>
#include <thread>

namespace xi {
namespace XDBotThermometer {	

class RunnableHolder: public Poco::Runnable {
public:
	RunnableHolder(Poco::Runnable& target) : _target(target) {}

	~RunnableHolder() {}

	void run() { _target.run(); }

private:
	Poco::Runnable& _target;
};

class CallableHolder: public Poco::Runnable {
public:
	CallableHolder(Callable callable, void* pData) : _callable(callable), _pData(pData) {}

	~CallableHolder() {}

	void run() { _callable(_pData); }

private:
	Callable _callable;

	void* _pData;
};

class TaskNotification : public Poco::Notification {
public:
	TaskNotification(Poco::SharedPtr<Poco::Runnable> task) : _task(task) {}
	
	~TaskNotification() {}
	
	Poco::SharedPtr<Poco::Runnable> task() { return _task; }
	
private:
	Poco::SharedPtr<Poco::Runnable> _task;
};

TaskSchedulerImpl::TaskSchedulerImpl(int minCapacity, int maxCapacity, int idleTime, int stackSize)
: _threadPool(new Poco::ThreadPool(minCapacity, maxCapacity, idleTime, stackSize))
{

}

TaskSchedulerImpl::TaskSchedulerImpl(const std::string& name, int minCapacity, int maxCapacity, int idleTime, int stackSize)
: _threadPool(new Poco::ThreadPool(name, minCapacity, maxCapacity, idleTime, stackSize))
{
	
}

TaskSchedulerImpl::~TaskSchedulerImpl()
{
	stop();
}

std::string TaskSchedulerImpl::name() const
{
	return _threadPool->name();
}

void TaskSchedulerImpl::start()
{
	for (int32_t i = 0; i < _threadPool->capacity(); ++i) {
		_threadPool->start(*this);
	}
}

void TaskSchedulerImpl::stop()
{
	_queue.wakeUpAll();
	_threadPool->stopAll();
}

void TaskSchedulerImpl::dispatch(Poco::Runnable& target)
{
	dispatch(new RunnableHolder(target));
}

void TaskSchedulerImpl::dispatch(Callable target, void* pData)
{
	dispatch(new CallableHolder(target, pData));
}

void TaskSchedulerImpl::dispatch(Poco::SharedPtr<Poco::Runnable> target)
{
	dispatchImpl(target);
}

void TaskSchedulerImpl::dispatchImpl(Poco::SharedPtr<Poco::Runnable> target)
{
	_queue.enqueueNotification(new TaskNotification(target));
}

void TaskSchedulerImpl::run() 
{
	Poco::AutoPtr<Poco::Notification> pNf(_queue.waitDequeueNotification());
	while (pNf) {
		if (TaskNotification* pTaskNf = dynamic_cast<TaskNotification*>(pNf.get())) {
			if (auto task = pTaskNf->task()) {
				task->run();
			}
		}
		pNf = _queue.waitDequeueNotification();
	}
}

}
}