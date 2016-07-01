//
//  Application.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 6/25/16.
//
//

#include "SDL.h"
#include <vector>
#include <OpenGL/gl3.h>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>

#include "config.h"
#include "Application.hpp"
#include "Shader.hpp"
#include "glUtilities.hpp"
#include "fileUtilities.hpp"
#include "RetinaSupport.h"

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
        glUniformMatrix4fv(_projLoc, 1, GL_FALSE, glm::value_ptr(proj));
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

        static const GLfloat vertices[] = {
            0.0f,  0.5f,  0.0f,
            0.5f, -0.5f,  0.0f,
            -0.5f, -0.5f,  0.0f
        };
        
        GLuint vbo = 0;
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        
        GLuint vao = 0;
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glBindVertexArray(0);
        
        _shader = std::unique_ptr<Shader>(new Shader(stringFromFileContents("vert.glsl"),
                                                     stringFromFileContents("frag.glsl")));
        _shader->use();
        _viewLoc = glGetUniformLocation(_shader->getProgram(), "view");
        _projLoc = glGetUniformLocation(_shader->getProgram(), "proj");
        
        glm::mat4 view = glm::lookAt(glm::vec3(0, 0, -3), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
        glUniformMatrix4fv(_viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        
        // Setup the projection matrix and viewport using the initial size of the window.
        {
            int windowWidth = 0, windowHeight = 0;
            SDL_GetWindowSize(_window, &windowWidth, &windowHeight);
            windowSizeChanged(windowWidth, windowHeight);
        }
        
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
            
            checkGLError();
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            
            _shader->use();
            glBindVertexArray(vao);
            glDrawArrays(GL_TRIANGLES, 0, 3);
            
            glFlush();
            SDL_GL_SwapWindow(_window);
        }
        
        _shader.reset();

        SDL_GL_DeleteContext(glContext);
        SDL_DestroyWindow(_window);
        _window = nullptr;
        SDL_Quit();
    }
    
} // namespace PinkTopaz
