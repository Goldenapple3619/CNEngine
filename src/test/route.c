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

    if (!ctx) {
        PROPAGATE_ERR();
        return (NULL);
    }

    global_ctx = ctx;

    if (!submodule_ctx(ctx, new_graphic_submodule())) {
        PROPAGATE_ERR();
        (void)delete_object(ctx);
        return (NULL);
    }

    if (!submodule_ctx(ctx, new_audio_ctx(true))) {
        PROPAGATE_ERR();
        (void)delete_object(ctx);
        return (NULL);
    }

    if (!submodule_ctx(ctx, new_gui_submodule())) {
        PROPAGATE_ERR();
        (void)delete_object(ctx);
        return (NULL);
    }

    if (!submodule_ctx(ctx, new_input_submodule())) {
        PROPAGATE_ERR();
        (void)delete_object(ctx);
        return (NULL);
    }

    cn_value val = call_method(ctx, "_init", NULL);

    if (val.type == CN_TYPE_NULL || val.as.i == VALUE_ERR.as.i) {
        PROPAGATE_ERR();
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

    if (!ctx) {
        PROPAGATE_ERR();
        return (1);
    }

    if (!add_test_window(ctx)) {
        PROPAGATE_ERR();
        DELOC(ctx);
        return (1);
    }

    call_method(ctx, "_run", NULL);
    DELOC(ctx);

    return (0);
}
