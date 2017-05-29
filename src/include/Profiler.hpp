//
//  Profiler.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/28/17.
//
//

#ifndef Profiler_hpp
#define Profiler_hpp

#include <string>

class Profiler
{
public:
    Profiler(const std::string &label);
    ~Profiler();
    
    void signpost(const std::string &intermediateLabel) const;
    
private:
    unsigned _beginMs;
    std::string _label;
};

#if 1
#define PROFILER(label)
#define PROFILER_SIGNPOST(label)
#else
#define PROFILER(label) Profiler profiler(label)
#define PROFILER_SIGNPOST(label) profiler.signpost(label)
#endif

#endif /* Profiler_hpp */
