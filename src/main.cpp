//
//  main.m
//  PinkTopaz
//
//  Created by Andrew Fox on 6/24/16.
//  Copyright © 2016 Andrew Fox. All rights reserved.
//

#include <cstdio>
#include <SDL2/SDL.h>
#include "config.h"
#include "Renderer.hpp"

int main(int argc, char * arg[])
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to initialize SDL.\n");
        return EXIT_FAILURE;
    }
    
    SDL_version compiled;
    SDL_version linked;
    
    SDL_VERSION(&compiled);
    SDL_GetVersion(&linked);
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "We compiled against SDL version %d.%d.%d.\n", compiled.major, compiled.minor, compiled.patch);
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "We are linking against SDL version %d.%d.%d.\n", linked.major, linked.minor, linked.patch);
    
    SDL_Window *window = SDL_CreateWindow(APP_NAME, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE);

    PinkTopaz::Renderer::Renderer renderer(window);
    renderer.start();

    while(true)
    {
        SDL_Event e;
        
        while(SDL_WaitEvent(&e) != 0)
        {
            if (e.type == SDL_QUIT) {
                renderer.setShouldShutdown();
                goto breakOut;
            }
        }
    }
breakOut:

    renderer.finish();

    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return EXIT_SUCCESS;
}
