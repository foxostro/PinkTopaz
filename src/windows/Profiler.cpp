//
//  Profiler.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/28/17.
//
//

#include "Profiler.hpp"

class ProfilerMacOS : public Profiler
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
    return std::shared_ptr<Profiler>();
}
