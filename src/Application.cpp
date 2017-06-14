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
#include "Exception.hpp"
#include "Profiler.hpp"
#include "VideoRefreshRate.hpp"
#include "Stopwatch.hpp"
#include "UnitTestDetector.h"

#include "SDL.h"
#include <map>

#include "Application.hpp"

void Application::inner(const std::shared_ptr<GraphicsDevice> &graphicsDevice,
                        const std::shared_ptr<TaskDispatcher> &dispatcher)
{
    Stopwatch stopwatch;
    
    // Get the display refresh rate. This is usually 60 Hz.
    const double refreshRate = getVideoRefreshRate();
    const uint64_t videoRefreshPeriodNanos = (uint64_t)(Stopwatch::NANOS_PER_SEC / refreshRate);
    
    World gameWorld(graphicsDevice, dispatcher);
    
    // Send an event containing the initial window size and scale factor.
    // This will allow the render system to setup projection matrices and such.
    {
        WindowSizeChangedEvent event;
        SDL_GetWindowSize(_window, &event.width, &event.height);
        event.windowScaleFactor = windowScaleFactor(_window);
        gameWorld.events.emit(event);
    }
    
    uint64_t currentTime = stopwatch.getCurrentTimeInNanos();
    bool quit = false;
    
    while(!quit)
    {
        PROFILER(Frame);
        
        const uint64_t newTime = stopwatch.getCurrentTimeInNanos();
        const uint64_t frameTime = newTime - currentTime;
        currentTime = newTime;
        const uint64_t nextTime = currentTime + videoRefreshPeriodNanos;
        SDL_Event e;
        
        if (SDL_PollEvent(&e)) {
            switch(e.type)
            {
                case SDL_QUIT:
                    PROFILER_SIGNPOST(Quit);
                    SDL_Log("Received SDL_QUIT.");
                    quit = true;
                    dispatcher->shutdown();
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
            }
        }
        
        gameWorld.update(frameTime / Stopwatch::NANOS_PER_MILLISEC);
        stopwatch.waitUntil(nextTime);
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
    
    // If we're running unit tests then don't bother actually starting the game.
    // When running unit tests, it is sufficient to get into the event loop.
    if(areWeBeingUnitTested()) {
        bool quit = false;
        while (!quit) {
            SDL_Event e;
            SDL_PollEvent(&e);
            switch(e.type)
            {
                case SDL_QUIT:
                    SDL_Log("Received SDL_QUIT.");
                    quit = true;
                    break;
            }
        }
    } else {
        inner(createDefaultGraphicsDevice(*_window),
              std::make_shared<TaskDispatcher>());
    }

    SDL_DestroyWindow(_window);
    _window = nullptr;
    SDL_Quit();
}
