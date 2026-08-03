#include "../engine_main.h"

static cn_value _search_symbol_container(Object *__this, void **args)
{
    if (!args || !args[0]) {
        RAISE(ERR_INVALID_POINTER, "can't search unspecified symbol.");
        return (VALUE_ERR);
    }
}

static cn_value _load_map(Object *__this, void **args)
{
    if (!args || !args[0]) {
        RAISE(ERR_INVALID_POINTER, "can't load unspecified map.");
        return (VALUE_ERR);
    }

    return (VALUE_OK);
}

uint8_t register_engine_asset_api(Object *ctx)
{
    SHR_INIT_METHOD(ctx, "load_map", &_load_map, 1);
}

uint8_t load_assets_handler(Object *ctx)
{
    #ifdef _WIN32
    if (call_method(ctx, "register_fmt", PACK_ARG("./libcnguiobj.dll")).as.i == VALUE_ERR.as.i) {
        PROPAGATE_ERR();
        return (1);
    }

    if (call_method(ctx, "register_fmt", PACK_ARG("./libcnsceneobj.dll")).as.i == VALUE_ERR.as.i) {
        PROPAGATE_ERR();
        return (1);
    }

    if (call_method(ctx, "register_fmt", PACK_ARG("./libcnobjectobj.dll")).as.i == VALUE_ERR.as.i) {
        PROPAGATE_ERR();
        return (1);
    }
    #else
    if (call_method(ctx, "register_fmt", PACK_ARG("./libcnguiobj.so")).as.i == VALUE_ERR.as.i) {
        PROPAGATE_ERR();
        return (1);
    }

    if (call_method(ctx, "register_fmt", PACK_ARG("./libcnsceneobj.so")).as.i == VALUE_ERR.as.i) {
        PROPAGATE_ERR();
        return (1);
    }

    if (call_method(ctx, "register_fmt", PACK_ARG("./libcnobjectobj.so")).as.i == VALUE_ERR.as.i) {
        PROPAGATE_ERR();
        return (1);
    }
    #endif

    return (0);
}

uint8_t load_entry_scene(Object *ctx)
{
    #if defined(_ENTRY_SCENE)
        if (!RET_OK(call_method(ctx, "load_map", PACK_ARG(_ENGINE_ENTRY_SCENE)))) {
            PROPAGATE_ERR();
            return (1);
        }
    #endif

    RAISE(WAR_IMPORTANT, "no entry scene set to be loaded.");

    return (0);
}
