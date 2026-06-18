#include "libcncore.h"

static cn_value _init(Object *__this, void **args)
{
    PREP_INIT()

    void (*_delobj_cb)(void *) = get_attr(__this, "_delobj_cb")->as.ptr;

    __temp_alloc = (void *)new_generic_vector();
    if (!__temp_alloc) {
        PROPAGATE_ERR();
        return (VALUE_ERR);
    }
    if (!set_attr(__this, "_vec", CN_TYPE_GENERIC_UNIQ_PTR, (cnany)__temp_alloc)) {
        PROPAGATE_ERR();
        (void)_delobj_cb(__temp_alloc);
        return (VALUE_ERR);
    }

    if (args) {
        if (call_method(__this, "push", args).as.i == VALUE_ERR.as.i) {
            PROPAGATE_ERR();
            return (VALUE_ERR);
        }
    }

    return (VALUE_OK);
}

static cn_value _push(Object *__this, void **args)
{
    if (!args) {
        RAISE(ERR_INVALID_POINTER, "can't push with no index.");
        return (VALUE_ERR);
    }

    struct generic_vector_s *vec = get_attr(__this, "_vec")->as.ptr;

    for (size_t i = 0; args[i]; ++i) {
        if (insert_generic_vector(vec, args[i])) {
            PROPAGATE_ERR();
            return (VALUE_ERR);
        }
    }
    return (VALUE_OK);
}

static cn_value _at(Object *__this, void **args)
{
    if (!args || !args[0]) {
        RAISE(ERR_INVALID_POINTER, "can't get with no index.");
        return (null_value);
    }
    
    struct generic_vector_s *vec = get_attr(__this, "_vec")->as.ptr;
    size_t index = *(size_t *)args[0];

    if (vec->size <= index) {
        RAISE(ERR_OUT_OF_BOUND, "can't get at invalid index.");
        return (null_value);
    }

    return ((cn_value){.type=CN_TYPE_GENERIC_UNIQ_PTR, .as.ptr=vec->content[index]});
}

static cn_value _len(Object *__this, void **args)
{
    (void)args;

    struct generic_vector_s *vec = get_attr(__this, "_vec")->as.ptr;

    return ((cn_value){.type=CN_TYPE_INT, .as.i=vec->size});
}

static cn_value _remove(Object *__this, void **args)
{
    if (!args || !(args[0])) {
        RAISE(ERR_INVALID_POINTER, "can't remove with no index.");
        return (VALUE_ERR);
    }
    
    struct generic_vector_s *vec = get_attr(__this, "_vec")->as.ptr;
    void (*_delobj_cb)(void *) = get_attr(__this, "_delobj_cb")->as.ptr;
    
    (void)remove_generic_ordered_vector(vec, *((size_t *)args[0]), _delobj_cb);
    return (VALUE_OK);
}

static cn_value _del(Object *__this, void **args)
{
    (void)args;

    PREP_DEL()

    void (*_delobj_cb)(void *) = get_attr(__this, "_delobj_cb")->as.ptr;

    __temp_alloc = get_attr(__this, "_vec");
    if (__temp_alloc && __temp_alloc->as.ptr) {
        delete_generic_vector(__temp_alloc->as.ptr, _delobj_cb);
        __temp_alloc->as.ptr = NULL;
    }

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

    return ((struct list_iterator_s){.get_element = __at, .size = __len, .pos = 0, ._obj = __list, .val = __len ? __at(__list, PACK_ARG(INLNE_PRIM_T_ARG((size_t)0))) : (cn_value){.type=CN_TYPE_NULL, .as.ptr = NULL}});
}

CN_API void list_iterator_next(struct list_iterator_s *iterator)
{
    if (!iterator || !iterator->get_element) {
        iterator->val = null_value;
        return;
    }

    ++iterator->pos;

    if (iterator->pos >= iterator->size) {
        iterator->val = null_value;
        return;
    }

    iterator->val = (iterator->get_element(iterator->_obj, PACK_ARG(&iterator->pos)));
}

CN_API cnbool list_iterator_value_isnull(const struct list_iterator_s *iterator)
{
    if (!iterator || iterator->val.type == CN_TYPE_NULL)
        return (true);
    return (false);
}

CN_API cnbool list_iterator_isend(const struct list_iterator_s *iterator)
{
    if (!iterator || iterator->pos >= iterator->size)
        return (true);
    return (false);
}

CN_API Object *new_list(void (*_delete_obj)(void *))
{
    Object *obj = new_object();

    if (!obj) {
        PROPAGATE_ERR();
        return (NULL);
    }

    SET_PARENT_CLASS_BUILD_STATIC(obj, create_default_object());

    if (!set_attr(obj, "_delobj_cb", CN_TYPE_GENERIC_UNIQ_PTR, (cnany)_delete_obj)) {
        PROPAGATE_ERR();
        (void)delete_object(obj);
        return (NULL);
    }

    CREATE_METHOD_CLASS_BUILD(obj, "_init", &_init);
    CREATE_METHOD_CLASS_BUILD(obj, "push", &_push);
    CREATE_METHOD_CLASS_BUILD(obj, "len", &_len);
    CREATE_METHOD_CLASS_BUILD(obj, "remove", &_remove);
    CREATE_METHOD_CLASS_BUILD(obj, "at", &_at);
    CREATE_METHOD_CLASS_BUILD(obj, "_del", &_del);

    return (obj);
}

