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
#include "SDL.h"
#include <algorithm> // for remove_if

CommandQueue::CommandQueue()
 : _mainThreadId(std::this_thread::get_id())
{}

void CommandQueue::execute()
{
    if (std::this_thread::get_id() != _mainThreadId) {
        throw Exception("This command queue can only be executed on the " \
                        "thread on which it was constructed. This is " \
                        "expected to be the OpenGL thread.");
    }
    
    // Under lock, move `_queue' to `queue'. This will let us execute commands
    // on the main thread here while we continue to enqueue additional commands
    // on background threads. We cannot do something like execute those commands
    // now because then commands would interleave inappropriately.
    Queue queue;
    {
        std::lock_guard<std::mutex> lock(_queueLock);
        queue = std::move(_queue);
    }
    
    for (Task &task : queue) {
        //SDL_LogInfo(SDL_LOG_CATEGORY_RENDER, "Executing command \"%s\" with id=%u.", task.label.c_str(), task.id);
        task.fn();
    }
}

void CommandQueue::cancel(unsigned id)
{
    auto pred = [id](const Task &task){
        return task.id == id;
    };
    std::lock_guard<std::mutex> lock(_queueLock);
    //SDL_LogInfo(SDL_LOG_CATEGORY_RENDER, "Cancelling command with id=%u.", id);
    _queue.erase(std::remove_if(_queue.begin(), _queue.end(), pred), _queue.end());
}

void CommandQueue::enqueue(unsigned id, const std::string &label, std::function<void()> &&fn)
{
    std::lock_guard<std::mutex> lock(_queueLock);
    Task task = { id, label, std::move(fn) };
    //SDL_LogInfo(SDL_LOG_CATEGORY_RENDER, "Enqueueing command \"%s\" with id=%u.", task.label.c_str(), task.id);
    _queue.emplace_back(std::move(task));
}

void CommandQueue::enqueue(CommandQueue &other)
{
    std::unique_lock<std::mutex> lock1(_queueLock, std::defer_lock);
    std::unique_lock<std::mutex> lock2(other._queueLock, std::defer_lock);
    std::lock(lock1, lock2);
    _queue.insert(_queue.end(),
                  std::make_move_iterator(other._queue.begin()),
                  std::make_move_iterator(other._queue.end()));
    other._queue.clear();
}
