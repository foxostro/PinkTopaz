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
 : _mainThreadId(std::this_thread::get_id()),
   _executing(false)
{}

void CommandQueue::execute()
{
    _executing = true;
    
    std::lock_guard<std::mutex> lock(_queueLock);
    
    while (!_queue.empty()) {
        auto command = std::move(_queue.front());
        CHECK_GL_ERROR();
        command();
        CHECK_GL_ERROR();
        _queue.pop();
    }
    
    _executing = false;
}

void CommandQueue::enqueue(std::function<void()> &&task)
{
    // If the queue is currently executing and we're on the main thread then a
    // call to enqueue has been made from within another task. In this case,
    // run the task immediately.
    // In practice, these recursive enqueues are usually destructors being run
    // because some object in a shared_ptr is now being destroyed.
    if (_executing && std::this_thread::get_id() == _mainThreadId) {
        task();
    } else {
        std::lock_guard<std::mutex> lock(_queueLock);
        _queue.push(std::move(task));
    }
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
