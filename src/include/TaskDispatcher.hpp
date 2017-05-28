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
    static constexpr bool ForceSerialDispatch = true;
    
    typedef std::function<void()> Task;
    
    TaskDispatcher();
    ~TaskDispatcher();
    
    void async(Task &&task);
    
private:
    void worker();
    
    std::vector<std::thread> _threads;
    std::mutex _lockTasks;
    std::condition_variable _cv;
    std::queue<Task> _tasks;
    std::atomic<bool> _threadShouldExit;
};

#endif /* TaskDispatcher_hpp */
