#include "test.h"

cn_value show_fps(Object *__this, void **args)
{
    if (!args || !args[0])
        return (null_value);

    static uint64_t freq = 0;

    if (freq == 0)
        freq = SDL_GetPerformanceFrequency();

    const uint64_t c = SDL_GetPerformanceCounter();

    const uint64_t old_time = has_attr(__this, "last_time") ? get_attr(__this, "last_time")->as.i : 0;
    const uint64_t new_time = (c / freq) * 1000000000ULL + (c % freq) * 1000000000ULL / freq;

    INIT_INT(__this, new_time, "last_time")

    char fps_text[23];

    if (new_time <= 0)
        return (null_value);

    snprintf(fps_text, sizeof(fps_text), "%.2f fps", 1e9 / (new_time - old_time));
    call_method(__this, "set_text", PACK_ARG(fps_text));
    return (null_value);
}
