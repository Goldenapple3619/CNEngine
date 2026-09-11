#include "../../engine_main.h"
#include "libcnassets.h"
#include "cnobjectobj.h"

static cn_value _search_symbol_container(Object *__this, void **args)
{
    if (!args || !args[0]) {
        RAISE(ERR_INVALID_POINTER, "can't search unspecified data object.");
        return (VALUE_NULL);
    }

    if (!args[1]) {
        RAISE(ERR_INVALID_POINTER, "can't search unspecified symbol.");
        return (VALUE_NULL);
    }

    cn_value *potential_readers = get_attr(__this, "active_assets_readers");
    CNAssetReader *reader;
    const char *cmp;
    struct section_blk *temp_blk;

    if (!potential_readers) {
        PROPAGATE_ERR();
        return (VALUE_NULL);
    }

    if (call_method(potential_readers->as.ptr, "has", PACK_ARG(args[0])).as.b) {
        reader = call_method(potential_readers->as.ptr, "at", PACK_ARG(args[0])).as.ptr;
    } else {
        reader = new_object_file_reader();

        if (!reader) {
            PROPAGATE_ERR();
            return (VALUE_NULL);
        }

        if (init_object_file_reader(reader, args[0])) {
            PROPAGATE_ERR();
            delete_object_file_reader(reader);
            return (VALUE_NULL);
        }

        if (object_file_reader_read_header(reader)) {
            PROPAGATE_ERR();
            delete_object_file_reader(reader);
            return (VALUE_NULL);
        }

        if (object_file_reader_read_section_header(reader)) {
            PROPAGATE_ERR();
            delete_object_file_reader(reader);
            return (VALUE_NULL);
        }

        if (call_method(potential_readers->as.ptr, "push", PACK_ARG(reader, args[0])).as.i == VALUE_ERR.as.i) {
            PROPAGATE_ERR();
            delete_object_file_reader(reader);
            return (VALUE_NULL);
        }
    }

    printf("==== %s\n", object_file_reader_get_string(reader, reader->header.name));

    for (size_t i = 0; i < reader->section_header.section_count; ++i) {
        cmp = object_file_reader_get_string(reader, reader->section_header.entries[i].section_name);

        if (!strcmp(args[1], cmp)) {
            printf("found: %s\n", cmp);

            temp_blk = malloc(sizeof(struct section_blk));

            if (!temp_blk) {
                RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new section_blk.");
                return (VALUE_NULL);
            }

            if (object_file_reader_get_section(reader, temp_blk, i)) {
                PROPAGATE_ERR();
                (void)free(temp_blk);
                return (VALUE_NULL);
            }
            return ((cn_value){.type=CN_TYPE_GENERIC_UNIQ_PTR, .as.ptr=temp_blk});
        }
    }

    return (VALUE_NULL);
}

static cn_value _get_all_asset_pack(Object *__this, void **args)
{
    if (!args || !args[0]) {
        RAISE(ERR_INVALID_POINTER, "can't get all asset pack in no path.");
        return (VALUE_NULL);
    }

    Object *list = build_object(new_list((expr_free)&free), NULL);
    String temp = {.c_str = NULL, .size = 0};

    if (!list) {
        PROPAGATE_ERR();
        return (VALUE_NULL);
    }

    if (str_override_cp(&temp, args[0])) {
        PROPAGATE_ERR();
        collect_object(list);
        return (VALUE_NULL);
    }

    if (str_rcadd_cp(&temp, "/")) {
        PROPAGATE_ERR();
        empty_str(&temp);
        collect_object(list);
        return (VALUE_NULL);
    }

    if (str_rcadd_cp(&temp, "pack000.cpk")) {
        PROPAGATE_ERR();
        empty_str(&temp);
        collect_object(list);
        return (VALUE_NULL);
    }

    if (call_method(list, "push", PACK_ARG(strdup(temp.c_str), NULL)).as.i == VALUE_ERR.as.i) {
        PROPAGATE_ERR();
        empty_str(&temp);
        collect_object(list);
        return (VALUE_NULL);
    }

    empty_str(&temp);
    return ((cn_value){.as.ptr=share_object(list), .type=CN_TYPE_OBJECT});
}

static cn_value _get_all_maps(Object *__this, void **args)
{
    if (!args || !args[0]) {
        RAISE(ERR_INVALID_POINTER, "can't get all maps in no path.");
        return (VALUE_NULL);
    }

    return (VALUE_NULL);
}

