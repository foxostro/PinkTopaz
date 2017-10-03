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

#define BOOST_THREAD_PROVIDES_FUTURE
#define BOOST_THREAD_PROVIDES_FUTURE_CONTINUATION
#include <boost/thread.hpp>
#include <boost/thread/future.hpp>

class AbstractTask
{
public:
    virtual ~AbstractTask() = default;
    virtual void cancel() = 0;
    virtual void execute() = 0;
};

template<typename ResultType>
class Task : public AbstractTask
{
    boost::packaged_task<ResultType> _task;
    
public:
    template<typename F>
    Task(F &&f) : _task(f) {}
    
    void cancel() override
    {
        // We can't really cancel a task. Instead, execute it now so we can get
        // the result. This happens when the task queue is shutdown and flushed.
        _task();
    }
    
    void execute() override
    {
        _task();
    }
    
    auto getFuture()
    {
        return _task.get_future();
    }
};

template<>
class Task<void> : public AbstractTask
{
    boost::packaged_task<void> _task;
    
public:
    template<typename F>
    Task(F &&f) : _task(f) {}
    
    void cancel() override {}
    
    void execute() override
    {
        _task();
    }
    
    auto getFuture()
    {
        return _task.get_future();
    }
};

class TaskDispatcher
{
public:
    TaskDispatcher() = delete;
    TaskDispatcher(unsigned numThreads);
    ~TaskDispatcher();
    
    // Returns true if the dispatcher is shutting down or has already shutdown.
    // Tasks can use this to check whether they should cancel themselves.
    inline bool isShutdown() const {
        return _threadShouldExit;
    }
    
    // Finish all scheduled tasks and exit all threads.
    void shutdown();
    
    // For each element of the range, calls the specified function asynchronously,
    // passing the element a parameter. The corresponding return function values are
    // returned in a vector of futures.
    template<typename RangeType, typename FunctionObjectType>
    auto map(RangeType range, FunctionObjectType &&functionObject)
    {
        using IterationType = decltype(*range.begin());
        using ResultType = typename std::result_of<FunctionObjectType(IterationType)>::type;
        
        std::vector<boost::future<ResultType>> futures;
        
        for (const auto &obj : range) {
            futures.emplace_back(async([functionObject, obj]{
                return functionObject(obj);
            }));
        }
        
        return futures;
    }
    
    // Schedule a function to be run asynchronously on a thread in the pool.
    // Returns a future to retrieve the function's return value.
    // The function can have any return type, but must accept no parameters.
    template<typename FunctionObjectType>
    auto async(FunctionObjectType &&functionObject)
    {
        using ResultType = typename std::result_of<FunctionObjectType()>::type;
        Task<ResultType> task(std::move(functionObject));
        auto taskPtr = std::make_shared<Task<ResultType>>(std::move(task));
        auto future = taskPtr->getFuture();
        
        {
            std::unique_lock<std::mutex> lock(_lockTaskPosted);
            _tasks.push(std::dynamic_pointer_cast<AbstractTask>(taskPtr));
        }
        _cvarTaskPosted.notify_one();
        
        return std::move(future);
    }
    
private:
    void worker();
    
    std::vector<std::thread> _threads;
    std::mutex _lockTaskPosted;
    std::condition_variable _cvarTaskPosted;
    std::mutex _lockTaskCompleted;
    std::queue<std::shared_ptr<AbstractTask>> _tasks;
    std::atomic<bool> _threadShouldExit;
};

#endif /* TaskDispatcher_hpp */
