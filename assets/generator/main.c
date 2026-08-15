#define SDL_MAIN_HANDLED

#include "engine_main.h"

static Object *global_ctx = NULL;

static void sigint_handler(int signum)
{
    (void)signum;

    if (global_ctx && has_method(global_ctx, "_stop"))
        call_method(global_ctx, "_stop", NULL);
}

static void report_errors(void)
{
    const ErrorContext *temp;

    while (has_error()) {
        temp = get_error();

        (void)print_error(temp, NULL);
    }
}

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    Object *ctx = new_ctx();
    cn_value temp_val;

    if (!ctx) {
        PROPAGATE_ERR();
        (void)run_gc();
        report_errors();
        return (1);
    }

    global_ctx = ctx;

    signal(SIGINT, &sigint_handler);
    signal(SIGTERM, &sigint_handler);

    if (load_submodules(ctx)) {
        PROPAGATE_ERR();
        DELOC(ctx);
        report_errors();
        return (1);
    }

    temp_val = call_method(ctx, "_init", NULL);

    if (temp_val.type == CN_TYPE_NULL || temp_val.as.i == VALUE_ERR.as.i) {
        PROPAGATE_ERR();
        DELOC(ctx);
        report_errors();
        return (1);
    }

    #if defined(_HAS_ASSETS) && (_HAS_ASSETS == 1)
        if (load_assets_handler(ctx)) {
            PROPAGATE_ERR();
            DELOC(ctx);
            report_errors();
            return (1);
        }

        if (register_engine_asset_api(ctx)) {
            PROPAGATE_ERR();
            DELOC(ctx);
            report_errors();
            return (1);
        }

        if (load_entry_scene(ctx)) {
            PROPAGATE_ERR();
            #if defined(_HAS_ASSETS) && (_HAS_ASSETS == 1)
                unregister_engine_asset_api(ctx);
            #endif
            DELOC(ctx);
            report_errors();
            return (1);
        }
    #endif

    #if defined(_HAS_GRAPHICS) && (_HAS_GRAPHICS == 1)
        if (add_main_interface(ctx)) {
            PROPAGATE_ERR();
            #if defined(_HAS_ASSETS) && (_HAS_ASSETS == 1)
                unregister_engine_asset_api(ctx);
            #endif
            DELOC(ctx);
            report_errors();
            return (1);
        }
    #endif

    call_method(ctx, "_run", NULL);
    call_method(ctx, "_stop", NULL);

    #if defined(_HAS_ASSETS) && (_HAS_ASSETS == 1)
        unregister_engine_asset_api(ctx);
    #endif

    DELOC(ctx);

    report_errors();

    return (0);
}
