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
    
public:
    Task() = delete;
    
    template<typename FunctionObjectType>
    Task(FunctionObjectType &&fn)
     : _cancelled(false),
       _task([this, f=std::move(fn)]() mutable {
           if (_cancelled) {
               throw BrokenPromiseException();
           }
           return f();
       })
    {}
    
    void cancel() override
    {
        _cancelled = true;
    }
    
    void execute() override
    {
        _task();
    }
    
    boost::future<ResultType> getFuture()
    {
        return _task.get_future();
    }
};

class TaskDispatcher;

template<typename ResultType>
class Future
{
private:
    boost::future<ResultType> _future;
    std::shared_ptr<TaskDispatcher> _dispatcher;
    std::shared_ptr<Task<ResultType>> _task;
    
public:
    ~Future() = default;
    Future() = delete;
    Future(const Future &future) = delete;
    Future(Future &&future) = default;
    
    Future(boost::future<ResultType> &&future,
           const std::shared_ptr<TaskDispatcher> &dispatcher,
           const std::shared_ptr<Task<ResultType>> &task)
     : _future(std::move(future)),
       _dispatcher(dispatcher),
       _task(task)
    {}
    
    bool isValid()
    {
        return _future.is_valid();
    }
    
    bool isReady()
    {
        return _future.is_ready();
    }
    
    void wait()
    {
        _future.wait();
    }
    
    ResultType get()
    {
        return _future.get();
    }
    
    void cancel()
    {
        _task->cancel();
    }
    
    // After this call, the Future is left in a moved-from state.
    template<typename F>
    auto then(F &&fn)
    {
        _task.reset();
        auto future = std::move(_future);
        auto dispatcher = std::move(_dispatcher);
        return dispatcher->async([fn=std::move(fn), future=std::move(future)]() mutable {
            return fn(future.get());
        });
    }
};

class TaskDispatcher : public std::enable_shared_from_this<TaskDispatcher>
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
    // function return values are accessible through a vector of Future objects
    // returned by the call to map().
    template<typename RangeType, typename FunctionObjectType>
    auto map(RangeType range, FunctionObjectType &&functionObject)
    {
        using IterationType = decltype(*range.begin());
        using ResultType = typename std::result_of<FunctionObjectType(IterationType)>::type;
        
        std::vector<Future<ResultType>> futures;
        
        for (const auto obj : range) {
            futures.emplace_back(async([functionObject, obj]{
                return functionObject(obj);
            }));
        }
        
        return futures;
    }
    
    // Issues `count' copies of the task and returns a vector of Future objects
    // through which the function return value will become accessible later.
    // The function can have any return type, but must accept no parameters.
    template<typename FunctionObjectType>
    auto map(size_t count, FunctionObjectType &&functionObject)
    {
        using ResultType = typename std::result_of<FunctionObjectType()>::type;
        std::vector<Future<ResultType>> tasks;
        tasks.reserve(count);
        for (size_t i = 0; i < count; ++i) {
            tasks.emplace_back(async(std::move(functionObject)));
        }
        return tasks;
    }
    
    // Schedule a task to be run asynchronously on a thread in the pool.
    // Returns a Future object from which the function return value can be
    // accessed later.
    // The function object for the task may have any return type, but must
    // accept no parameters.
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
        
        return Future<ResultType>(std::move(taskPtr->getFuture()),
                                  shared_from_this(),
                                  taskPtr);
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

// Wait for all futures in the range to complete.
template<typename FutureCollectionType>
static inline void waitForAll(FutureCollectionType &futures)
{
    for (auto &future : futures) {
        future.wait();
    }
}

#endif /* TaskDispatcher_hpp */
