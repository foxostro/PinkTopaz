//
//  Renderer.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 6/25/16.
//
//

#include <OpenGL/gl3.h>
#include <SDL2/SDL.h>

#include "Renderer.hpp"

namespace PinkTopaz::Renderer {
    
    Actor::Actor() : shouldShutdown(false)
    {
        // Nothing to do
    }
    
    Actor::~Actor()
    {
        // Nothing to do
    }
    
    void Actor::start()
    {
        thr = std::unique_ptr<std::thread>(new std::thread([this]{ this->run(); }));
    }
    
    void Actor::finish()
    {
        setShouldShutdown();
        thr->join();
    }
    
    void Actor::setShouldShutdown()
    {
        shouldShutdown = true;
    }
    
    void Actor::run()
    {
        preLoop();
        
        while(!shouldShutdown)
        {
            pump();
        }

        postLoop();
    }
    
    Renderer::Renderer(SDL_Window *_window) : window(_window)
    {
        // Nothing to do
    }
    
    void Renderer::preLoop()
    {
        SDL_LogSetPriority(SDL_LOG_CATEGORY_RENDER, SDL_LOG_PRIORITY_INFO);
        
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetSwapInterval(1);
        SDL_GL_CreateContext(window);
        
        int major = 0;
        int minor = 0;
        glGetIntegerv(GL_MAJOR_VERSION, &major);
        glGetIntegerv(GL_MINOR_VERSION, &minor);
        SDL_LogInfo(SDL_LOG_CATEGORY_RENDER, "OpenGL version is %d.%d\n", major, minor);
        
        glClearColor(0.2, 0.4, 0.5, 1.0);
    }
    
    void Renderer::pump()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glFlush();
        SDL_GL_SwapWindow(window);
    }
    
    void Renderer::postLoop()
    {        
        SDL_LogInfo(SDL_LOG_CATEGORY_RENDER, "Exiting render loop.");
    }
    
} // namespace PinkTopaz::Renderer