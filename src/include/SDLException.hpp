//
//  SDLException.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/12/18.
//
//

#ifndef SDLException_hpp
#define SDLException_hpp

#include "Exception.hpp"
#include "SDL.h"
    
// Exception thrown when an error occurs in the SDL library.
class SDLException : public Exception
{
public:
    template<typename... Args>
    SDLException(Args&&... args)
    : Exception("{}: {}",
                fmt::format(std::forward<Args>(args)...),
                SDL_GetError())
    {}
};

#endif /* SDLException_hpp */
