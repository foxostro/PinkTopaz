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

namespace PinkTopaz {
    
    FrameTimer::FrameTimer(Renderer::GraphicsDevice &graphicsDevice,
                           Renderer::StringRenderer &stringRenderer)
     : //_graphicsDevice(graphicsDevice),
       _stringRenderer(stringRenderer),
       _framesBetweenReport(100),
       _frameTimeLabel(),
       _frameTimeFence(graphicsDevice.makeFence()),
       _timeAccum(0),
       _countDown(0),
       _firstReportingPeriod(true),
       _ticksBeginMs(0),
       _ticksEndMs(0),
       _renderPass()
    {
        _renderPass.blend = false;
        _renderPass.clear = false;
        _renderPass.depthTest = false;
    }
    
    void FrameTimer::beginFrame()
    {
        _ticksBeginMs = SDL_GetTicks();
    }
    
    void FrameTimer::endFrame()
    {
#if 0
        // Measure the time it takes for all GPU work to complete.
        // We do this by issuing a GPU fence in a new encoder and waiting for
        // it to complete.
        auto encoder = _graphicsDevice.encoder(_renderPass);
        
        encoder->updateFence(_frameTimeFence);
        
        // The completion handler for the fence below will have definitely
        // executed by the time swapBuffers() returns.
        encoder->waitForFence(_frameTimeFence, [=]{
            this->_ticksEndMs = SDL_GetTicks();
        });
        
        _graphicsDevice.submit(encoder);
#else
        _ticksEndMs = SDL_GetTicks();
#endif
    }
    
    void FrameTimer::afterFrame()
    {
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
    
} // namespace PinkTopaz
