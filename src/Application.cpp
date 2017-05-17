//
//  Application.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 6/25/16.
//
//

#include "Application.hpp"

#include "pinktopaz_config.h"
#include "FileUtilities.hpp"
#include "RetinaSupport.h"
#include "World.hpp"
#include "WindowSizeChangedEvent.hpp"
#include "KeypressEvent.hpp"
#include "MouseMoveEvent.hpp"
#include "Terrain/VoxelDataLoader.hpp"
#include "Terrain/VoxelDataStore.hpp"
#include "Terrain/MesherMarchingCubes.hpp"
#include "Exception.hpp"

#include "SDL.h"
#include "SDL_image.h"
#include <map>
#include <glm/glm.hpp>

RenderableStaticMesh Application::createTerrainMesh(const std::shared_ptr<GraphicsDevice> &graphicsDevice)
{
    // Load terrain texture array from a single image.
    // TODO: create a TextureArrayLoader class to encapsulate tex loading.
    SDL_Surface *surface = IMG_Load("terrain.png");
    
    if (!surface) {
        throw Exception("Failed to load terrain terrain.png.");
    }

    TextureDescriptor texDesc = {
        Texture2DArray,
        BGRA8,
        static_cast<size_t>(surface->w),
        static_cast<size_t>(surface->w),
        static_cast<size_t>(surface->h / surface->w),
        4,
        true,
    };
    auto texture = graphicsDevice->makeTexture(texDesc, surface->pixels);
        
    TextureSamplerDescriptor samplerDesc = {
        ClampToEdge,
        ClampToEdge,
        NearestMipMapNearest,
        Nearest
    };
    auto sampler = graphicsDevice->makeTextureSampler(samplerDesc);
    
    // Create a voxel data store and fill it with voxel data we load from file.
    // We need the voxel data store to use the dimensions and resolution of the
    // data contained in the file.
    std::vector<uint8_t> bytes = binaryFileContents("0_0_0.voxels.dat");
    AABB box;
    glm::ivec3 res;
    VoxelDataLoader voxelDataLoader;
    voxelDataLoader.retrieveDimensions(bytes, box, res);
    VoxelDataStore voxelDataStore(box, res);
    voxelDataStore.writerTransaction([&](VoxelData &voxels){
        voxelDataLoader.load(bytes, voxels);
    });
    
    // The voxel file uses a binary SOLID/EMPTY flag for voxels. So, we get
    // values that are either 0.0 or 1.0.
    std::shared_ptr<Mesher> mesher(new MesherMarchingCubes());
    StaticMesh mesh;
    voxelDataStore.readerTransaction([&](const VoxelData &voxels){
        mesh = mesher->extract(voxels, 0.5f);
    });
        
    auto vertexBufferData = mesh.getBufferData();
    auto vertexBuffer = graphicsDevice->makeBuffer(vertexBufferData,
                                                   StaticDraw,
                                                   ArrayBuffer);
    vertexBuffer->addDebugMarker("Terrain Vertices", 0, vertexBufferData.size());
        
    TerrainUniforms uniforms;
    auto uniformBuffer = graphicsDevice->makeBuffer(sizeof(uniforms),
                                                    &uniforms,
                                                    DynamicDraw,
                                                    UniformBuffer);
    vertexBuffer->addDebugMarker("Terrain Uniforms", 0, sizeof(uniforms));
        
    auto shader = graphicsDevice->makeShader(mesh.getVertexFormat(),
                                             "vert", "frag",
                                             false);
    
    RenderableStaticMesh meshContainer = {
        mesh.getVertexCount(),
        vertexBuffer,
        uniformBuffer,
        shader,
        texture,
        sampler
    };
        
    return meshContainer;
}
    
void Application::inner(const std::shared_ptr<GraphicsDevice> &graphicsDevice)
{
    std::map<SDL_Keycode, bool> prevKeyStates, keyStates;
        
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
        char *path = SDL_GetBasePath();
        setCurrentWorkingDirectory(path);
        SDL_free(path);
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
