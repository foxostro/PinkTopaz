//
//  KeypressEvent.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 3/5/17.
//
//

#ifndef KeypressEvent_hpp
#define KeypressEvent_hpp

#include "SDL.h"
    
struct KeypressEvent
{
    KeypressEvent(SDL_Keycode k, bool d, unsigned t)
     : key(k), down(d), timestamp(t)
    {}
    
    KeypressEvent() : key(SDLK_UNKNOWN), down(false), timestamp(0) {}
        
    SDL_Keycode key;
    bool down;
    unsigned timestamp;
};

#endif /* KeypressEvent_hpp */
