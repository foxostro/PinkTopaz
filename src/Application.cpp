//
//  Application.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 6/25/16.
//
//

#include "SDL.h"
#include <vector>

#include "config.h"
#include "Application.hpp"
#include "Shader.hpp"
#include "glUtilities.hpp"
#include "fileUtilities.hpp"

namespace PinkTopaz {
    
    Application::Application()
    {
        // Nothing to do
    }
    
    Application::~Application()
    {
        // Nothing to do
    }
    
    void Application::runInnerScope(SDL_Window *window)
    {
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

        Shader shader(stringFromFileContents("vert.glsl.txt"),
                      stringFromFileContents("frag.glsl.txt"));
        
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
                }
            }
            
            checkGLError();
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            
            shader.use();
            glBindVertexArray(vao);
            glDrawArrays(GL_TRIANGLES, 0, 3);
            
            glFlush();
            SDL_GL_SwapWindow(window);
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

        SDL_version compiled;
        SDL_version linked;
        
        SDL_VERSION(&compiled);
        SDL_GetVersion(&linked);
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "We compiled against SDL version %d.%d.%d.\n", compiled.major, compiled.minor, compiled.patch);
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "We are linking against SDL version %d.%d.%d.\n", linked.major, linked.minor, linked.patch);
        
        SDL_Window *window = SDL_CreateWindow(APP_NAME, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600,
                                  SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE);

        SDL_LogSetPriority(SDL_LOG_CATEGORY_RENDER, SDL_LOG_PRIORITY_INFO);
        SDL_LogSetPriority(SDL_LOG_CATEGORY_ERROR, SDL_LOG_PRIORITY_ERROR);

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetSwapInterval(1);
        SDL_GLContext glContext = SDL_GL_CreateContext(window);

        int major = 0;
        int minor = 0;
        glGetIntegerv(GL_MAJOR_VERSION, &major);
        glGetIntegerv(GL_MINOR_VERSION, &minor);
        SDL_LogInfo(SDL_LOG_CATEGORY_RENDER, "OpenGL version is %d.%d\n", major, minor);
        
        runInnerScope(window);

        SDL_GL_DeleteContext(glContext);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }
    
} // namespace PinkTopaz
