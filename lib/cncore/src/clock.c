#include "libcncore.h"

CN_API cnnumber clock_tick(Clock *c, int32_t tps)
{
    if (!c)
        return (0.0);
    cnnumber execution_time = 1000.0 / tps;
    cntime now = SDL_GetTicks64();
    cnnumber delta = (cnnumber)(now - c->old_time);

    if (tps == -1) {
        c->old_time = now;
        c->last_dt = delta;
        return (c->last_dt);
    }

    cnnumber sleep_time = execution_time - delta;

    if (sleep_time >= 1.0)
        (void)SDL_Delay((uint32_t)round(sleep_time));

    c->old_time = SDL_GetTicks64();
    c->last_dt = sleep_time > 0.0 ? delta + sleep_time : delta;
    return (c->last_dt);
}

CN_API Clock *new_clock(void)
{
    Clock *c = (Clock *)malloc(sizeof(Clock));

    if (!c)
        return (NULL);

    return (c);
}

CN_API void delete_clock(Clock *c)
{
    if (!c)
        return;
    (void)free((void *)c);
}
