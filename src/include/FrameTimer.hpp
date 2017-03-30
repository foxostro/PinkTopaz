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
    
// Measures the time each frame takes and displays that on the screen.
class FrameTimer
{
public:
    FrameTimer(StringRenderer &stringRenderer);
        
    // Call after swapBuffers() to calculate the time and update the UI.
    void tick();
        
private:
    StringRenderer &_stringRenderer;
    const unsigned _framesBetweenReport;
        
    StringRenderer::StringHandle _frameTimeLabel;
    unsigned _timeAccum;
    unsigned _countDown;
    bool _firstReportingPeriod;
    unsigned _ticksBeginMs;
};

#endif /* RenderSystem_hpp */
