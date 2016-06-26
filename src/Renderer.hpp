//
//  Renderer.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 6/25/16.
//
//

#ifndef Renderer_hpp
#define Renderer_hpp

#include <SDL2/SDL.h>
#include "Actor.hpp"

namespace PinkTopaz {
    
    // Actor which handles the task of Renderering.
    // This one does OpenGL renderering. This actor and associated objects are actually the only parts of the engine
    // that are permitted to access the OpenGL API.
    class Renderer : public Actor
    {
    private:
        SDL_Window *window;

    protected:
        // Called on the actor thread before entering the loop.
        virtual void preLoop();
        
        // A single iteration of the actor message handling loop.
        virtual void pump();
        
        // Called on the actor thread after exiting the loop.
        virtual void postLoop();
        
    public:
        Renderer(SDL_Window *window);
    };

} // namespace PinkTopaz

#endif /* Renderer_hpp */
