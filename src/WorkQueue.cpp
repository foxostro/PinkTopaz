//
//  WorkQueue.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 6/25/16.
//
//

#include "WorkQueue.hpp"

namespace PinkTopaz {
    
    WorkQueue::WorkQueue() : shouldShutdown(false)
    {
        // Nothing to do
    }
    
    WorkQueue::~WorkQueue()
    {
        setShouldShutdown();
        join();
    }
    
    void WorkQueue::setShouldShutdown()
    {
        shouldShutdown = true;
    }
    
    void WorkQueue::start()
    {
        thr = std::unique_ptr<std::thread>(new std::thread([this]{ this->run(); }));
    }
    
    void WorkQueue::join()
    {
        if (thr) {
            queueCvar.notify_all(); // Wake up threads to allow them to process the shutdown event.
            thr->join();
            thr.reset();
        }
    }
    
    void WorkQueue::enqueue(std::function<void (void)> workItem)
    {
        std::lock_guard<std::mutex> lock(queueLock);
        queue.push(workItem);
        queueCvar.notify_one();
    }
    
    void WorkQueue::run()
    {
        while(!shouldShutdown)
        {
            std::unique_lock<std::mutex> lock(queueLock);
            queueCvar.wait(lock);
            if (!shouldShutdown && !queue.empty()) {
                auto workItem = queue.front();
                workItem();
                queue.pop();
            }
        }
    }
    
} // namespace PinkTopaz
