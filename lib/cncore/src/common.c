#include "libcncore.h"
#include <SDL2/SDL.h>

CN_API cnbool start_core(void)
{
    return (true);
}

CN_API void end_core(void)
{
    (void)SDL_Quit();
}