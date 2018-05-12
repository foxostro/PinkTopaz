//
//  main.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 6/24/16.
//  Copyright © 2016-2018 Andrew Fox. All rights reserved.
//

#include "Application.hpp"
#include <exception>

int main(int argc, char * arg[])
{
    try {
        Application game;
        game.run(); // Blocks until the window closes.
    } catch (const std::exception &exception) {
        fprintf(stderr, "Unhandled exception \"%s\". Aborting.\n", exception.what());
        abort();
    } catch(...) {
        fprintf(stderr, "Unhandled exception of unknown type. Aborting.\n");
        abort();
    }
    return EXIT_SUCCESS;
}
