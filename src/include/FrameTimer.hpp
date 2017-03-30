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
#include "UniqueName.hpp"

#define FRAME_TIMER(frameTimer) auto UNIQUE_NAME(token) = (frameTimer).token()
    
// Measures the time each frame takes and displays that on the screen.
class FrameTimer
{
public:
    class Token
    {
    public:
        Token(FrameTimer &frameTimer) : _frameTimer(frameTimer)
        {
            _frameTimer.beginFrame();
        }
            
        ~Token()
        {
            _frameTimer.endFrame();
        }
            
    private:
        FrameTimer &_frameTimer;
    };
        
    FrameTimer(StringRenderer &stringRenderer);
        
    // Call at the beginning of the frame to start timing.
    void beginFrame();
        
    // Call after swapBuffers() to calculate the time and update the UI.
    void endFrame();
        
    // The token will begin and end the frame at the edges of it's scope.
    inline Token token() { return Token(*this); }
        
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
