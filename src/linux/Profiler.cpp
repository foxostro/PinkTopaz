//
//  Profiler.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/28/17.
//
//

#include "Profiler.hpp"
#include <mutex>

class ProfilerLinux : public Profiler
{
public:
    void start(Label label) override
    {
        // stub
    }
    
    void end(Label label) override
    {
        // stub
    }
    
    void signPost(Label label) override
    {
        // stub
    }
};

std::shared_ptr<Profiler> getProfiler()
{
    static std::shared_ptr<Profiler> profiler;
    static std::once_flag flag;
    std::call_once(flag, [&]{
        profiler = std::make_shared<ProfilerLinux>();
    });
    return profiler;
}
