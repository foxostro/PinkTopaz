//
//  TaskDispatcher.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/28/17.
//
//

#ifndef TaskDispatcher_hpp
#define TaskDispatcher_hpp

#include "Exception.hpp"

#include <functional>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>
#include <thread>
#include <cassert>
#include <atomic>

#define BOOST_THREAD_PROVIDES_FUTURE
#define BOOST_THREAD_PROVIDES_FUTURE_CONTINUATION
#include <boost/thread.hpp>
#include <boost/thread/future.hpp>

// Exception thrown when a Task promise is broken.
class BrokenPromiseException : public Exception {};

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
    std::atomic<bool> _cancelled;
    boost::packaged_task<ResultType> _task;
    boost::future<ResultType> _future;
    
public:
    Task() : _cancelled(false) {}
    
    template<typename F>
    Task(F &&f)
     : _cancelled(false),
       _task([this, f=std::move(f)]{
           if (_cancelled) {
               throw BrokenPromiseException();
           }
           return f();
       }),
       _future(_task.get_future())
    {}
    
    void cancel() override
    {
        _cancelled = true;
    }
    
    void execute() override
    {
        _task();
    }
    
    boost::future<ResultType>& getFuture()
    {
        return _future;
    }
};

class TaskDispatcher
{
public:
    TaskDispatcher() = delete;
    TaskDispatcher(const std::string &name,
                   unsigned numThreads);
    ~TaskDispatcher();
    
    // Returns true if the dispatcher is shutting down or has already shutdown.
    // Tasks can use this to check whether they should cancel themselves.
    inline bool isShutdown() const {
        return _threadShouldExit;
    }
    
    // Finish all scheduled tasks and exit all threads.
    void shutdown();
    
    // For each element of the range, calls the specified function
    // asynchronously, passing the element a parameter. The corresponding
    // function return values are accessible through a vector of Task objects
    // returned by the call to map().
    template<typename RangeType, typename FunctionObjectType>
    auto map(RangeType range, FunctionObjectType &&functionObject)
    {
        using IterationType = decltype(*range.begin());
        using ResultType = typename std::result_of<FunctionObjectType(IterationType)>::type;
        
        std::vector<std::shared_ptr<Task<ResultType>>> tasks;
        
        for (const auto obj : range) {
            tasks.emplace_back(async([functionObject, obj]{
                return functionObject(obj);
            }));
        }
        
        return tasks;
    }
    
    // Issues `count' copies of the task and returns a vector of Task objects.
    // The function can have any return type, but must accept no parameters.
    template<typename FunctionObjectType>
    auto map(size_t count, FunctionObjectType &&functionObject)
    {
        using ResultType = typename std::result_of<FunctionObjectType()>::type;
        std::vector<std::shared_ptr<Task<ResultType>>> tasks(count);
        for (size_t i = 0; i < count; ++i) {
            tasks.emplace_back(async(std::move(functionObject)));
        }
        return tasks;
    }
    
    // Schedule a function to be run asynchronously on a thread in the pool.
    // Returns a Task object from which a future can be obtained to retrieve the
    // function's return value.
    // The function can have any return type, but must accept no parameters.
    template<typename FunctionObjectType>
    auto async(FunctionObjectType &&functionObject)
    {
        using ResultType = typename std::result_of<FunctionObjectType()>::type;
        auto taskPtr = std::make_shared<Task<ResultType>>(std::move(functionObject));
        
        {
            std::unique_lock<std::mutex> lock(_lockTaskPosted);
            _tasks.push(std::dynamic_pointer_cast<AbstractTask>(taskPtr));
        }
        _cvarTaskPosted.notify_one();
        
        return taskPtr;
    }
    
    // Wait for all tasks in the range to complete.
    template<typename TaskCollectionType>
    void waitForAll(TaskCollectionType &tasks)
    {
        for (auto &task : tasks) {
            task->getFuture().wait();
        }
    }
    
private:
    void worker(const std::string &name);
    
    std::vector<std::thread> _threads;
    std::mutex _lockTaskPosted;
    std::condition_variable _cvarTaskPosted;
    std::mutex _lockTaskCompleted;
    std::queue<std::shared_ptr<AbstractTask>> _tasks;
    std::atomic<bool> _threadShouldExit;
};

#endif /* TaskDispatcher_hpp */
