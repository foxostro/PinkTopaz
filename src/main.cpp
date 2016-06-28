//
//  main.m
//  PinkTopaz
//
//  Created by Andrew Fox on 6/24/16.
//  Copyright © 2016 Andrew Fox. All rights reserved.
//

#include "SDL.h"
#include "Application.hpp"

int main(int argc, char * arg[])
{
    PinkTopaz::Application game;
    game.run(); // Blocks until the window closes.
    return EXIT_SUCCESS;
}
