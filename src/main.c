#define SDL_MAIN_HANDLED

#include <stdio.h>
#include <signal.h>
#include "libcncore.h"
#include "libcnaudio.h"
#include "libcninput.h"
#include "libcngraphic.h"
#include "librgui.h"
#include "libr2d.h"
#include "libr3d.h"
#include "engine.h"

static Object *global_ctx = NULL;

static void sigint_handler(int signum)
{
    (void)signum;

    if (global_ctx && has_method(global_ctx, "_stop"))
        call_method(global_ctx, "_stop", NULL);
}

Object *build_engine(void)
{
    Object *ctx = new_ctx();

    if (!ctx)
        return (NULL);

    global_ctx = ctx;

    if (!submodule_ctx(ctx, new_graphic_submodule())) {
        (void)delete_object(ctx);
        return (NULL);
    }

    if (!submodule_ctx(ctx, new_audio_ctx(true))) {
        (void)delete_object(ctx);
        return (NULL);
    }

    if (!submodule_ctx(ctx, new_gui_submodule())) {
        (void)delete_object(ctx);
        return (NULL);
    }

    if (!submodule_ctx(ctx, new_input_submodule())) {
        (void)delete_object(ctx);
        return (NULL);
    }

    cn_value val = call_method(ctx, "_init", NULL);

    if (val.type == CN_TYPE_NULL || val.as.i == VALUE_ERR.as.i) {
        delete_object(ctx);
        return (NULL);
    }

    signal(SIGINT, &sigint_handler);

    return (ctx);
}

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    Object *ctx = build_engine();

    if (!ctx)
        return (1);

    if (!add_map_editor_window(ctx)) {
        delete_object(ctx);
        return (1);
    }

    call_method(ctx, "_run", NULL);
    delete_object(ctx);

    run_gc();

    return (0);
}