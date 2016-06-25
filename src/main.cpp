#include <cstdio>
#include "SDL.h"

int main(void)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init failed\n");
        return EXIT_FAILURE;
    }

    SDL_version compiled;
    SDL_version linked;

    SDL_VERSION(&compiled);
    SDL_GetVersion(&linked);
    printf("We compiled against SDL version %d.%d.%d.\n", compiled.major, compiled.minor, compiled.patch);
    printf("We are linking against SDL version %d.%d.%d.\n", linked.major, linked.minor, linked.patch);

    SDL_Quit();
    return EXIT_SUCCESS;
}
