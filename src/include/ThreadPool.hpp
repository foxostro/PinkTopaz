//
//  ThreadPool.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/28/17.
//
//

#include <functional>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>
#include <thread>

class ThreadPool
{
public:
    typedef std::function<void()> Job;
    
    ThreadPool();
    ~ThreadPool();
    
    void enqueue(Job &&job);
    
private:
    void worker();
    
    std::vector<std::thread> _threads;
    std::mutex _lockJobs;
    std::condition_variable _cv;
    std::queue<Job> _jobs;
    std::atomic<bool> _threadShouldExit;
};

extern ThreadPool g_threadPool;
