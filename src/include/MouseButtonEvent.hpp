//
//  MouseButtonEvent.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/8/18.
//
//

#ifndef MouseButtonEvent_hpp
#define MouseButtonEvent_hpp

#include "SDL.h"
    
struct MouseButtonEvent
{
    MouseButtonEvent(int b, bool d) : button(b), down(d) {}
    MouseButtonEvent() : button(SDL_BUTTON_LEFT), down(false) {}
        
    int button;
    bool down;
};

#endif /* MouseButtonEvent_hpp */
