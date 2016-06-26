//
//  Actor.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 6/25/16.
//
//

#ifndef Actor_hpp
#define Actor_hpp

#include <thread>
#include <atomic>

namespace PinkTopaz {
    
    // Generic actor object. Accepts messages and commands which are queued and processed in a separate thread.
    // The message queue must be implemented in the child classes. The API for this is very application-specific
    // so we cannot define this API in the base class. Instead, the Actor base class exclusively handles the creation
    // and management of the actor thread.
    class Actor
    {
    private:
        std::unique_ptr<std::thread> thr;
        std::atomic_bool shouldShutdown;
        
        // The thread runs this method.
        void run();
        
    protected:
        // Called on the actor thread before entering the loop.
        virtual void preLoop() = 0;
        
        // A single iteration of the actor message handling loop.
        virtual void pump() = 0;
        
        // Called on the actor thread after exiting the loop.
        virtual void postLoop() = 0;
        
    public:
        // Constructor. Does not start the thread. The actor is not ready until start() is called.
        Actor();
        
        // Destructor. Call finish() to ensure clean shutdown of the actor first.
        virtual ~Actor();
        
        // Signals that the thread should shutdown and terminate.
        void setShouldShutdown();
        
        // Starts the thread. The actor may begin processing messages after this call.
        void start();
        
        // Blocks until the thread terminates and joins.
        void join();
    };
    
} // namespace PinkTopaz

#endif /* Actor_hpp */
