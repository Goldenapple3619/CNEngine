#include "libcnassets.h"
#include "libcncore.h"

static cn_value _register_fmt(Object *__this, void **args)
{
    if (!args || !args[0]) {
        RAISE(ERR_INVALID_POINTER, "can't register fmt with no path to a fmt library.");
        return (VALUE_ERR);
    }

    if (!has_attr(__this, "assets_fmts")) {
        RAISE(ERR_NOT_COMPATIBLE, "base is not inited properly, assets_fmts not found.");
        return (VALUE_ERR);
    }

    char *dl_path = args[0];
    Object *assets_fmts = get_attr(__this, "assets_fmts")->as.ptr;

    if (!assets_fmts) {
        RAISE(ERR_INVALID_POINTER, "base is not inited properly, assets_fmts is empty.");
        return (VALUE_ERR);
    }

    struct asset_loader_s *loader = new_asset_loader();

    if (!loader) {
        PROPAGATE_ERR();
        return (VALUE_ERR);
    }

    if (asset_loader_init(loader, dl_path)) {
        PROPAGATE_ERR();
        delete_asset_loader(loader);
        return (VALUE_ERR);
    }
    
    if (call_method(assets_fmts, "push", PACK_ARG(loader, NULL)).as.i == VALUE_ERR.as.i) {
        PROPAGATE_ERR();
        delete_asset_loader(loader);
        return (VALUE_ERR);
    }

    return (VALUE_OK);
}

static cn_value _find_asset_type(Object *__this, void **args)
{
    if (!args || !args[0]) {
        RAISE(ERR_INVALID_POINTER, "can't find asset type with no name.");
        return (VALUE_NULL);
    }

    if (!has_attr(__this, "assets_fmts")) {
        RAISE(ERR_NOT_COMPATIBLE, "base is not inited properly, assets_fmts not found.");
        return (VALUE_NULL);
    }

    Object *assets_fmts = get_attr(__this, "assets_fmts")->as.ptr;
    struct asset_loader_s *loader;

    if (!assets_fmts) {
        RAISE(ERR_INVALID_POINTER, "base is not inited properly, assets_fmts is empty.");
        return (VALUE_NULL);
    }
    
    for (struct list_iterator_s it = list_get_iterator(assets_fmts); !list_iterator_isend(&it); list_iterator_next(&it)) {
        if (list_iterator_value_isnull(&it))
            continue;
        
        loader = it.val.as.ptr;

        if (loader->registry && loader->registry->asset_type == *(uint16_t *)args[0])
            return ((cn_value){.type = CN_TYPE_GENERIC_UNIQ_PTR, .as.ptr = loader->registry});
    }

    return (VALUE_NULL);
}

static cn_value _find_asset_name(Object *__this, void **args)
{
    if (!args || !args[0]) {
        RAISE(ERR_INVALID_POINTER, "can't find asset name with no name.");
        return (VALUE_NULL);
    }

    if (!has_attr(__this, "assets_fmts")) {
        RAISE(ERR_NOT_COMPATIBLE, "base is not inited properly, assets_fmts not found.");
        return (VALUE_NULL);
    }

    Object *assets_fmts = get_attr(__this, "assets_fmts")->as.ptr;
    struct asset_loader_s *loader;

    if (!assets_fmts) {
        RAISE(ERR_INVALID_POINTER, "base is not inited properly, assets_fmts is empty.");
        return (VALUE_NULL);
    }
    
    for (struct list_iterator_s it = list_get_iterator(assets_fmts); !list_iterator_isend(&it); list_iterator_next(&it)) {
        if (list_iterator_value_isnull(&it))
            continue;
        
        loader = it.val.as.ptr;

        if (loader->registry && loader->registry->name && !strcmp(loader->registry->name, (const char *)args[0]))
            return ((cn_value){.type = CN_TYPE_GENERIC_UNIQ_PTR, .as.ptr = loader->registry});
    }

    return (VALUE_NULL);
}

