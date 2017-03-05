//
//  FrameTimer.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 3/4/17.
//
//

#ifndef FrameTimer_hpp
#define FrameTimer_hpp

#include "Renderer/StringRenderer.hpp"

namespace PinkTopaz {
    
    // Measures the time each frame takes and displays that on the screen.
    class FrameTimer
    {
    public:
        FrameTimer(Renderer::StringRenderer &stringRenderer);
        
        // Call at the beginning of the frame to start timing.
        void beginFrame();
        
        // Call immediately before swapBuffers() to mark the end of the frame.
        void endFrame();
        
        // Call after swapBuffers() to calculate the time and update the UI.
        void afterFrame();
        
    private:
        Renderer::StringRenderer &_stringRenderer;
        const unsigned _framesBetweenReport;
        
        Renderer::StringRenderer::StringHandle _frameTimeLabel;
        unsigned _timeAccum;
        unsigned _countDown;
        bool _firstReportingPeriod;
        unsigned _ticksBeginMs, _ticksEndMs;
        Renderer::RenderPassDescriptor _renderPass;
    };
    
} // namespace PinkTopaz

#endif /* RenderSystem_hpp */
