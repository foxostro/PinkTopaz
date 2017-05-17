//
//  Application.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 6/25/16.
//
//

#include "pinktopaz_config.h"
#include "FileUtilities.hpp"
#include "RetinaSupport.h"
#include "World.hpp"
#include "WindowSizeChangedEvent.hpp"
#include "KeypressEvent.hpp"
#include "MouseMoveEvent.hpp"
#include "Exception.hpp"

#include "SDL.h"
#include <map>

#include "Application.hpp"

void Application::inner(const std::shared_ptr<GraphicsDevice> &graphicsDevice)
{
    std::map<SDL_Keycode, bool> prevKeyStates, keyStates;
    
    World gameWorld(graphicsDevice);
    
    // Send an event containing the initial window size and scale factor.
    // This will allow the render system to setup projection matrices and such.
    {
        WindowSizeChangedEvent event;
        SDL_GetWindowSize(_window, &event.width, &event.height);
        event.windowScaleFactor = windowScaleFactor(_window);
        gameWorld.events.emit(event);
    }
        
    bool quit = false;
        
    unsigned ticksBeginMs = SDL_GetTicks();
        
    while(!quit)
    {
        SDL_Event e;
            
        if (SDL_PollEvent(&e)) {
            switch(e.type)
            {
                case SDL_QUIT:
                    SDL_Log("Received SDL_QUIT.");
                    quit = true;
                    break;
                        
                case SDL_WINDOWEVENT:
                    switch(e.window.event)
                    {
                        case SDL_WINDOWEVENT_RESIZED:
                            // fall through
                                
                        case SDL_WINDOWEVENT_SIZE_CHANGED:
                        {
                            WindowSizeChangedEvent event;
                            event.width = e.window.data1;
                            event.height = e.window.data2;
                            event.windowScaleFactor = windowScaleFactor(_window);
                            gameWorld.events.emit(event);
                        } break;
                    }
                    break;
                        
                case SDL_KEYDOWN:
                    keyStates[e.key.keysym.sym] = true;
                    break;
                        
                case SDL_KEYUP:
                    keyStates[e.key.keysym.sym] = false;
                    break;
                        
                case SDL_MOUSEMOTION:
                    gameWorld.events.emit(MouseMoveEvent(e.motion.xrel,
                                                            e.motion.yrel));
                    break;
            }
        }
            
        for(std::pair<SDL_Keycode, bool> pair : keyStates)
        {
            if (pair.second != prevKeyStates[pair.first]) {
                gameWorld.events.emit(KeypressEvent(pair.first, pair.second));
            }
        }
            
        unsigned ticksEndMs = SDL_GetTicks();
        unsigned ticksElapsedMs = ticksEndMs - ticksBeginMs;
        ticksBeginMs = ticksEndMs;
        entityx::TimeDelta dt = ticksElapsedMs;

        gameWorld.update(dt);
            
        prevKeyStates = keyStates;
    }
}
    
void Application::run()
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SDL_Init failed: %s\n", SDL_GetError());
    }

    // Set the current working directory to the SDL data path.
    // On Mac OS, this is the bundle "Resources" directory.
    {
        char *pathStr = SDL_GetBasePath();
        boost::filesystem::path cwd(pathStr);
        boost::filesystem::current_path(cwd);
        SDL_free(pathStr);
    }

    SDL_LogSetPriority(SDL_LOG_CATEGORY_RENDER, SDL_LOG_PRIORITY_INFO);
    SDL_LogSetPriority(SDL_LOG_CATEGORY_ERROR, SDL_LOG_PRIORITY_ERROR);
        
    _window = SDL_CreateWindow(APP_NAME, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600,
                                SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE);
        
    SDL_SetRelativeMouseMode(SDL_TRUE);
        
    inner(createDefaultGraphicsDevice(*_window));

    SDL_DestroyWindow(_window);
    _window = nullptr;
    SDL_Quit();
}
