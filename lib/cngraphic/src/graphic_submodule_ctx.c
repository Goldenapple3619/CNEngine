#include "libcncore.h"
#include "libcngraphic.h"

static cn_value _draw(Object *__this, void *args)
{
    (void)args;

    cn_value *temp = get_attr(__this, "w");

    if (!temp)
        return (null_value);
    clear_window((Window *)temp->as.ptr, 0xff0000ff);
    draw_window((Window *)temp->as.ptr);

    return (null_value);
}

static cn_value _init(Object *__this, void *args)
{
    (void)__this;

    Videomode v = {.size.x = 800, .size.y = 600, .position.x = 0, .position.y = 0, .flags = VDM_CLOSABLE, .native_flags = VDM_N_SHWN};
    Window *w;

    if (!args || !(((void **)args)[0]))
        return (VALUE_ERR);
    
    Object *ctx = (Object *)(((void **)args)[0]);
    
    if (!start_graphics())
        return (VALUE_ERR);

    w = new_window("test", NULL, &v);

    if (!w)
        return (VALUE_ERR);

    if (!set_attr(ctx, "w", CN_TYPE_GENERIC_UNIQ_PTR, w))
        return (VALUE_ERR);

    if (!set_method(ctx, "draw", _draw))
        return (VALUE_ERR);
    
    return (VALUE_OK);
}

static cn_value _del(Object *__this, void *args)
{
    (void)__this;

    if (!args || !(((void **)args)[0]))
        return (VALUE_ERR);
    
    Object *ctx = (Object *)(((void **)args)[0]);
    cn_value *temp = get_attr(ctx, "w");
    
    if (temp)
        delete_window(temp->as.ptr);
    end_graphics();
    
    return (VALUE_OK);
}

CN_API Object *new_graphic_submodule(void)
{
    Object *obj = new_object();

    if (!obj)
        return (NULL);
    obj->base = create_default_object();
    if (!obj->base)
        return (NULL);
    set_method(obj, "_init", &_init);
    set_method(obj, "_del", &_del);
    return (obj);
}

