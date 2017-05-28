//
//  TaskDispatcher.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/28/17.
//
//

#include "TaskDispatcher.hpp"
#include <algorithm>

TaskDispatcher::TaskDispatcher() : _threadShouldExit(false)
{
    for (unsigned i = 0, numTheads = std::max(1u, std::thread::hardware_concurrency()); i < numTheads; ++i) {
        _threads.emplace_back([this]{
            worker();
        });
    }
}

TaskDispatcher::~TaskDispatcher()
{
    _threadShouldExit = true;
    _cv.notify_all();
    for (std::thread &thread : _threads) {
        thread.join();
    }
}

void TaskDispatcher::worker()
{
    while (true) {
        Task task;
        
        {
            std::unique_lock<std::mutex> lock(_lockTasks);
            _cv.wait(lock, [this]{
                return _threadShouldExit || !_tasks.empty();
            });
            
            if (_threadShouldExit) {
                return;
            }
            
            if (_tasks.empty()) {
                continue;
            }
            
            task = std::move(_tasks.front());
            _tasks.pop();
        }
        
        task();
    }
}

void TaskDispatcher::async(Task &&task)
{
    if (ForceSerialDispatch) {
        task();
    } else {
        {
            std::unique_lock<std::mutex> lock(_lockTasks);
            _tasks.push(task);
        }
        _cv.notify_one();
    }
}
