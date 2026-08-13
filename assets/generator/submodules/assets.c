#include "../engine_main.h"
#include "libcnassets.h"

static cn_value _search_symbol_container(Object *__this, void **args)
{
    if (!args || !args[0]) {
        RAISE(ERR_INVALID_POINTER, "can't search unspecified data object.");
        return (null_value);
    }

    if (!args[1]) {
        RAISE(ERR_INVALID_POINTER, "can't search unspecified symbol.");
        return (null_value);
    }

    CNAssetReader *reader = new_object_file_reader();

    if (!reader) {
        PROPAGATE_ERR();
        return (null_value);
    }
    if (init_object_file_reader(reader, args[0])) {
        PROPAGATE_ERR();
        delete_object_file_reader(reader);
        return (null_value);
    }
    if (object_file_reader_read_header(reader)) {
        PROPAGATE_ERR();
        delete_object_file_reader(reader);
        return (null_value);
    }
    if (object_file_reader_read_section_header(reader)) {
        PROPAGATE_ERR();
        delete_object_file_reader(reader);
        return (null_value);
    }

    printf("==== %s\n", object_file_reader_get_string(reader, reader->header.name));

    for (size_t i = 0; i < reader->section_header.section_count; ++i) {
        printf("%s\n", object_file_reader_get_string(reader, reader->section_header.entries[i].section_name));
    }

    delete_object_file_reader(reader);
    return (null_value);
}

static cn_value _apply_map(Object *__this, void **args)
{
    if (!args || !args[0]) {
        RAISE(ERR_INVALID_POINTER, "can't load unspecified map.");
        return (VALUE_ERR);
    }

    return (VALUE_OK);
}

static cn_value _load_ressource(Object *__this, void **args)
{
    if (!args || !args[0]) {
        RAISE(ERR_INVALID_POINTER, "can't load unspecified ressource.");
        return (VALUE_ERR);
    }

    return (VALUE_OK);
}

uint8_t register_engine_asset_api(Object *ctx)
{
    SHR_INIT_METHOD(ctx, "apply_map", &_apply_map, 1);
    SHR_INIT_METHOD(ctx, "load_ressource", &_load_ressource, 1);
    SHR_INIT_METHOD(ctx, "search_symbol", &_search_symbol_container, 1);

    call_method(ctx, "search_symbol", PACK_ARG("../game/pack000.cpk", "raw.rwa.dirt.png"));
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
