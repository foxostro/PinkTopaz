//
//  CommandQueue.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 2/24/17.
//
//

#ifndef CommandQueue_hpp
#define CommandQueue_hpp

#include <queue>
#include <mutex>
#include <thread>
    
class CommandQueue
{
public:
    CommandQueue();
    ~CommandQueue() = default;

    // Immediately execute all commands in the command queue.
    void execute();
    
    // Add a queue to the command queue for later execution.
    void enqueue(std::function<void()> &&task);
    
    // Add a queue to the command queue for later execution.
    void enqueue(CommandQueue &otherQueue);
    
private:
    std::thread::id _mainThreadId;
    std::mutex _queueLock;
    std::queue<std::function<void()>> _queue;
};

#endif /* CommandQueue_hpp */
