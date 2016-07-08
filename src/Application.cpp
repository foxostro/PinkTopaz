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
    
    Application::Application() : _window(nullptr)
    {
        // Nothing to do
    }
    
    Application::~Application()
    {
        // Nothing to do
    }
    
    void Application::windowSizeChanged(int windowWidth, int windowHeight)
    {
        constexpr float znear = 0.1f;
        constexpr float zfar = 100.0f;
        float scaleFactor = windowScaleFactor(_window);
        glViewport(0, 0, windowWidth * scaleFactor, windowHeight * scaleFactor);
        glm::mat4 proj = glm::perspective(glm::pi<float>() * 0.25f, (float)windowWidth / windowHeight, znear, zfar);
        _shader->setUniform("proj", proj);
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

        SDL_version compiled;
        SDL_version linked;
        
        SDL_VERSION(&compiled);
        SDL_GetVersion(&linked);
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "We compiled against SDL version %d.%d.%d.\n", compiled.major, compiled.minor, compiled.patch);
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "We are linking against SDL version %d.%d.%d.\n", linked.major, linked.minor, linked.patch);
        
        _window = SDL_CreateWindow(APP_NAME, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600,
                                   SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE);

        SDL_LogSetPriority(SDL_LOG_CATEGORY_RENDER, SDL_LOG_PRIORITY_INFO);
        SDL_LogSetPriority(SDL_LOG_CATEGORY_ERROR, SDL_LOG_PRIORITY_ERROR);

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetSwapInterval(1);
        SDL_GLContext glContext = SDL_GL_CreateContext(_window);

        int major = 0;
        int minor = 0;
        glGetIntegerv(GL_MAJOR_VERSION, &major);
        glGetIntegerv(GL_MINOR_VERSION, &minor);
        SDL_LogInfo(SDL_LOG_CATEGORY_RENDER, "OpenGL version is %d.%d\n", major, minor);
        
        glClearColor(0.2, 0.4, 0.5, 1.0);
        
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        
        _shader = std::unique_ptr<Shader>(new Shader(stringFromFileContents("vert.glsl"),
                                                     stringFromFileContents("frag.glsl")));
        _shader->use();
        _shader->setUniform("view", glm::lookAt(glm::vec3(85.1, 16.1, 140.1),
                                                glm::vec3(80.1, 20.1, 130.1),
                                                glm::vec3(0, 1, 0)));
        _shader->setUniform("tex", 0);

        // Setup the projection matrix and viewport using the initial size of the window.
        {
            int windowWidth = 0, windowHeight = 0;
            SDL_GetWindowSize(_window, &windowWidth, &windowHeight);
            windowSizeChanged(windowWidth, windowHeight);
        }
        
        auto texture = std::unique_ptr<TextureArray>(new TextureArray("terrain.png"));
        auto vao = std::unique_ptr<StaticMeshVAO>(new StaticMeshVAO(StaticMesh("terrain.3d.bin")));

        // Now that setup is complete, check for OpenGL error before entering the game loop.
        checkGLError();
        
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
                                windowSizeChanged(e.window.data1, e.window.data2);
                                break;
                        }
                        break;
                }
            }
            
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            
            texture->bind();
            _shader->use();
            glBindVertexArray(vao->getVAO());
            glDrawArrays(GL_TRIANGLES, 0, vao->getNumVerts());
            
            glFlush();
            SDL_GL_SwapWindow(_window);
        }

        _shader.reset();
        vao.reset();
        texture.reset();

        SDL_GL_DeleteContext(glContext);
        SDL_DestroyWindow(_window);
        _window = nullptr;
        SDL_Quit();
    }
    
} // namespace PinkTopaz
