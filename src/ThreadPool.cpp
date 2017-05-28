//
//  ThreadPool.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/28/17.
//
//

#include "ThreadPool.hpp"
#include <algorithm>

ThreadPool g_threadPool;

ThreadPool::ThreadPool() : _threadShouldExit(false)
{
    for (unsigned i = 0, numTheads = std::max(1u, std::thread::hardware_concurrency()); i < numTheads; ++i) {
        _threads.emplace_back([this]{
            worker();
        });
    }
}

ThreadPool::~ThreadPool()
{
    _threadShouldExit = true;
    _cv.notify_all();
    for (std::thread &thread : _threads) {
        thread.join();
    }
}

void ThreadPool::worker()
{
    while (true) {
        std::function<void()> job;
        
        {
            std::unique_lock<std::mutex> lock(_lockJobs);
            _cv.wait(lock, [this]{
                return _threadShouldExit || !_jobs.empty();
            });
            
            if (_threadShouldExit) {
                return;
            }
            
            if (_jobs.empty()) {
                continue;
            }
            
            job = std::move(_jobs.front());
            _jobs.pop();
        }
        
        job();
    }
}

void ThreadPool::enqueue(std::function<void()> &&job)
{
    {
        std::unique_lock<std::mutex> lock(_lockJobs);
        _jobs.push(job);
    }
    _cv.notify_one();
}
