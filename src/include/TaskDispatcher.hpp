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

class TaskDispatcher
{
public:
    using Task = std::function<void()>;
    
    TaskDispatcher();
    TaskDispatcher(unsigned numThreads);
    ~TaskDispatcher();
    
    // Finish all scheduled tasks and exit all threads.
    void shutdown();
    
    // Schedule a function to be run asynchronously on a thread in the pool.
    // Returns a future to retrieve the function's return value.
    // The function can have any return type, but must accept no parameters.
    template<typename FunctionObjectType>
    auto async(FunctionObjectType &&functionObject)
    {
        // Create a packaged_task. This makes it easier to setup the promise and
        // future and connect it to the result of the function object.
        //
        // We choose boost::packaged_task over std::packaged_task because we
        // want to use boost::future instead of std::future. The reason to
        // choose boost::future is that it permits `.then' continuation chains.
        //
        // std::function requires a copyable lambda to be passed to it. This
        // prevents us from marking the lambda as mutable which then prevents us
        // from capturing the packaged_task in the lambda with a move-capture.
        // Since the packaged_task cannot be moved into the lambda, and cannot
        // be captured by-copy or by-reference, we need some other way to keep
        // it alive until the lambda executes: wrap it in a shared_ptr.
        using ResultType = typename boost::result_of<FunctionObjectType()>::type;
        auto packagedTaskPtr = std::make_shared<boost::packaged_task<ResultType>>(std::move(functionObject));
        
        auto future = packagedTaskPtr->get_future();
        
        // The call to execute the packaged task is wrapped in a std::function,
        // which is then enqueued. This deals with pulling the packaged_task
        // object out of the shared_ptr.
        enqueue([packagedTaskPtr]{
            (*packagedTaskPtr)();
        });
        
        return std::move(future);
    }
    
    // Schedule a task to be run asynchronously on another thread.
    void enqueue(Task &&task);
    
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
