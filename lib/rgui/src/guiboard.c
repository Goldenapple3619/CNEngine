#include "librgui.h"

static cn_value _init(Object *__this, void **args)
{
    PREP_INIT()

    if (!args || !args[0])
        return (VALUE_ERR);

    struct gui_board_mode_s *mode = args[0];

    Vector2 upscale = (Vector2){
        .x = (mode->upscale.x != -1 ? mode->upscale.x : mode->resolution.x),
        .y = (mode->upscale.y != -1 ? mode->upscale.y : mode->resolution.y),
    };

    INIT_VEC2(__this, mode->position, "position");
    INIT_VEC2(__this, mode->resolution, "resolution");
    INIT_VEC2(__this, upscale, "upscale");

    INIT_CUSTOM_ALLOCATION(__this, new_texture(&mode->resolution, true), delete_texture, "texture");
    INIT_CUSTOM_ALLOCATION(__this, build_object(new_list(), NULL), delete_object, "elements");

    return (VALUE_OK);
}

static cn_value _update(Object *__this, void **args)
{
    Object *elements = get_attr(__this, "elements")->as.ptr;
    size_t len = call_method(elements, "len", NULL).as.i;
    cn_value val;
    Object *temp;

    for (size_t i = 0; i < len; ++i) {
        val = call_method(elements, "at", (cnany []){(size_t []){i}, NULL});

        if (val.type == CN_TYPE_NULL)
            continue;
        
        temp = val.as.ptr;
        
        if (has_method(temp, "_update"))
            (void)call_method(temp, "_update", args);
    } 

    return (null_value);
}

static cn_value _events(Object *__this, void **args)
{
    Object *elements = get_attr(__this, "elements")->as.ptr;
    size_t len = call_method(elements, "len", NULL).as.i;
    cn_value val;
    Object *temp;

    for (size_t i = 0; i < len; ++i) {
        val = call_method(elements, "at", (cnany []){(size_t []){i}, NULL});

        if (val.type == CN_TYPE_NULL)
            continue;
        
        temp = val.as.ptr;
        
        if (has_method(temp, "_events"))
            (void)call_method(temp, "_events", args);
    } 

    return (null_value);
}

static cn_value _draw(Object *__this, void **args)
{
    if (!args || !args[0])
        return (null_value);

    Object *elements = get_attr(__this, "elements")->as.ptr;
    Texture *texture = get_attr(__this, "texture")->as.ptr;
    size_t len = call_method(elements, "len", NULL).as.i;
    Window *window = args[0];
    cn_value val;
    Object *temp;

    clear_texture(texture, 0x00000000);

    for (size_t i = 0; i < len; ++i) {
        val = call_method(elements, "at", (cnany []){(size_t []){i}, NULL});

        if (val.type == CN_TYPE_NULL)
            continue;
        
        temp = val.as.ptr;
        
        if (has_method(temp, "_draw"))
            (void)call_method(temp, "_draw", (cnany []){__this, NULL});
    }

    Vector2 upscale = get_attr(__this, "upscale")->as.vec2;
    Vector2 resolution = get_attr(__this, "resolution")->as.vec2;

    blit_ratio(texture, window->texture, NULL, &(get_attr(__this, "position")->as.vec2), &(Vector2){upscale.x / resolution.x, upscale.y / resolution.y});

    return (null_value);
}

static cn_value _add_element(Object *__this, void **args)
{
    if (!args || !args[0])
        return (VALUE_ERR);

    if (call_method(get_attr(__this, "elements")->as.ptr, "push", args).as.i == VALUE_ERR.as.i)
        return (VALUE_ERR);
    return (VALUE_OK);
}

static cn_value _del(Object *__this, void **args)
{
    (void)args;

    PREP_DEL();

    DEL_CUSTOM_ALLOCAION(__this, delete_texture, "texture");
    DEL_CUSTOM_ALLOCAION(__this, delete_object, "elements");

    return (null_value);
}

CN_API Object *new_guiboard(void)
{
    Object *obj = new_object();

    if (!obj)
        return (NULL);

    SET_PARENT_CLASS_BUILD(obj, create_default_object());
    CREATE_METHOD_CLASS_BUILD(obj, "_init", &_init);
    CREATE_METHOD_CLASS_BUILD(obj, "_events", &_events);
    CREATE_METHOD_CLASS_BUILD(obj, "_update", &_update);
    CREATE_METHOD_CLASS_BUILD(obj, "_draw", &_draw);
    CREATE_METHOD_CLASS_BUILD(obj, "add_element", &_add_element);
    CREATE_METHOD_CLASS_BUILD(obj, "_del", &_del);
    return (obj);
}