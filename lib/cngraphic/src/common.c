#include "libcngraphic.h"
#include <SDL2/SDL.h>
#include <SDL_image.h>

CN_API cnbool start_graphics(void)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        RAISE(ERR_OS, "failed to init video.");
        return (false);
    }

    if (SDL_Init(SDL_INIT_EVENTS) != 0) {
        RAISE(ERR_OS, "failed to init events.");
        return (false);
    }

    if (SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3)) {
        RAISE(ERR_OS, "failed to set GL major version to 3.");
        return (false);
    }
    if (SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3)) {
        RAISE(ERR_OS, "failed to set GL minor version to 3.");
        return (false);
    }
    if (SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE)) {
        RAISE(ERR_OS, "failed to set GL context profile mask to gl context profile core.");
        return (false);
    }

    if ((IMG_Init(IMG_INIT_JPG) & IMG_INIT_JPG) != IMG_INIT_JPG) {
        RAISE(ERR_OS, "failed to load image format for jpeg.");
        return (false);
    }
    if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) != IMG_INIT_PNG) {
        RAISE(ERR_OS, "failed to load image format for png.");
        return (false);
    }
    return (true);
}

CN_API void end_graphics(void)
{
    (void)SDL_QuitSubSystem(SDL_INIT_EVENTS);
    (void)SDL_QuitSubSystem(SDL_INIT_VIDEO);
    (void)IMG_Quit();
}
