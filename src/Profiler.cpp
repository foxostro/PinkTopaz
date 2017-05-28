//
//  Profiler.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/28/17.
//
//

#include "Profiler.hpp"
#include "SDL.h"

Profiler::Profiler(const std::string &label)
 : _beginMs(SDL_GetTicks()), _label(label)
{}

Profiler::~Profiler()
{
    const unsigned endMs = SDL_GetTicks();
    const unsigned elapsedMs = endMs - _beginMs;
    SDL_Log("[Profiler] %s -- %u ms", _label.c_str(), elapsedMs);
}

void Profiler::signpost(const std::string &intermediateLabel) const
{
    const unsigned endMs = SDL_GetTicks();
    const unsigned elapsedMs = endMs - _beginMs;
    SDL_Log("[Profiler] %s -- %s -- %u ms",
            _label.c_str(), intermediateLabel.c_str(), elapsedMs);
}
