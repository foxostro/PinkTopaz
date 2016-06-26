//
//  main.m
//  PinkTopaz
//
//  Created by Andrew Fox on 6/24/16.
//  Copyright © 2016 Andrew Fox. All rights reserved.
//

#include "SDL.h"
#include "Window.hpp"

int main(int argc, char * arg[])
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SDL_Init failed: %s\n", SDL_GetError());
    }
    
    SDL_version compiled;
    SDL_version linked;
    
    SDL_VERSION(&compiled);
    SDL_GetVersion(&linked);
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "We compiled against SDL version %d.%d.%d.\n", compiled.major, compiled.minor, compiled.patch);
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "We are linking against SDL version %d.%d.%d.\n", linked.major, linked.minor, linked.patch);

    {
        PinkTopaz::Window window;
        window.run(); // Blocks until the window closes.
    }
    
    SDL_Quit();

    return EXIT_SUCCESS;
}
