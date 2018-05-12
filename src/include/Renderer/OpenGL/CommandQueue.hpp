//
//  CommandQueue.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 2/24/17.
//
//

#ifndef CommandQueue_hpp
#define CommandQueue_hpp

#include <vector>
#include <mutex>
#include <thread>
#include <string>
#include <functional>
#include <spdlog/spdlog.h>

class CommandQueue
{
public:
    CommandQueue(std::shared_ptr<spdlog::logger> log);
    ~CommandQueue() = default;
    
    // Cancel all tasks with the specified ID. Cancelled tasks are simply never
    // executed. Once a task begins executing, it cannot be cancelled.
    void cancel(unsigned id);

    // Immediately execute all commands in the command queue.
    void execute();
    
    // Add a task to the command queue for later execution.
    // The specified ID allows the task to be cancelled later via cancel().
    void enqueue(unsigned id, const std::string &label, std::function<void()> &&task);
    
    // Add a queue to the command queue for later execution.
    void enqueue(CommandQueue &otherQueue);
    
private:
    std::thread::id _mainThreadId;
    std::mutex _queueLock;
    
    // The queue stores information about the task to execute.
	struct Task
	{
		unsigned id;
		std::string label;
		std::function<void()> fn;
	};
    typedef std::vector<Task> Queue;
    Queue _queue;
    
    std::shared_ptr<spdlog::logger> _log;
};

#endif /* CommandQueue_hpp */
