//
//  Application.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 6/25/16.
//
//

#include "Application.hpp"

#include "SDL.h"
#include "config.h"
#include "FileUtilities.hpp"
#include "RetinaSupport.h"
#include "World.hpp"
#include "WindowSizeChangedEvent.hpp"

#include "Renderer/OpenGL/GraphicsDeviceOpenGL.hpp"

namespace PinkTopaz {
    
    Application::Application() : _window(nullptr)
    {
        // Nothing to do
    }
    
    Application::~Application()
    {
        // Nothing to do
    }
    
    std::shared_ptr<Renderer::GraphicsDevice> Application::createGraphicsDevice()
    {
        assert(_window);
        SDL_Window &window = *_window;
        auto concreteGraphicsDevice = std::make_shared<Renderer::OpenGL::GraphicsDeviceOpenGL>(window);
        auto abstractGraphicsDevice = std::dynamic_pointer_cast<Renderer::GraphicsDevice>(concreteGraphicsDevice);
        return abstractGraphicsDevice;
    }
    
    void Application::inner()
    {
        auto graphicsDevice = createGraphicsDevice();

        auto shader = graphicsDevice->makeShader("vert", "frag");
        shader->setShaderUniform("view", glm::mat4(1.0f));
        shader->setShaderUniform("tex", 0);
        
        auto texture = graphicsDevice->makeTextureArray("terrain.png");

        auto mesh = std::make_shared<Renderer::StaticMesh>("terrain.3d.bin");
        auto buffer = graphicsDevice->makeBuffer(mesh->getVertexFormat(),
                                                 mesh->getBufferData(),
                                                 mesh->getVertexCount(),
                                                 Renderer::BufferUsageStaticDraw);
        
        World gameWorld(graphicsDevice, buffer, shader, texture);
        
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
        unsigned timeAccum = 0;
        unsigned framesBetweenFpsReport = 60;
        unsigned countDown = framesBetweenFpsReport;
        
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
                }
            }
            
            unsigned ticksEndMs = SDL_GetTicks();
            unsigned ticksElapsedMs = ticksEndMs - ticksBeginMs;
            timeAccum += ticksElapsedMs;
            ticksBeginMs = ticksEndMs;
            entityx::TimeDelta dt = ticksElapsedMs;
            
            if (countDown == 0) {
                float frameTime = (float)timeAccum / (float)framesBetweenFpsReport;
                SDL_Log("frame time: %.3f ms", frameTime);
                countDown = framesBetweenFpsReport;
                timeAccum = 0;
            } else {
                countDown--;
            }

            gameWorld.update(dt);
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
            char *path = SDL_GetBasePath();
            setCurrentWorkingDirectory(path);
            SDL_free(path);
        }
        
        SDL_LogSetPriority(SDL_LOG_CATEGORY_RENDER, SDL_LOG_PRIORITY_INFO);
        SDL_LogSetPriority(SDL_LOG_CATEGORY_ERROR, SDL_LOG_PRIORITY_ERROR);
        
        _window = SDL_CreateWindow(APP_NAME, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600,
                                   SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE);
        
        inner();

        SDL_DestroyWindow(_window);
        _window = nullptr;
        SDL_Quit();
    }
    
} // namespace PinkTopaz
