//
//  CommandQueue.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 2/24/17.
//
//

#include "Renderer/OpenGL/CommandQueue.hpp"
#include "Renderer/OpenGL/glUtilities.hpp"
#include "Exception.hpp"

namespace PinkTopaz::Renderer::OpenGL {
    
    void CommandQueue::execute()
    {
        std::lock_guard<std::mutex> lock(queueLock);
        
        CHECK_GL_ERROR();
        
        while (!queue.empty())
        {
            auto command = std::move(queue.front());
            command();
            CHECK_GL_ERROR();
            queue.pop();
        }
    }
    
    void CommandQueue::enqueue(std::function<void()> &&task)
    {
        std::lock_guard<std::mutex> lock(queueLock);
        queue.push(std::move(task));
    }
    
    void CommandQueue::enqueue(CommandQueue &otherQueue)
    {
        std::unique_lock<std::mutex> lock1(queueLock, std::defer_lock);
        std::unique_lock<std::mutex> lock2(otherQueue.queueLock, std::defer_lock);
        
        std::lock(lock1, lock2);
        
        while (!otherQueue.queue.empty())
        {
            auto command = std::move(otherQueue.queue.front());
            queue.push(command);
            otherQueue.queue.pop();
        }
    }
    
}; // namespace PinkTopaz::Renderer::OpenGL
