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

class TaskDispatcher
{
public:
    // AFOX_TODO: Remove `ForceSerialDispatch'.
    static constexpr bool ForceSerialDispatch = false;
    
    typedef std::function<void()> Task;
    
    TaskDispatcher();
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

#endif /* TaskDispatcher_hpp */
