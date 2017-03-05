//
//  Application.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 6/25/16.
//
//

#include "Application.hpp"

#include "config.h"
#include "FileUtilities.hpp"
#include "RetinaSupport.h"
#include "World.hpp"
#include "WindowSizeChangedEvent.hpp"

#include "SDL.h"
#include "SDL_image.h"

namespace PinkTopaz {
    
    RenderableStaticMesh Application::createTerrainMesh(const std::shared_ptr<Renderer::GraphicsDevice> &graphicsDevice)
    {
        auto shader = graphicsDevice->makeShader("vert", "frag");
        
        // Load terrain texture array from a single image.
        // TODO: create a TextureArrayLoader class to encapsulate tex loading.
        SDL_Surface *surface = IMG_Load("terrain.png");
        Renderer::TextureDescriptor texDesc = {
            .type = Renderer::Texture2DArray,
            .format = Renderer::BGRA8,
            .width = static_cast<size_t>(surface->w),
            .height = static_cast<size_t>(surface->w),
            .depth = static_cast<size_t>(surface->h / surface->w),
            .unpackAlignment = 4,
            .generateMipMaps = true,
        };
        auto texture = graphicsDevice->makeTexture(texDesc, surface->pixels);
        
        Renderer::TextureSamplerDescriptor samplerDesc = {
            .addressS = Renderer::ClampToEdge,
            .addressT = Renderer::ClampToEdge,
            .minFilter = Renderer::NearestMipMapNearest,
            .maxFilter = Renderer::Nearest
        };
        auto sampler = graphicsDevice->makeTextureSampler(samplerDesc);
        
        auto mesh = std::make_shared<Renderer::StaticMesh>("terrain.3d.bin");
        auto vertexBuffer = graphicsDevice->makeBuffer(mesh->getVertexFormat(),
                                                       mesh->getBufferData(),
                                                       Renderer::StaticDraw);
        
        Renderer::StaticMesh::Uniforms uniforms;
        auto uniformBuffer = graphicsDevice->makeUniformBuffer(sizeof(uniforms), Renderer::DynamicDraw);
        uniformBuffer->replace(sizeof(uniforms), &uniforms);
        
        RenderableStaticMesh meshContainer = {
            .vertexCount = mesh->getVertexCount(),
            .buffer = vertexBuffer,
            .uniforms = uniformBuffer,
            .shader = shader,
            .texture = texture,
            .textureSampler = sampler
        };
        
        return meshContainer;
    }
    
    void Application::inner(const std::shared_ptr<Renderer::GraphicsDevice> &graphicsDevice)
    {
        RenderableStaticMesh mesh = createTerrainMesh(graphicsDevice);
        World gameWorld(graphicsDevice, mesh);
        
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
                }
            }
            
            unsigned ticksEndMs = SDL_GetTicks();
            unsigned ticksElapsedMs = ticksEndMs - ticksBeginMs;
            ticksBeginMs = ticksEndMs;
            entityx::TimeDelta dt = ticksElapsedMs;

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
        
        inner(Renderer::createDefaultGraphicsDevice(*_window));

        SDL_DestroyWindow(_window);
        _window = nullptr;
        SDL_Quit();
    }
    
} // namespace PinkTopaz
