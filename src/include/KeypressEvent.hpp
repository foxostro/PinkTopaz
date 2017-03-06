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

namespace PinkTopaz {
    
    struct KeypressEvent
    {
        KeypressEvent(SDL_Keycode k, bool d) : key(k), down(d) {}
        KeypressEvent() : key(SDLK_UNKNOWN), down(false) {}
        
        SDL_Keycode key;
        bool down;
    };

} // namespace PinkTopaz

#endif /* KeypressEvent_hpp */
