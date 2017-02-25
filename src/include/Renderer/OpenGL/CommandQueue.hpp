//
//  CommandQueue.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 2/24/17.
//
//

#ifndef CommandQueue_hpp
#define CommandQueue_hpp

#include <memory>
#include <queue>
#include <mutex>

namespace PinkTopaz::Renderer::OpenGL {
    
    class CommandQueue
    {
    public:
        // Immediately execute all commands in the command queue.
        void execute();
        
        // Add a queue to the command queue for later execution.
        void enqueue(std::function<void()> &&task);
        
        // Add a queue to the command queue for later execution.
        void enqueue(CommandQueue &otherQueue);
        
    private:
        std::mutex queueLock;
        std::queue<std::function<void()>> queue;
    };
    
}; // namespace PinkTopaz::Renderer::OpenGL

#endif /* CommandQueue_hpp */
