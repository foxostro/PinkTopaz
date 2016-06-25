//
//  main.m
//  PinkTopaz
//
//  Created by Andrew Fox on 6/24/16.
//  Copyright © 2016 Andrew Fox. All rights reserved.
//

#include <cstdio>
#include <thread>
#include <atomic>
#include <OpenGL/gl3.h>
#include <SDL2/SDL.h>
#include "config.h"


static std::atomic_bool shouldQuit(false);


void renderLoop(SDL_Window *window)
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
    
    int numIter = 200;
    int countDown = numIter;
    Uint64 accum = 0;

    while(!shouldQuit)
    {
        Uint64 startTime = SDL_GetPerformanceCounter();
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glFlush();
        SDL_GL_SwapWindow(window);

        Uint64 finishTime = SDL_GetPerformanceCounter();
        accum += (finishTime - startTime);
        countDown--;
        
        if (countDown <= 0) {
            double elapsed = (double)(accum*1000.0/numIter) / SDL_GetPerformanceFrequency();
            SDL_LogInfo(SDL_LOG_CATEGORY_RENDER, "%d frames took an average of %.001f ms each.", numIter, elapsed);

            countDown = numIter;
            accum = 0;
        }
    }
    
    SDL_LogInfo(SDL_LOG_CATEGORY_RENDER, "Exiting render loop.");
}


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
    
    std::thread thr(renderLoop, window);
    
    while(!shouldQuit)
    {
        SDL_Event e;
        
        while(SDL_WaitEvent(&e) != 0)
        {
            if (e.type == SDL_QUIT) {
                shouldQuit = true;
                break;
            }
        }
    }
    
    thr.join();
    
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return EXIT_SUCCESS;
}
