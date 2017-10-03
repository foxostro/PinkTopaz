//
//  TaskDispatcher.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/28/17.
//
//

#include "TaskDispatcher.hpp"
#include <algorithm>

TaskDispatcher::TaskDispatcher() : TaskDispatcher(std::max(1u, 2*std::thread::hardware_concurrency())) {}

TaskDispatcher::TaskDispatcher(unsigned numThreads) : _threadShouldExit(false)
{
    for (unsigned i = 0; i < numThreads; ++i) {
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
    
    while (!_tasks.empty()) {
        std::shared_ptr<AbstractTask> task(_tasks.front());
        _tasks.pop();
        task->cancel();
    }
    
    _cvarTaskPosted.notify_all();
    for (std::thread &thread : _threads) {
        thread.join();
    }
    _threads.clear();
}

void TaskDispatcher::worker()
{
    while (true) {
        std::shared_ptr<AbstractTask> taskPtr;
        
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
            
            taskPtr = _tasks.front();
            _tasks.pop();
        }
        
        assert(taskPtr);
        taskPtr->execute();
    }
}