static cn_value _apply_scene(Object *__this, void **args)
{
    if (!args || !args[0]) {
        RAISE(ERR_INVALID_POINTER, "can't apply unspecified scene.");
        return (VALUE_ERR);
    }

    return (VALUE_OK);
}

static cn_value _load_scene(Object *__this, void **args)
{
    if (!args || !args[0]) {
        RAISE(ERR_INVALID_POINTER, "can't load unspecified scene.");
        return (VALUE_NULL);
    }

    return (VALUE_NULL);
}

static cn_value _load_symbol(Object *__this, void **args)
{
    if (!args || !args[0]) {
        RAISE(ERR_INVALID_POINTER, "can't load unspecified symbol.");
        return (VALUE_NULL);
    }

    char *temp = strdup(args[0]);
    cn_value temp_value = VALUE_NULL;

    if (!temp) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new symbol name.");
        return (VALUE_NULL);
    }

    char *first_dot = strstr(temp, ".");

    if (!first_dot) {
        RAISE(ERR_INVALID_TYPE, "can't parse symbol properly.");
        (void)free(temp);
        return (VALUE_NULL);
    }

    *first_dot = 0;

    if (!strcmp(temp, "raw")) {
        temp_value = (call_method(__this, "load_ressource", PACK_ARG(first_dot + 1)));
    } else if (!strcmp(temp, "scene")) {
        temp_value = (call_method(__this, "load_scene", PACK_ARG(first_dot + 1)));
    } else if (!strcmp(temp, "object")) {
        temp_value = (call_method(__this, "load_object", PACK_ARG(first_dot + 1)));
    } else if (!strcmp(temp, "symbol")) {
        temp_value = (call_method(__this, "load_function", PACK_ARG(first_dot + 1)));
    } else if (!strcmp(temp, "gui")) {
        temp_value = (call_method(__this, "load_gui", PACK_ARG(first_dot + 1)));
    } else {
        RAISE_FMT(ERR_INVALID_TYPE, "can't load '%s'.", (const char *)args[0]);
        (void)free(temp);
        return (temp_value);
    }

    if (RET_NULL(temp_value) || VAL_EMPTY(temp_value)) {
        PROPAGATE_ERR();
        return (VALUE_NULL);
    }

    (void)free(temp);
    return (temp_value);
}

uint8_t register_engine_asset_api(Object *ctx)
{
    PREP_INIT();

    SHR_OBJECT_STATIC(ctx, new_atlas(NULL, (expr_free)&delete_object_file_reader), NULL, "active_assets_readers", 1);

    SHR_INIT_METHOD(ctx, "apply_scene", &_apply_scene, 1);
    SHR_INIT_METHOD(ctx, "load_symbol", &_load_symbol, 1);
    SHR_INIT_METHOD(ctx, "load_ressource", &_load_ressource, 1);
    SHR_INIT_METHOD(ctx, "search_symbol_container", &_search_symbol_container, 1);
    SHR_INIT_METHOD(ctx, "get_all_asset_pack", &_get_all_asset_pack, 1);

    cn_value temp = call_method(ctx, "load_symbol", PACK_ARG("raw.assets/dirt.png"));

    if (RET_NULL(temp)) {
        PROPAGATE_ERR();
        return (1);
    }

    (void)free(temp.as.ptr);
    return (0);
}

void unregister_engine_asset_api(Object *ctx)
{
    call_method(get_attr(ctx, "active_assets_readers")->as.ptr, "empty", NULL);
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
    #elif defined(__APPLE__)
    if (call_method(ctx, "register_fmt", PACK_ARG("./libcnguiobj.dylib")).as.i == VALUE_ERR.as.i) {
        PROPAGATE_ERR();
        return (1);
    }

    if (call_method(ctx, "register_fmt", PACK_ARG("./libcnsceneobj.dylib")).as.i == VALUE_ERR.as.i) {
        PROPAGATE_ERR();
        return (1);
    }

    if (call_method(ctx, "register_fmt", PACK_ARG("./libcnobjectobj.dylib")).as.i == VALUE_ERR.as.i) {
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
        #ifndef STRINGIFY
            #define STRINGIFY(x) #x
            #define TOSTRING(x) STRINGIFY(x)
        #endif
        if (!RET_OK(call_method(ctx, "apply_scene", PACK_ARG(TOSTRING(_ENTRY_SCENE))))) {
            PROPAGATE_ERR();
            return (1);
        }
    #else
        RAISE(WAR_IMPORTANT, "no entry scene set to be loaded.");
    #endif

    return (0);
}
