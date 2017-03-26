//
//  WindowSizeChangedEvent.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 7/8/16.
//
//

#ifndef WindowSizeChangedEvent_hpp
#define WindowSizeChangedEvent_hpp

struct WindowSizeChangedEvent
{
    WindowSizeChangedEvent() : windowScaleFactor(0), width(0), height(0) {}
        
    float windowScaleFactor;
    int width, height;
};

#endif /* WindowSizeChangedEvent_hpp */
