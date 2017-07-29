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
    for (unsigned i = 0, numTheads = std::max(1u, 2*std::thread::hardware_concurrency()); i < numTheads; ++i) {
        _threads.emplace_back([this]{
            worker();
        });
    }
}

TaskDispatcher::~TaskDispatcher()
{
    shutdown();
}

void TaskDispatcher::shutdown()
{
    _threadShouldExit = true;
    _cvarTaskPosted.notify_all();
    for (std::thread &thread : _threads) {
        thread.join();
    }
    _threads.clear();
}

void TaskDispatcher::async(Task &&task)
{
    {
        std::unique_lock<std::mutex> lock(_lockTaskPosted);
        _tasks.push(task);
    }
    _cvarTaskPosted.notify_one();
}

void TaskDispatcher::worker()
{
    while (true) {
        Task task;
        
        {
            std::unique_lock<std::mutex> lock(_lockTaskPosted);
            _cvarTaskPosted.wait(lock, [this]{
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