static cn_value _find_section_type(Object *__this, void **args)
{
    if (!args || !args[0]) {
        RAISE(ERR_INVALID_POINTER, "can't find section type with no type.");
        return (VALUE_NULL);
    }

    if (!has_attr(__this, "assets_fmts")) {
        RAISE(ERR_NOT_COMPATIBLE, "base is not inited properly, assets_fmts not found.");
        return (VALUE_NULL);
    }

    Object *assets_fmts = get_attr(__this, "assets_fmts")->as.ptr;
    struct asset_loader_s *loader;

    if (!assets_fmts) {
        RAISE(ERR_INVALID_POINTER, "base is not inited properly, assets_fmts is empty.");
        return (VALUE_NULL);
    }
    
    for (struct list_iterator_s it = list_get_iterator(assets_fmts); !list_iterator_isend(&it); list_iterator_next(&it)) {
        if (list_iterator_value_isnull(&it))
            continue;
        
        loader = it.val.as.ptr;

        if (!loader->registry)
            continue;

        for (size_t i = 0; i < loader->registry->registered_sections.size; ++i) {
            if (loader->registry->registered_sections.content[i] && ((struct section_registry *)loader->registry->registered_sections.content[i])->section_type == *(uint16_t *)args[0])
                return ((cn_value){.type = CN_TYPE_GENERIC_UNIQ_PTR, .as.ptr = loader->registry});
        }
    }

    return (VALUE_NULL);
}

static cn_value _find_section_name(Object *__this, void **args)
{
    if (!args || !args[0]) {
        RAISE(ERR_INVALID_POINTER, "can't find section name with no name.");
        return (VALUE_NULL);
    }

    if (!has_attr(__this, "assets_fmts")) {
        RAISE(ERR_NOT_COMPATIBLE, "base is not inited properly, assets_fmts not found.");
        return (VALUE_NULL);
    }

    Object *assets_fmts = get_attr(__this, "assets_fmts")->as.ptr;
    struct asset_loader_s *loader;

    if (!assets_fmts) {
        RAISE(ERR_INVALID_POINTER, "base is not inited properly, assets_fmts is empty.");
        return (VALUE_NULL);
    }
    
    for (struct list_iterator_s it = list_get_iterator(assets_fmts); !list_iterator_isend(&it); list_iterator_next(&it)) {
        if (list_iterator_value_isnull(&it))
            continue;
        
        loader = it.val.as.ptr;

        if (!loader->registry)
            continue;

        for (size_t i = 0; i < loader->registry->registered_sections.size; ++i) {
            if (loader->registry->registered_sections.content[i] && ((struct section_registry *)loader->registry->registered_sections.content[i])->name && !strcmp(((struct section_registry *)loader->registry->registered_sections.content[i])->name, (const char *)args[0]))
                return ((cn_value){.type = CN_TYPE_GENERIC_UNIQ_PTR, .as.ptr = loader->registry});
        }
    }

    return (VALUE_NULL);
}

static cn_value _init(Object *__this, void **args)
{
    (void)__this;

    PREP_INIT()

    if (!args || !(args[0])) {
        RAISE(ERR_INVALID_POINTER, "can't init asset submodule without ctx argument.")
        return (VALUE_ERR);
    }
    
    Object *ctx = (Object *)(args[0]);

    INIT_OBJECT_STATIC(ctx, new_list((expr_free)&delete_asset_loader), NULL, "assets_fmts");
    INIT_METHOD(ctx, "register_fmt", &_register_fmt);
    INIT_METHOD(ctx, "find_asset_by_type", &_find_asset_type);
    INIT_METHOD(ctx, "find_asset_by_name", &_find_asset_name);
    INIT_METHOD(ctx, "find_section_by_type", &_find_section_type);
    INIT_METHOD(ctx, "find_section_by_name", &_find_section_name);

    return (VALUE_OK);
}

static cn_value _del(Object *__this, void **args)
{
    (void)__this;

    if (!args || !(args[0]))
        return (VALUE_ERR);

    Object *ctx = (Object *)(args[0]);

    (void)ctx;

    return (VALUE_OK);
}

CN_API Object *new_asset_submodule(void)
{
    Object *obj = new_object();

    if (!obj) {
        PROPAGATE_ERR();
        return (NULL);
    }

    SET_PARENT_CLASS_BUILD_STATIC(obj, create_default_object());

    CREATE_METHOD_CLASS_BUILD(obj, "_init", &_init);
    CREATE_METHOD_CLASS_BUILD(obj, "_del", &_del);
    return (obj);
}
