//
//  WorkQueue.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 6/25/16.
//
//

#ifndef WorkQueue_hpp
#define WorkQueue_hpp

#include <thread>
#include <atomic>
#include <queue>

namespace PinkTopaz {
    
    // Multithreaded work queue. Accepts commands, puts them into a queue, and processes them asynchonously on a
    // dedicated thread.
    class WorkQueue
    {
    private:
        std::unique_ptr<std::thread> thr;
        std::atomic_bool shouldShutdown;
        std::mutex queueLock;
        std::condition_variable queueCvar;
        std::queue<std::function<void (void)> > queue;
        
        // The thread runs this method.
        void run();
        
    public:
        // Constructor. Does not start the thread. The work queue is not ready until start() is called.
        WorkQueue();
        
        // Destructor. Ensures clean shutdown of the queue.
        virtual ~WorkQueue();
        
        // Signals that the thread should shutdown and terminate.
        void setShouldShutdown();
        
        // Starts the thread. The work queue may begin processing work items after this call.
        void start();
        
        // Blocks until the thread terminates and joins.
        void join();
        
        // Adds a work item to the queue to be processed asynchronously.
        void enqueue(std::function<void (void)> workItem);
    };
    
} // namespace PinkTopaz

#endif /* WorkQueue_hpp */
