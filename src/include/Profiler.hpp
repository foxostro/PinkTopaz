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

#if 0
#define PROFILER(label)
#else
#define PROFILER(label) Profiler profiler(label);
#endif

#endif /* Profiler_hpp */
