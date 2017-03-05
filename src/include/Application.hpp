//
//  Application.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 6/25/16.
//
//

#ifndef Application_hpp
#define Application_hpp

#include "SDL.h"
#include <memory>

#include "Renderer/GraphicsDevice.hpp"
#include "RenderableStaticMesh.hpp"

namespace PinkTopaz {
    
    // The central game loop, basically.
    class Application
    {
    public:
        Application() = default;
        ~Application() = default;

        // Runs the game loop on the current thread and returns when the game has ended.
        void run();
        
    private:
        RenderableStaticMesh createTerrainMesh(const std::shared_ptr<Renderer::GraphicsDevice> &g);
        void inner(const std::shared_ptr<Renderer::GraphicsDevice> &g);
        
        SDL_Window *_window;
    };

} // namespace PinkTopaz

#endif /* Application_hpp */
