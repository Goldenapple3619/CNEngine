#include "libcngraphic.h"

CN_API cnbool start_graphics(void)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
        return (false);
    if (SDL_Init(SDL_INIT_EVENTS) != 0)
        return (false);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    if ((IMG_Init(IMG_INIT_JPG) & IMG_INIT_JPG) != IMG_INIT_JPG)
        return (false);
    if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) != IMG_INIT_PNG)
        return (false);
    return (true);
}

CN_API void end_graphics(void)
{
    (void)SDL_QuitSubSystem(SDL_INIT_EVENTS);
    (void)SDL_QuitSubSystem(SDL_INIT_VIDEO);
    (void)IMG_Quit();
}
