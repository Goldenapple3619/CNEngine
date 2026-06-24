#include "engine_main.h"

static Object *global_ctx = NULL;

static void sigint_handler(int signum)
{
    (void)signum;

    if (global_ctx && has_method(global_ctx, "_stop"))
        call_method(global_ctx, "_stop", NULL);
}

int main(size_t argc, char **argv)
{
    (void)argc;
    (void)argv;

    Object *ctx = new_ctx();
    cn_value temp_val;

    if (!ctx) {
        PROPAGATE_ERR();
        return (1);
    }

    global_ctx = ctx;

    signal(SIGINT, &sigint_handler);
    signal(SIGTERM, &sigint_handler);

    if (load_submodules(ctx)) {
        PROPAGATE_ERR();
        DELOC(ctx);
        return (1);
    }

    temp_val = call_method(ctx, "_init", NULL);

    if (temp_val.type == CN_TYPE_NULL || temp_val.as.i == VALUE_ERR.as.i) {
        PROPAGATE_ERR();
        DELOC(ctx);
        return (1);
    }

    #if defined(_ENGINE_HAS_ASSETS) && (_ENGINE_HAS_ASSETS == 1)
        if (load_assets_handler(ctx)) {
            PROPAGATE_ERR();
            DELOC(ctx);
            return (1);
        }

        if (load_entry_scene(ctx)) {
            PROPAGATE_ERR();
            DELOC(ctx);
            return (1);
        }
    #endif

    call_method(ctx, "_run", NULL);
    call_method(ctx, "_stop", NULL);
    DELOC(ctx);

    return (0);
}
