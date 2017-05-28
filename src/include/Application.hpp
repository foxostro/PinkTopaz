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
#include "TaskDispatcher.hpp"

// The central game loop, basically.
class Application
{
public:
    Application() = default;
    ~Application() = default;

    // Runs the game loop on the current thread and returns when the game has ended.
    void run();
        
private:
    void inner(const std::shared_ptr<GraphicsDevice> &graphicsDevice,
               const std::shared_ptr<TaskDispatcher> &dispatcher);
    
    SDL_Window *_window;
};

#endif /* Application_hpp */
