//
//  TaskDispatcher.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/28/17.
//
//

#ifndef TaskDispatcher_hpp
#define TaskDispatcher_hpp

#include <functional>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>
#include <thread>
#include <cassert>

class TaskDispatcher
{
public:
    using Task = std::function<void()>;
    
    TaskDispatcher();
    TaskDispatcher(unsigned numThreads);
    ~TaskDispatcher();
    
    // Finish all scheduled tasks and exit all threads.
    void shutdown();
    
    // Schedule a task to be run asynchronously on another thread.
    void async(Task &&task);
    
private:
    void worker();
    
    std::vector<std::thread> _threads;
    std::mutex _lockTaskPosted;
    std::condition_variable _cvarTaskPosted;
    std::mutex _lockTaskCompleted;
    std::queue<Task> _tasks;
    bool _threadShouldExit;
};

class TaskGroup
{
public:
    TaskGroup(size_t count) : _count(count) {}
    
    void completeOne()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        assert(_count > 0);
        --_count;
        _cvar.notify_one();
    }
    
    void wait()
    {
        std::unique_lock<std::mutex> lock(_mutex);
        _cvar.wait(lock, [&]{
            return _count == 0;
        });
    }
    
private:
    std::mutex _mutex;
    std::condition_variable _cvar;
    size_t _count;
};

#endif /* TaskDispatcher_hpp */
