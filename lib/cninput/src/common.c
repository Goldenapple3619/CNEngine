#include "libcninput.h"

CN_API cnbool start_input(void)
{
    if (SDL_Init(SDL_INIT_GAMECONTROLLER))
        return (false);
    if (SDL_Init(SDL_INIT_HAPTIC))
        return (false);
    if (SDL_Init(SDL_INIT_JOYSTICK))
        return (false);
    return (true);
}

CN_API void end_input(void)
{
    (void)SDL_QuitSubSystem(SDL_INIT_GAMECONTROLLER);
    (void)SDL_QuitSubSystem(SDL_INIT_HAPTIC);
    (void)SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
}
