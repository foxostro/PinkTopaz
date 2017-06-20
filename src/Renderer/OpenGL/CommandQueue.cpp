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

CommandQueue::CommandQueue()
 : _mainThreadId(std::this_thread::get_id())
{}

void CommandQueue::execute()
{
    if (std::this_thread::get_id() != _mainThreadId) {
        throw Exception("This command queue can only be executed on the " \
                        "thread on which it was constructed. This is expected" \
                        " to be the OpenGL thread.");
    }
    
    // Under lock, move `_queue' to `queue'. This will let us execute commands
    // on the main thread here while we continue to enqueue additional commands
    // on background threads. We cannot do something like execute those commands
    // now because then commands would interleave inappropriately.
    std::queue<std::function<void()>> queue;
    {
        std::lock_guard<std::mutex> lock(_queueLock);
        queue = std::move(_queue);
    }
    
    while (!queue.empty()) {
        auto command = std::move(queue.front());
        command();
        queue.pop();
    }
}

void CommandQueue::enqueue(std::function<void()> &&task)
{
    std::lock_guard<std::mutex> lock(_queueLock);
    _queue.push(std::move(task));
}

void CommandQueue::enqueue(CommandQueue &otherQueue)
{
    std::unique_lock<std::mutex> lock1(_queueLock, std::defer_lock);
    std::unique_lock<std::mutex> lock2(otherQueue._queueLock, std::defer_lock);
    
    std::lock(lock1, lock2);
    
    while (!otherQueue._queue.empty()) {
        auto command = std::move(otherQueue._queue.front());
        _queue.push(command);
        otherQueue._queue.pop();
    }
}
