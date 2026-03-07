#include "libcngraphic.h"

CN_API cnbool start_graphics(void)
{
    if (SDL_Init(SDL_INIT_TIMER) != 0)
        return (false);
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
        return (false);
    if (SDL_Init(SDL_INIT_EVENTS) != 0)
        return (false);
    if ((IMG_Init(IMG_INIT_JPG) & IMG_INIT_JPG) != IMG_INIT_JPG)
        return (false);
    if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_JPG) != IMG_INIT_JPG)
        return (false);
    return (true);
}

CN_API void end_graphics(void)
{
    IMG_Quit();
    SDL_Quit();
}
