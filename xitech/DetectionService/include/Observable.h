#pragma once

#include <memory>
#include <functional>
#include <algorithm>
#include <vector>
#include <mutex>

namespace xi {

    class Observable
    {
    public:
        virtual ~Observable(){}

    protected:
        template<class T>
        void addObserver(std::vector<std::weak_ptr<T>>& observers, std::weak_ptr<T> observer)
        {
            auto lObserver = observer.lock();
            if(lObserver) {
                std::lock_guard<std::mutex> lock(_mutex);
                observers.push_back(observer);
            }
        }

        template<class T> 
        void removeObserver(std::vector<std::weak_ptr<T>>& observers, std::weak_ptr<T> observer)
        {
            std::lock_guard<std::mutex> lock(_mutex);
            typename std::vector<std::weak_ptr<T>>::iterator iter = observers.begin();
            auto lockObserver = observer.lock();
            if (lockObserver == nullptr) {
                return;
            }

            while (iter != observers.end()) {
                if (iter->expired()) {
                    iter = observers.erase(iter);
                    continue;
                }
                if (iter->lock().get() == lockObserver.get()) {
                    break;
                }
                ++iter;
            }
            if (iter != observers.end()) {
                observers.erase(iter);
            }
        }

        template<class T>
        void removeInvalidObserver(std::vector<std::weak_ptr<T>>& observers)
        {
            observers.erase(std::remove_if(observers.begin(), observers.end(), [](const std::weak_ptr<T>& observer) {
                                                return observer.expired();
                                            }), observers.end());
        }

        template<class T>
        void notifyObserver(std::vector<std::weak_ptr<T>>& observers, std::function<void(std::shared_ptr<T>& ot)> func)
        {
            std::vector<std::weak_ptr<T>> copiedObservers;
            {
                std::lock_guard<std::mutex> lock(_mutex);
                removeInvalidObserver(observers);
                copiedObservers = observers;
            }
            for(auto it = copiedObservers.begin(); it != copiedObservers.end(); ++it) {
                auto observer = (*it).lock();
                if (observer) {
                    func(observer);
                }
            }
        }

    private:
        std::mutex _mutex;
    };

}
