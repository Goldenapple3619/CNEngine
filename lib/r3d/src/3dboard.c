#include "libr3d.h"

static cn_value _init(Object *__this, void **args)
{
    if (!args || !args[0])
        return (VALUE_ERR);

    struct threed_board_mode_s *mode = args[0];

    Vector2 upscale = (Vector2){
        .x = (mode->upscale.x != -1 ? mode->upscale.x : mode->resolution.x),
        .y = (mode->upscale.y != -1 ? mode->upscale.y : mode->resolution.y),
    };

    INIT_VEC2(__this, mode->position, "position");
    INIT_VEC2(__this, mode->resolution, "resolution");
    INIT_VEC2(__this, upscale, "upscale");

    INIT_OBJECT_SHR(__this, mode->scene, "scene");

    return (VALUE_OK);
}

static cn_value _draw(Object *__this, void **args)
{
    (void)__this;
    (void)args;

    Window *window = args[0];

    Object *scene = get_attr(__this, "scene")->as.ptr;
    Object *elements = get_attr(scene, "objects")->as.ptr;
    size_t len = call_method(elements, "len", NULL).as.i;
    cn_value val;
    Object *temp;
    int64_t flags;
    Vector3 *temp_position;
    Vector3 *temp_scale;
    Rect *temp_rotation;

    for (size_t i = 0; i < len; ++i) {
        val = call_method(elements, "at", (cnany []){(size_t []){i}, NULL});

        if (val.type == CN_TYPE_NULL)
            continue;
        
        temp = val.as.ptr;
        flags = get_attr(temp, "_flags")->as.i;
        
        if (!((flags & CN_OBJ_DRAWABLE) > 0))
            continue;

        temp_position = &get_attr(temp, "position")->as.vec3;
        temp_scale = &get_attr(temp, "scale")->as.vec3;
        temp_rotation = &get_attr(temp, "rotation")->as.rect;

        (void)temp_position;
        (void)temp_scale;
        (void)temp_rotation;

        if (has_method(temp, "_draw"))
            (void)call_method(temp, "_draw", (cnany []){__this, window, NULL});
    }

    Vector2 upscale = get_attr(__this, "upscale")->as.vec2;
    Vector2 resolution = get_attr(__this, "resolution")->as.vec2;

    (void)upscale;
    (void)resolution;

    return (null_value);
}

static cn_value _del(Object *__this, void **args)
{
    (void)args;
    (void)__this;

    return (null_value);
}

CN_API Object *new_3dboard(void)
{
    Object *obj = new_object();

    if (!obj)
        return (NULL);

    SET_PARENT_CLASS_BUILD(obj, create_default_object());
    CREATE_METHOD_CLASS_BUILD(obj, "_init", &_init);
    CREATE_METHOD_CLASS_BUILD(obj, "_draw", &_draw);
    CREATE_METHOD_CLASS_BUILD(obj, "_del", &_del);
    return (obj);
}