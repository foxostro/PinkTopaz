//
//  FrameTimer.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 7/8/16.
//
//

#include "FrameTimer.hpp"
#include "SDL.h"
#include <sstream>
    
FrameTimer::FrameTimer(StringRenderer &stringRenderer)
    : _stringRenderer(stringRenderer),
    _framesBetweenReport(60),
    _timeAccum(0),
    _countDown(60),
    _firstReportingPeriod(true),
    _ticksBeginMs(0)
{}
    
void FrameTimer::beginFrame()
{
    _ticksBeginMs = SDL_GetTicks();
}
    
void FrameTimer::endFrame()
{
    unsigned _ticksEndMs = SDL_GetTicks();
        
    // Report the average time between frames.
    unsigned ticksElapsedMs = _ticksEndMs - _ticksBeginMs;
    _timeAccum += ticksElapsedMs;
        
    if (_countDown == 0) {
        float frameTime = (float)_timeAccum / _framesBetweenReport;
            
        std::stringstream ss;
        ss.precision(2);
        ss << "Frame Time: " << std::fixed << frameTime << " ms";
        std::string s(ss.str());
            
        if (_firstReportingPeriod) {
            _firstReportingPeriod = false;
            const glm::vec4 color(0.2f, 0.2f, 0.2f, 1.0f);
            const glm::vec2 position(30.0f, 1140.0f);
            _frameTimeLabel = _stringRenderer.add(s, position, color);
        } else {
            _stringRenderer.replaceContents(_frameTimeLabel, s);
        }
            
        _countDown = _framesBetweenReport;
        _timeAccum = 0;
    } else {
        --_countDown;
    }
}
