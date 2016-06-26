//
//  Renderer.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 6/25/16.
//
//

#include "SDL.h"
#include "config.h"
#include "Renderer.hpp"
#include "Window.hpp"

namespace PinkTopaz {
    
    Window::Window()
    {
        // Nothing to do
    }
    
    Window::~Window()
    {
        // Nothing to do
    }
    
    void Window::run()
    {
        SDL_Window *window = SDL_CreateWindow(APP_NAME, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600,
                                  SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE);

        Renderer::Renderer renderer(window);
        renderer.start();
        
        bool quit = false;
        
        while(!quit)
        {
            SDL_Event e;
            
            if (!SDL_WaitEvent(&e)) {
                SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SDL_WaitEvent failed: %s\n", SDL_GetError());
                return;
            } else switch(e.type) {
                case SDL_QUIT:
                    SDL_Log("Recived SDL_QUIT.");
                    quit = true;
                    renderer.setShouldShutdown();
                    break;
            }
        }

        renderer.join();
        SDL_DestroyWindow(window);
    }
    
} // namespace PinkTopaz
