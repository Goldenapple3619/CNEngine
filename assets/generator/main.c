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

    if (!ctx)
        return (1);

    global_ctx = ctx;

    signal(SIGINT, &sigint_handler);

    if (load_submodules(ctx)) {
        delete_object(ctx);
        return (1);
    }

    temp_val = call_method(ctx, "_init", NULL);

    if (temp_val.type == CN_TYPE_NULL || temp_val.as.i == VALUE_ERR.as.i) {

    }

    call_method(ctx, "_run", NULL);
    call_method(ctx, "_stop", NULL);
    DELOC(ctx);

    return (0);
}
