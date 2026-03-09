#include "libcnaudio.h"

CN_API cnbool start_audio(void)
{
    if (SDL_Init(SDL_INIT_AUDIO) != 0)
        return (false);
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
        return (false);

    return (true);
};

CN_API void end_audio(void)
{
    Mix_CloseAudio();
    SDL_Quit();
};