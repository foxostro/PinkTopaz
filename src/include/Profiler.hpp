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
#include <functional>

#define PROFILER_ENABLED 0

class ThreadProfiler
{
public:
    class Scope
    {
    public:
        typedef std::function<void(const std::string &label, unsigned elapsedMs)> Handler;
        
        Scope(const std::string &label, const Handler &onDtor);
        ~Scope();
        
    private:
        const std::string _label;
        const Handler _onDtor;
        const unsigned _beginMs;
    };
    
    ThreadProfiler();
    ~ThreadProfiler() = default;
    
    Scope scope(const std::string &label);
    
private:
    size_t _level;
};

#if PROFILER_ENABLED
#define PROFILER(profiler, name, label) auto name = (profiler).scope(label)
#else
#define PROFILER(profiler, name, label)
#endif

#endif /* Profiler_hpp */
