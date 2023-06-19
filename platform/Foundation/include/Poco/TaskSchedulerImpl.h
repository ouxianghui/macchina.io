#pragma once

#include <memory>
#include <string>
#include <Poco/SharedPtr.h>
#include "Poco/ThreadPool.h"
#include "Poco/NotificationQueue.h"
#include "Poco/Runnable.h"

namespace xi {

typedef void (*Callable)(void*);

class TaskSchedulerImpl : public Poco::Runnable {
public:
	TaskSchedulerImpl(int minCapacity = 2, int maxCapacity = 16, int idleTime = 60, int stackSize = POCO_THREAD_STACK_SIZE);

	TaskSchedulerImpl(const std::string& name, int minCapacity = 2, int maxCapacity = 16, int idleTime = 60, int stackSize = POCO_THREAD_STACK_SIZE);

	~TaskSchedulerImpl();

	std::string name() const;

	void start();

	void stop();

	void dispatch(Poco::Runnable& target);

	void dispatch(Callable target, void* pData = 0);

	void dispatch(Poco::SharedPtr<Poco::Runnable> target);

	template <class Functor>
	void dispatchFunc(const Functor& fn) { dispatchImpl(new FunctorRunnable<Functor>(fn)); }

	template <class Functor>
	void dispatchFunc(Functor&& fn) { dispatchImpl(new FunctorRunnable<Functor>(std::move(fn))); }

protected:
	void run() override;

	void dispatchImpl(Poco::SharedPtr<Poco::Runnable> target);

protected:
	template <class Functor>
	class FunctorRunnable : public Poco::Runnable {
	public:
		FunctorRunnable(const Functor& functor) : _functor(functor) {}

		FunctorRunnable(Functor&& functor) : _functor(std::move(functor)) {}

		~FunctorRunnable() {}

		void run() override { _functor(); }

	private:
		Functor _functor;
	};

private:
	Poco::SharedPtr<Poco::ThreadPool> _threadPool;

	Poco::NotificationQueue _queue;
};	

}