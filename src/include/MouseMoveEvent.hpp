//
//  MouseMoveEvent.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 3/7/17.
//
//

#ifndef MouseMoveEvent_hpp
#define MouseMoveEvent_hpp

#include "SDL.h"
    
struct MouseMoveEvent
{
    MouseMoveEvent(int dx, int dy, unsigned t)
     : deltaX(dx), deltaY(dy)
    {}
    
    MouseMoveEvent() : deltaX(0), deltaY(0), timestamp(0) {}
        
    int deltaX, deltaY;
    unsigned timestamp;
};

#endif /* MouseMoveEvent_hpp */
