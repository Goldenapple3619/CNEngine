#include "libcncore.h"

static cn_value _init(Object *__this, void **args)
{
    PREP_INIT()

    (void)args;

    void (*_delobj_cb)(void *) = get_attr(__this, "_delobj_cb")->as.ptr;

    __temp_alloc = (void *)new_generic_map(); \
    if (!__temp_alloc) \
        return (VALUE_ERR); \
    if (!set_attr(__this, "_map", CN_TYPE_GENERIC_UNIQ_PTR, (cnany)__temp_alloc)) { \
        (void)delete_generic_map(__temp_alloc, _delobj_cb); \
        return (VALUE_ERR); \
    }

    return (VALUE_OK);
}

static cn_value _push(Object *__this, void **args)
{
    if (!args || !args[0] || !args[1])
        return (VALUE_ERR);

    struct generic_map_s *gen_map = get_attr(__this, "_map")->as.ptr;
    void (*_delobj_cb)(void *) = get_attr(__this, "_delobj_cb")->as.ptr;
    
    if (add_generic_map(gen_map, args[0], *(size_t *)(args[1]), _delobj_cb))
        return (VALUE_ERR);

    return (VALUE_OK);
}

static cn_value _at(Object *__this, void **args)
{
    if (!args || !args[0])
        return (null_value);
    
    struct generic_map_s *gen_map = get_attr(__this, "_map")->as.ptr;
    void (*_delobj_cb)(void *) = get_attr(__this, "_delobj_cb")->as.ptr;
    void (*_fetchobj_cb)(void *) = get_attr(__this, "_fetchobj_cb")->as.ptr;
    const char *entry = args[0];

    if (gen_map->size <= index)
        return (null_value);

    return ((cn_value){.type=CN_TYPE_GENERIC_UNIQ_PTR, .as.ptr=get_generic_map(gen_map, entry, _fetchobj_cb, _delobj_cb)});
}

static cn_value _at_value(Object *__this, void **args)
{
    if (!args || !args[0])
        return (null_value);
    
    struct generic_map_s *gen_map = get_attr(__this, "_map")->as.ptr;
    size_t index = *(size_t *)args[0];

    if (gen_map->size <= index)
        return (null_value);

    return ((cn_value){.type=CN_TYPE_GENERIC_UNIQ_PTR, .as.ptr=gen_map->content[index]});
}

static cn_value _at_key(Object *__this, void **args)
{
    if (!args || !args[0])
        return (null_value);
    
    struct generic_map_s *gen_map = get_attr(__this, "_map")->as.ptr;
    size_t index = *(size_t *)args[0];

    if (gen_map->size <= index)
        return (null_value);

    return ((cn_value){.type=CN_TYPE_INT, .as.i=gen_map->keys[index]});
}

static cn_value _len(Object *__this, void **args)
{
    (void)args;

    struct generic_map_s *gen_map = get_attr(__this, "_map")->as.ptr;

    return ((cn_value){.type=CN_TYPE_INT, .as.i=gen_map->size});
}

static cn_value _remove(Object *__this, void **args)
{
    if (!args || !(args[0]))
        return (VALUE_ERR);
    
    struct generic_map_s *gen_map = get_attr(__this, "_map")->as.ptr;
    void (*_delobj_cb)(void *) = get_attr(__this, "_delobj_cb")->as.ptr;
    
    (void)remove_generic_map(gen_map, args[0], _delobj_cb);
    return (VALUE_OK);
}

static cn_value _del(Object *__this, void **args)
{
    (void)args;

    PREP_DEL()

    void (*_delobj_cb)(void *) = get_attr(__this, "_delobj_cb")->as.ptr;

    __temp_alloc = get_attr(__this, "_map");
    if (__temp_alloc && __temp_alloc->as.ptr)
        delete_generic_map(__temp_alloc->as.ptr, _delobj_cb);

    return (null_value);
}

CN_API struct list_iterator_s atlas_get_iterator(Object *__atlas, cnbool get_value_instead_of_key)
{
    if (!__atlas)
        return ((struct list_iterator_s){0});

    cn_method __at = NULL;

    if (get_value_instead_of_key)
        __at = get_method(__atlas, "at_value");
    else
        __at = get_method(__atlas, "at_key");
    size_t __len = call_method(__atlas, "len", NULL).as.i;

    if (!__at)
        return ((struct list_iterator_s){0});

    return ((struct list_iterator_s){.get_element = __at, .size = __len, .pos = 0, ._obj = __atlas, .val = __at(__atlas, (cnany []){&(size_t){0}})});
}

CN_API void atlas_iterator_next(struct list_iterator_s *iterator)
{
    if (!iterator || !iterator->get_element || iterator->pos >= iterator->size) {
        iterator->val = null_value;
        return;
    }

    ++iterator->pos;
    iterator->val = (iterator->get_element(iterator->_obj, (cnany []){&iterator->pos}));
}

CN_API cnbool atlas_iterator_value_isnull(const struct list_iterator_s *iterator)
{
    if (!iterator || iterator->val.type == CN_TYPE_NULL) {
        return (true);
    }
    return (false);
}

CN_API cnbool atlas_iterator_isend(const struct list_iterator_s *iterator)
{
    if (!iterator || iterator->pos >= iterator->size)
        return (true);
    return (false);
}

CN_API Object *new_atlas(void *(*_fetch_default)(const char *k), void (*_delete_obj)(void *))
{
    Object *obj = new_object();

    if (!obj)
        return (NULL);

    SET_PARENT_CLASS_BUILD(obj, create_default_object());

    if (!set_attr(obj, "_delobj_cb", CN_TYPE_GENERIC_UNIQ_PTR, (cnany)_delete_obj)) {
        (void)delete_object(obj);
        return (NULL);
    }
    if (!set_attr(obj, "_fetchobj_cb", CN_TYPE_GENERIC_UNIQ_PTR, (cnany)_fetch_default)) {
        (void)delete_object(obj);
        return (NULL);
    }

    CREATE_METHOD_CLASS_BUILD(obj, "_init", &_init);
    CREATE_METHOD_CLASS_BUILD(obj, "push", &_push);
    CREATE_METHOD_CLASS_BUILD(obj, "len", &_len);
    CREATE_METHOD_CLASS_BUILD(obj, "remove", &_remove);
    CREATE_METHOD_CLASS_BUILD(obj, "at", &_at);
    CREATE_METHOD_CLASS_BUILD(obj, "at_value", &_at_value);
    CREATE_METHOD_CLASS_BUILD(obj, "at_key", &_at_key);
    CREATE_METHOD_CLASS_BUILD(obj, "_del", &_del);

    return (obj);
}

