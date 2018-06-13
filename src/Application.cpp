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
#include "MouseButtonEvent.hpp"
#include "MouseMoveEvent.hpp"
#include "SDLException.hpp"
#include "Profiler.hpp"
#include "VideoRefreshRate.hpp"
#include "AutoreleasePool.hpp"

#include "SDL.h"
#include <spdlog/spdlog.h>
#include <map>
#include <chrono>

#include "Application.hpp"

// Exception thrown when the system does not meet minimum requirements.
class ApplicationRequirementsException : public Exception
{
public:
    template<typename... Args>
    ApplicationRequirementsException(Args&&... args)
    : Exception(std::forward<Args>(args)...)
    {}
};

// Exception thrown when required AVX support is not available on this system.
class AVXUnsupportedException : public ApplicationRequirementsException
{
public:
    AVXUnsupportedException()
    : ApplicationRequirementsException("This application requires the AVX2 "
                                       "instruction set found on Intel Haswell "
                                       "chips, or newer.")
    {}
};

void Application::inner(const std::shared_ptr<GraphicsDevice> &graphicsDevice,
                        const std::shared_ptr<TaskDispatcher> &mainThreadDispatcher)
{
    // Get the display refresh rate. This is usually 60 Hz.
    const double refreshRate = getVideoRefreshRate().value_or(60.0);
	const auto videoRefreshPeriod = std::chrono::duration<double>(1 / refreshRate);
    
    World gameWorld(_log,
                    _preferences,
                    graphicsDevice,
                    mainThreadDispatcher);
    
    // Send an event containing the initial window size and scale factor.
    // This will allow the render system to setup projection matrices and such.
    {
        WindowSizeChangedEvent event;
        SDL_GetWindowSize(_window, &event.width, &event.height);
        event.windowScaleFactor = windowScaleFactor(_window);
        gameWorld.events.emit(event);
    }
    
	auto currentTime = std::chrono::high_resolution_clock::now();
    
    while (true) {
        PROFILER(Frame);
        AutoreleasePool pool;
        
        const auto newTime = std::chrono::high_resolution_clock::now();
        const auto frameDuration = newTime - currentTime;
        currentTime = newTime;
        const auto nextTime = currentTime + videoRefreshPeriod;
        SDL_Event e;
        
        while (SDL_PollEvent(&e)) {
            switch(e.type)
            {
                case SDL_QUIT:
                    PROFILER_SIGNPOST(Quit);
                    _log->info("Received SDL_QUIT.");
                    
                    // Some futures may be waiting on tasks in main thread dispatch queue.
                    // Shutdown order matters.
                    mainThreadDispatcher->shutdown();
                    return;
                    
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
                    gameWorld.events.emit(KeypressEvent(e.key.keysym.sym,
                                                        /* down= */ true));

					// Quit on Control+Q too.
					if (e.key.keysym.sym == SDLK_q && (SDL_GetModState() & KMOD_CTRL)) {
						SDL_Event event;
						event.type = SDL_QUIT;
                        if (SDL_PushEvent(&event) < 0) {
                            throw SDLException("SDL_PushEvent failed");
                        }
					}
                    break;
                    
                case SDL_KEYUP:
                    gameWorld.events.emit(KeypressEvent(e.key.keysym.sym,
                                                        /* down= */ false));
                    break;
                        
                case SDL_MOUSEMOTION:
                    gameWorld.events.emit(MouseMoveEvent(e.motion.xrel,
                                                         e.motion.yrel));
                    break;
                    
                case SDL_MOUSEBUTTONDOWN:
                    gameWorld.events.emit(MouseButtonEvent(e.button.button,
                                                           /* down= */ true));
                    break;
                    
                case SDL_MOUSEBUTTONUP:
                    gameWorld.events.emit(MouseButtonEvent(e.button.button,
                                                           /* down= */ false));
                    break;
            }
        }
        
        const entityx::TimeDelta frameDurationMs = std::chrono::duration_cast<std::chrono::milliseconds>(frameDuration).count();
        gameWorld.update(frameDurationMs);
        
        // Sometimes, we need to execute a task specifically on the main thread.
        // For example, a background task needs to update an entity component.
        mainThreadDispatcher->flush();
        
        std::this_thread::sleep_until(nextTime);
    }
}
    
void Application::run()
{
    AutoreleasePool pool;
    
    _log = spdlog::stdout_color_mt("console");
    
    // Load user preferences from file.
    {
        boost::filesystem::path prefsFileName = getPrefPath()/"preferences.xml";
        if (boost::filesystem::exists(prefsFileName)) {
            std::ifstream inputStream(prefsFileName.c_str());
            cereal::XMLInputArchive archive(inputStream);
            archive(cereal::make_nvp("preferences", _preferences));
            _log->info("Loaded user preferences from file \"{}\": {}",
                      prefsFileName.string(),
                      _preferences);
        } else {
            // Save the default preferences to file.
            std::ofstream outputStream(prefsFileName.c_str());
            cereal::XMLOutputArchive archive(outputStream);
            archive(cereal::make_nvp("preferences", _preferences));
            _log->info("Saved default user preferences to file \"{}\": {}",
                      prefsFileName.string(),
                      _preferences);
        }
    }
    
    _log->set_level(_preferences.logLevel);
    
    if (!SDL_HasAVX()) {
        throw AVXUnsupportedException();
    }
    
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        throw SDLException("SDL_Init failed");
    }

    // Set the current working directory to the SDL data path.
    // On Mac OS, this is the bundle "Resources" directory.
    {
        char *pathStr = SDL_GetBasePath();
        if (!pathStr) {
            throw SDLException("SDL_GetBasePath failed");
        }
        boost::filesystem::path cwd(pathStr);
        boost::filesystem::current_path(cwd);
        SDL_free(pathStr);
    }
        
    _window = SDL_CreateWindow(APP_NAME, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600,
                                SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI);
    if (!_window) {
        throw SDLException("SDL_CreateWindow failed");
    }
        
    if (SDL_SetRelativeMouseMode(SDL_TRUE) != 0) {
        throw SDLException("SDL_SetRelativeMouseMode failed");
    }
    
    inner(createDefaultGraphicsDevice(_log, *_window),
          std::make_shared<TaskDispatcher>("Main Thread Dispatcher", 0));

    SDL_DestroyWindow(_window);
    _window = nullptr;
    SDL_Quit();
}
