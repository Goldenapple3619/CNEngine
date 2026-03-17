#define SDL_MAIN_HANDLED

#include <stdio.h>
#include <signal.h>
#include "libcncore.h"
#include "libcngraphic.h"

static Object *global_ctx = NULL;

void sigint_handler(int signum)
{
    (void)signum;

    if (global_ctx && has_method(global_ctx, "_stop"))
        call_method(global_ctx, "_stop", NULL);
}

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    Videomode v = {.size.x = 800, .size.y = 600, .position.x = 0, .position.y = 0, .flags = VDM_CLOSABLE, .native_flags = VDM_N_SHWN};
    Object *ctx = new_ctx();

    if (!ctx)
        return (1);

    global_ctx = ctx;

    if (!submodule_ctx(ctx, new_graphic_submodule()))
        return (1);

    cn_value val = call_method(ctx, "_init", NULL);

    if (val.type == CN_TYPE_NULL || val.as.i == VALUE_ERR.as.i) {
        delete_object(ctx);
        return (1);
    }
    
    val = call_method(ctx, "spawn_interface", (void *[]){"test", NULL, &v});

    if (val.type == CN_TYPE_NULL) {
        delete_object(ctx);
        return (1);
    }

    signal(SIGINT, &sigint_handler);
    call_method(ctx, "_run", NULL);
    delete_object(ctx);
    return (0);
}