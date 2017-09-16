//
//  Profiler.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/28/17.
//
//

#include "Profiler.hpp"
#include <dispatch/dispatch.h>
#include <sys/kdebug_signpost.h>

class ProfilerMacOS : public Profiler
{
public:
    void start(Label label) override
    {
        kdebug_signpost_start((uint32_t)label, 0, 0, 0, 0);
    }
    
    void end(Label label) override
    {
        kdebug_signpost_end((uint32_t)label, 0, 0, 0, 0);
    }
    
    void signPost(Label label) override
    {
        kdebug_signpost((uint32_t)label, 0, 0, 0, 0);
    }
};

std::shared_ptr<Profiler> getProfiler()
{
    static std::shared_ptr<Profiler> instance;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        instance = std::make_shared<ProfilerMacOS>();
    });
    return instance;
}
