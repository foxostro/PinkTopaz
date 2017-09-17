//
//  main.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 6/24/16.
//  Copyright © 2016-2017 Andrew Fox. All rights reserved.
//

#include "SDL.h"
#include "Application.hpp"

int main(int argc, char * arg[])
{
    Application game;
    game.run(); // Blocks until the window closes.
    return EXIT_SUCCESS;
}
