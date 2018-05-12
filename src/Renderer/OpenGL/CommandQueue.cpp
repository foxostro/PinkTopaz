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
#include <algorithm> // for remove_if

CommandQueue::CommandQueue(std::shared_ptr<spdlog::logger> log)
 : _mainThreadId(std::this_thread::get_id()),
   _log(log)
{}

void CommandQueue::execute()
{
    if (std::this_thread::get_id() != _mainThreadId) {
        throw CommandQueueInappropriateThreadOpenGLException();
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
        _log->trace("Executing command \"{}\" with id={}.", task.label, task.id);
        task.fn();
    }
}

void CommandQueue::cancel(unsigned id)
{
    auto pred = [id](const Task &task){
        return task.id == id;
    };
    std::lock_guard<std::mutex> lock(_queueLock);
    _log->trace("Cancelling command with id={}.", id);
    _queue.erase(std::remove_if(_queue.begin(), _queue.end(), pred), _queue.end());
}

void CommandQueue::enqueue(unsigned id, const std::string &label, std::function<void()> &&fn)
{
    std::lock_guard<std::mutex> lock(_queueLock);
    Task task = { id, label, std::move(fn) };
    _log->trace("Enqueueing command \"{}\" with id={}.", task.label, task.id);
    _queue.emplace_back(std::move(task));
}

void CommandQueue::enqueue(CommandQueue &other)
{
    std::scoped_lock lock(_queueLock, other._queueLock);
    _queue.insert(_queue.end(),
                  std::make_move_iterator(other._queue.begin()),
                  std::make_move_iterator(other._queue.end()));
    other._queue.clear();
}
