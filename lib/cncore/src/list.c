#include "libcncore.h"

static cn_value _init(Object *__this, void **args)
{
    PREP_INIT()

    INIT_CUSTOM_ALLOCATION(__this, new_object_vector(), delete_object_vector, "_vec");

    if (args)
        (void)call_method(__this, "push", args);

    return (VALUE_OK);
}

static cn_value _push(Object *__this, void **args)
{
    if (!args)
        return (VALUE_ERR);

    ObjectVector *vec = get_attr(__this, "_vec")->as.ptr;

    for (size_t i = 0; args[i]; ++i) {
        if (insert_object_vector(vec, (Object *)args[i]))
            return (VALUE_ERR);
    }
    return (VALUE_OK);
}

static cn_value _at(Object *__this, void **args)
{
    if (!args || !args[0])
        return (null_value);
    
    ObjectVector *vec = get_attr(__this, "_vec")->as.ptr;
    size_t index = *(size_t *)args[0];

    if (vec->size <= index)
        return (null_value);

    return ((cn_value){.type=CN_TYPE_OBJECT, .as.ptr=vec->objects[index]});
}

static cn_value _len(Object *__this, void **args)
{
    (void)args;

    ObjectVector *vec = get_attr(__this, "_vec")->as.ptr;

    return ((cn_value){.type=CN_TYPE_INT, .as.i=vec->size});
}

static cn_value _remove(Object *__this, void **args)
{
    if (!args || !(args[0]))
        return (VALUE_ERR);
    
    ObjectVector *vec = get_attr(__this, "_vec")->as.ptr;
    
    (void)remove_object_ordered_vector(vec, *((size_t *)args[0]));
    return (VALUE_OK);
}

static cn_value _del(Object *__this, void **args)
{
    (void)args;

    PREP_DEL()

    DEL_CUSTOM_ALLOCAION(__this, delete_object_vector, "_vec");

    return (null_value);
}

CN_API struct list_iterator_s list_get_iterator(Object *__list)
{
    if (!__list)
        return ((struct list_iterator_s){0});

    cn_method __at = get_method(__list, "at");
    size_t __len = call_method(__list, "len", NULL).as.i;

    if (!__at)
        return ((struct list_iterator_s){0});

    return ((struct list_iterator_s){.get_element = __at, .size = __len, .pos = 0, ._obj = __list, .val = __at(__list, (cnany []){&(size_t){0}})});
}

CN_API void list_iterator_next(struct list_iterator_s *iterator)
{
    if (!iterator || !iterator->get_element || iterator->pos >= iterator->size) {
        iterator->val = null_value;
        return;
    }

    ++iterator->pos;
    iterator->val = (iterator->get_element(iterator->_obj, (cnany []){&iterator->pos}));
}

CN_API cnbool list_iterator_value_isnull(const struct list_iterator_s *iterator)
{
    if (!iterator || iterator->val.type == CN_TYPE_NULL) {
        return (true);
    }
    return (false);
}

CN_API cnbool list_iterator_isend(const struct list_iterator_s *iterator)
{
    if (!iterator || iterator->pos >= iterator->size)
        return (true);
    return (false);
}

CN_API Object *new_list()
{
    Object *obj = new_object();

    if (!obj)
        return (NULL);

    SET_PARENT_CLASS_BUILD(obj, create_default_object());
    CREATE_METHOD_CLASS_BUILD(obj, "_init", &_init);
    CREATE_METHOD_CLASS_BUILD(obj, "push", &_push);
    CREATE_METHOD_CLASS_BUILD(obj, "len", &_len);
    CREATE_METHOD_CLASS_BUILD(obj, "remove", &_remove);
    CREATE_METHOD_CLASS_BUILD(obj, "at", &_at);
    CREATE_METHOD_CLASS_BUILD(obj, "_del", &_del);

    return (obj);
}

