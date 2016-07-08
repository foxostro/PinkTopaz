//
//  Application.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 6/25/16.
//
//

#include "SDL.h"
#include "SDL_image.h"
#include <vector>
#include "opengl.hpp"
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
#include <entityx/entityx.h>

#include "config.h"
#include "Application.hpp"
#include "TextureArray.hpp"
#include "Shader.hpp"
#include "glUtilities.hpp"
#include "FileUtilities.hpp"
#include "RetinaSupport.h"
#include "StaticMesh.hpp"

namespace PinkTopaz {
    // Tags the entity which acts as the camera.
    struct ActiveCamera
    {
        ActiveCamera() {}
    };

    // Position and Orientation of an entity.
    struct Transform
    {
        Transform() {}
        
        Transform(const glm::mat4x4 &val) : value(val) {}
        
        Transform(const glm::vec3 &eye, const glm::vec3 &center, const glm::vec3 &up)
        {
            value = glm::lookAt(eye, center, up);
        }
        
        glm::mat4x4 value;
    };

    // Gives the entity a static mesh which is rendered to represent the entity in the world.
    struct RenderableStaticMesh
    {
        RenderableStaticMesh() {}
        
        RenderableStaticMesh(const std::shared_ptr<StaticMeshVAO> &vao,
                             const std::shared_ptr<Shader> &shader,
                             const std::shared_ptr<TextureArray> &texture)
        {
            this->vao = vao;
            this->shader = shader;
            this->texture = texture;
        }

        std::shared_ptr<StaticMeshVAO> vao;
        std::shared_ptr<Shader> shader;
        std::shared_ptr<TextureArray> texture;
    };
    
    // System for rendering static meshes in the world, associated with the RenderableStaticMesh components.
    class StaticMeshRenderSystem : public entityx::System<StaticMeshRenderSystem>
    {
    public:
        StaticMeshRenderSystem() {}
        
        void update(entityx::EntityManager &es,
                    entityx::EventManager &events,
                    entityx::TimeDelta dt) override
        {
            auto cameraTransform = getCameraTransform(es);
            
            es.each<RenderableStaticMesh, Transform>([cameraTransform](entityx::Entity entity,
                                                                       RenderableStaticMesh &mesh,
                                                                       Transform &transform)
            {
                auto shader = mesh.shader;
                auto texture = mesh.texture;
                auto vao = mesh.vao;
                
                glm::mat4x4 view = cameraTransform->value * transform.value;
                shader->setUniform("view", view);
                
                shader->bind();
                texture->bind();
                glBindVertexArray(vao->getVAO());
                glDrawArrays(GL_TRIANGLES, 0, vao->getNumVerts());
                shader->unbind();
            });
        }

        // Grab the first entity that is tagged with ActiveCamera and retrieve it's Transform.
        entityx::ComponentHandle<Transform> getCameraTransform(entityx::EntityManager &es)
        {
            entityx::ComponentHandle<Transform> cameraTransform;
            entityx::ComponentHandle<ActiveCamera> cameraTag;
            auto camera = *es.entities_with_components(cameraTag, cameraTransform).begin();
            cameraTransform = camera.component<Transform>();
            return cameraTransform;
        }
    };
    
    // A World is the same thing as a game zone or level.
    // This is a collection of interacting entities and associated systems.
    // It is, of course, entirely possible to have multiple worlds. However, interactions across worlds are not
    // intended to be routine or easily modeled.
    class World : public entityx::EntityX
    {
    public:
        explicit World(const std::shared_ptr<StaticMeshVAO> &vao,
                       const std::shared_ptr<Shader> &shader,
                       const std::shared_ptr<TextureArray> &texture)
        {
            systems.add<StaticMeshRenderSystem>();
            systems.configure();
            
            // Create an entity to represent the camera.
            // Render systems will look this entity up by using the ActiveCamera component as a tag.
            // They will retrieve it's transformation and take it into account when rendering their stuff.
            entityx::Entity camera = entities.create();
            camera.assign<Transform>(glm::vec3(85.1, 16.1, 140.1),
                                     glm::vec3(80.1, 20.1, 130.1),
                                     glm::vec3(0, 1, 0));
            camera.assign<ActiveCamera>();
            
            // Create an entity to represent the terrain.
            entityx::Entity terrain = entities.create();
            terrain.assign<RenderableStaticMesh>(vao, shader, texture);
            terrain.assign<Transform>(glm::mat4x4());
        }
        
        void update(entityx::TimeDelta dt)
        {
            systems.update<StaticMeshRenderSystem>(dt);
        }
    };
    
    Application::Application() : _window(nullptr)
    {
        // Nothing to do
    }
    
    Application::~Application()
    {
        // Nothing to do
    }
    
    void Application::windowSizeChanged(int windowWidth, int windowHeight, std::shared_ptr<Shader> &shader)
    {
        constexpr float znear = 0.1f;
        constexpr float zfar = 100.0f;
        float scaleFactor = windowScaleFactor(_window);
        glViewport(0, 0, windowWidth * scaleFactor, windowHeight * scaleFactor);
        glm::mat4 proj = glm::perspective(glm::pi<float>() * 0.25f, (float)windowWidth / windowHeight, znear, zfar);
        shader->setUniform("proj", proj);
    }
    
    void Application::inner()
    {
        auto shader = std::shared_ptr<Shader>(new Shader(stringFromFileContents("vert.glsl"),
                                                         stringFromFileContents("frag.glsl")));
        shader->setUniform("view", glm::mat4(1.0f));
        shader->setUniform("tex", 0);
        
        // Setup the projection matrix and viewport using the initial size of the window.
        {
            int windowWidth = 0, windowHeight = 0;
            SDL_GetWindowSize(_window, &windowWidth, &windowHeight);
            windowSizeChanged(windowWidth, windowHeight, shader);
        }
        
        auto texture = std::shared_ptr<TextureArray>(new TextureArray("terrain.png"));
        auto vao = std::shared_ptr<StaticMeshVAO>(new StaticMeshVAO(StaticMesh("terrain.3d.bin")));
        
        // Now that setup is complete, check for OpenGL error before entering the game loop.
        checkGLError();
        
        World gameWorld(vao, shader, texture);
        
        bool quit = false;
        
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
                            windowSizeChanged(e.window.data1, e.window.data2, shader);
                            break;
                    }
                        break;
                }
            }
            
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            gameWorld.update(0);
            glFlush();
            SDL_GL_SwapWindow(_window);
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
        
        // Set up the window and OpenGL context.
        _window = SDL_CreateWindow(APP_NAME, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600,
                                   SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetSwapInterval(1);
        SDL_GLContext glContext = SDL_GL_CreateContext(_window);

        // Check the OpenGL version and log an error if it's not supported.
        // But we'll try to run anyway.
        {
            int major = 0;
            int minor = 0;
            glGetIntegerv(GL_MAJOR_VERSION, &major);
            glGetIntegerv(GL_MINOR_VERSION, &minor);
            SDL_LogInfo(SDL_LOG_CATEGORY_RENDER, "OpenGL version is %d.%d\n", major, minor);
            
            if (!(major >= 4 && minor >= 1)) {
                SDL_LogError(SDL_LOG_CATEGORY_RENDER, "This application requires at least OpenGL 4.1 to run.");
            }
        }
        
        glClearColor(0.2, 0.4, 0.5, 1.0);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        
        inner();

        SDL_GL_DeleteContext(glContext);
        SDL_DestroyWindow(_window);
        _window = nullptr;
        SDL_Quit();
    }
    
} // namespace PinkTopaz
