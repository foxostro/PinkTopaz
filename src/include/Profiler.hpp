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
#include "UniqueName.hpp"

// Change this macro to 1 to enable profiling with the PROFILER macro. Set to 0
// disable this profiling at compile time. The profiling is fairly low overhead
// but can still take a millisecond or two per frame.
#define PROFILER_ENABLED 0

// The Profiler class is useful for logging events during the execution of the
// game. This can help provide signposts and context to profiling tool output.
// For example, this is very useful when used along with the Instruments "Time
// Profiler" and "Points of Interest" tools.
class Profiler
{
public:
    enum Label
    {
        Frame = 0,
        Render,
        InitWorld,
        Quit,
    };
    
    class Scope
    {
    public:
        Scope(Profiler &profiler, Label label)
         : _profiler(profiler), _label(label)
        {
            _profiler.start(_label);
        }
        
        ~Scope()
        {
            _profiler.end(_label);
        }
        
    private:
        Profiler &_profiler;
        const Label _label;
    };
    
    Profiler() = default;
    virtual ~Profiler() = default;
    
    // Return a scope object. This will log the beginning of an event,
    // identified by `label', in the constructor and will log the end of that
    // same event in the destructor.
    // Use this with the PROFILER macro to easily log the beginning and end of
    // a scope block.
    inline Scope scope(Label label)
    {
        return Scope(*this, label);
    }
    
    // Log the start of an event, identified by `label'.
    virtual void start(Label label) = 0;
    
    // Log the end of an event, identified by `label'.
    virtual void end(Label label) = 0;
    
    // Log an instantaneous event, identified by `label'.
    virtual void signPost(Label label) = 0;
};

#if PROFILER_ENABLED

// Returns the global profiler. There is one profiler for the entire app.
std::shared_ptr<Profiler> getProfiler();

#define PROFILER_SIGNPOST(label) do { getProfiler()->signPost(Profiler :: label); } while(false)

// Log the beginning and end of an event that spans the scope where this macro
// is defined. See also the Profiler::scope() method.
#define PROFILER(label) auto PP_CAT(Profiler, __COUNTER__) = getProfiler()->scope(Profiler :: label)

#else
#define PROFILER_SIGNPOST(label)
#define PROFILER(label)
#endif

#endif /* Profiler_hpp */
