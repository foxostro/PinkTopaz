//
//  Profiler.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/28/17.
//
//

#include "Profiler.hpp"
#include "SDL.h"

ThreadProfiler::Scope::Scope(const std::string &label,
                             const ThreadProfiler::Scope::Handler &onDtor)
 : _label(label),
   _onDtor(onDtor),
   _beginMs(SDL_GetTicks())
{}

ThreadProfiler::Scope::~Scope()
{
    const unsigned elapsedMs = SDL_GetTicks() - _beginMs;
    _onDtor(_label, elapsedMs);
}

ThreadProfiler::Scope ThreadProfiler::scope(const std::string &label)
{
    _level++;
    
#if PROFILER_ENABLED
    std::string indent(_level, '\t');
    SDL_Log("%s%s -- Begin", indent.c_str(), label.c_str());
#endif
    
    return Scope(label, [&](const std::string &label, unsigned elapsed){
#if PROFILER_ENABLED
        std::string indent(_level, '\t');
        SDL_Log("%s%s -- End (%d ms)", indent.c_str(), label.c_str(), elapsed);
#endif
        _level--;
    });
}

ThreadProfiler::ThreadProfiler()
 : _level(0)
{}
