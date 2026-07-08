#include "libcninput.h"

CN_API cnbool start_input(void)
{
    if (SDL_Init(SDL_INIT_GAMECONTROLLER)) {
        RAISE(ERR_OS, "failed to init gamecontroller sdl module.");
        return (false);
    }
    if (SDL_Init(SDL_INIT_HAPTIC)) {
        RAISE(ERR_OS, "failed to init haptic sdl module.");
        return (false);
    }
    if (SDL_Init(SDL_INIT_JOYSTICK)) {
        RAISE(ERR_OS, "failed to init joystick sdl module.");
        return (false);
    }
    return (true);
}

CN_API void end_input(void)
{
    (void)SDL_QuitSubSystem(SDL_INIT_GAMECONTROLLER);
    (void)SDL_QuitSubSystem(SDL_INIT_HAPTIC);
    (void)SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
}
