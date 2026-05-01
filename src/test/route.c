#include "test.h"

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

int test(size_t argc, char **argv)
{
    (void)argc;
    (void)argv;

    Object *ctx = build_engine();

    if (!ctx)
        return (1);

    add_test_window(ctx);

    call_method(ctx, "_run", NULL);
    delete_object(ctx);

    run_gc();

    return (0);
}
