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

    cn_value *potential_readers = get_attr(__this, "active_assets_readers");
    CNAssetReader *reader;

    if (!potential_readers) {
        PROPAGATE_ERR();
        return (null_value);
    }

    if (call_method(potential_readers->as.ptr, "has", PACK_ARG(args[0])).as.b) {
        reader = call_method(potential_readers->as.ptr, "at", PACK_ARG(args[0])).as.ptr;
    } else {
        reader = new_object_file_reader();

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

        if (call_method(potential_readers->as.ptr, "push", PACK_ARG(reader, args[0])).as.i == VALUE_ERR.as.i) {
            PROPAGATE_ERR();
            delete_object_file_reader(reader);
            return (null_value);
        }
    }

    printf("==== %s\n", object_file_reader_get_string(reader, reader->header.name));

    for (size_t i = 0; i < reader->section_header.section_count; ++i) {
        printf("%s\n", object_file_reader_get_string(reader, reader->section_header.entries[i].section_name));
    }

    RAISE(ERR_OK, "uninplemented.");
    return (null_value);
}

static cn_value _get_all_asset_pack(Object *__this, void **args)
{
    if (!args || !args[0]) {
        RAISE(ERR_INVALID_POINTER, "can't get all asset pack in no path.");
        return (null_value);
    }

    Object *list = build_object(new_list((expr_free)&free), NULL);
    String temp = {.c_str = NULL, .size = 0};

    if (!list) {
        PROPAGATE_ERR();
        return (null_value);
    }

    if (str_override_cp(&temp, args[0])) {
        PROPAGATE_ERR();
        collect_object(list);
        return (null_value);
    }

    if (str_rcadd_cp(&temp, "/")) {
        PROPAGATE_ERR();
        empty_str(&temp);
        collect_object(list);
        return (null_value);
    }

    if (str_rcadd_cp(&temp, "pack000.cpk")) {
        PROPAGATE_ERR();
        empty_str(&temp);
        collect_object(list);
        return (null_value);
    }

    if (call_method(list, "push", PACK_ARG(strdup(temp.c_str), NULL)).as.i == VALUE_ERR.as.i) {
        PROPAGATE_ERR();
        empty_str(&temp);
        collect_object(list);
        return (null_value);
    }

    empty_str(&temp);
    return ((cn_value){.as.ptr=share_object(list), .type=CN_TYPE_OBJECT});
}

static cn_value _get_all_maps(Object *__this, void **args)
{
    if (!args || !args[0]) {
        RAISE(ERR_INVALID_POINTER, "can't get all maps in no path.");
        return (null_value);
    }

    return (null_value);
}

static cn_value _apply_scene(Object *__this, void **args)
{
    if (!args || !args[0]) {
        RAISE(ERR_INVALID_POINTER, "can't apply unspecified scene.");
        return (VALUE_ERR);
    }

    return (VALUE_OK);
}

static cn_value _load_ressource(Object *__this, void **args)
{
    if (!args || !args[0]) {
        RAISE(ERR_INVALID_POINTER, "can't load unspecified ressource.");
        return (null_value);
    }

    String str = {.c_str = NULL, .size = 0};

    if (str_override_cp(&str, args[0])) {
        PROPAGATE_ERR();
        return (null_value);
    }

    if (str_lcadd_cp(&str, "raw.rwa.")) {
        empty_str(&str);
        PROPAGATE_ERR();
        return (null_value);
    }

    cn_value val = call_method(__this, "get_all_asset_pack", PACK_ARG("../game"));
    cn_value container_val;

    if (val.type == CN_TYPE_NULL) {
        empty_str(&str);
        PROPAGATE_ERR();
        return (null_value);
    }

    for (struct list_iterator_s it = list_get_iterator(val.as.ptr); !list_iterator_isend(&it); list_iterator_next(&it)) {
        if (list_iterator_value_isnull(&it))
            continue;

        container_val = call_method(__this, "search_symbol_container", PACK_ARG(it.val.as.str, str.c_str));

        if (container_val.type == CN_TYPE_NULL)
            continue;

        collect_object(val.as.ptr);
        empty_str(&str);

        return (container_val);
    }

    collect_object(val.as.ptr);
    empty_str(&str);
    return (null_value);
}

static cn_value _load_scene(Object *__this, void **args)
{
    if (!args || !args[0]) {
        RAISE(ERR_INVALID_POINTER, "can't load unspecified scene.");
        return (null_value);
    }

    

    return (null_value);
}

static cn_value _load_symbol(Object *__this, void **args)
{
    if (!args || !args[0]) {
        RAISE(ERR_INVALID_POINTER, "can't load unspecified symbol.");
        return (null_value);
    }

    char *temp = strdup(args[0]);
    cn_value temp_value = null_value;

    if (!temp) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new symbol name.");
        return (null_value);
    }

    char *first_dot = strstr(temp, ".");

    if (!first_dot) {
        RAISE(ERR_INVALID_TYPE, "can't parse symbol properly.");
        (void)free(temp);
        return (null_value);
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

    if (temp_value.type == CN_TYPE_NULL) {
        PROPAGATE_ERR();
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

    call_method(ctx, "load_symbol", PACK_ARG("raw.assets/dirt.png"));

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
