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

namespace PinkTopaz {
    
    struct MouseMoveEvent
    {
        MouseMoveEvent(int dx, int dy) : deltaX(dx), deltaY(dy) {}
        MouseMoveEvent() : deltaX(0), deltaY(0) {}
        
        int deltaX, deltaY;
    };

} // namespace PinkTopaz

#endif /* MouseMoveEvent_hpp */
