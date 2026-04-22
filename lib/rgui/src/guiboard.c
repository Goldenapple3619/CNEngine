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
    INIT_INT(__this, mode->flags, "_flags");

    INIT_CUSTOM_ALLOCATION(__this, new_texture(&mode->resolution, true), delete_texture, "texture");
    INIT_OBJECT_STATIC(__this, new_list(), NULL, "elements");

    return (VALUE_OK);
}

static cn_value _update(Object *__this, void **args)
{
    Object *elements = get_attr(__this, "elements")->as.ptr;
    Object *temp;

    for (struct list_iterator_s it = list_get_iterator(elements); !list_iterator_isend(&it); list_iterator_next(&it)) {
        if (list_iterator_value_isnull(&it))
            continue;
        
        temp = it.val.as.ptr;
        
        if (has_method(temp, "_update"))
            (void)call_method(temp, "_update", args);
    } 

    return (null_value);
}

static cn_value _events(Object *__this, void **args)
{
    Object *elements = get_attr(__this, "elements")->as.ptr;
    Object *temp;

    if (!args || !args[0])
        return (null_value);

    Window *window = args[0];
    
    if (((get_attr(__this, "_flags")->as.i & FLAG_RGUI_DYNAMIC_RESOLUTION) > 0) && has_event_window(window, EV_RESIZE)) {
        const struct event_map_entry_s *events = get_event_window(window, EV_RESIZE);
        Vector2 new_size = (Vector2){events->events[events->size - 1]->x, events->events[events->size - 1]->y};

        set_attr(__this, "resolution", CN_TYPE_VEC2, &new_size);
        set_attr(__this, "upscale", CN_TYPE_VEC2, &new_size);
        resize_texture(get_attr(__this, "texture")->as.ptr, &new_size);
    }

    for (struct list_iterator_s it = list_get_iterator(elements); !list_iterator_isend(&it); list_iterator_next(&it)) {
        if (list_iterator_value_isnull(&it))
            continue;
        
        temp = it.val.as.ptr;
        
        if (has_method(temp, "_events"))
            (void)call_method(temp, "_events", args);
    } 

    return (null_value);
}

static cn_value _draw(Object *__this, void **args)
{
    if (!args || !args[0])
        return (null_value);

    Vector2 *upscale = &get_attr(__this, "upscale")->as.vec2;
    Vector2 *resolution = &get_attr(__this, "resolution")->as.vec2;
    Vector2 *position = &get_attr(__this, "position")->as.vec2;
    Object *elements = get_attr(__this, "elements")->as.ptr;
    Texture *texture = get_attr(__this, "texture")->as.ptr;
    Window *window = args[0];
    Quad *gl_quad = NULL;
    Object *temp;

    if (((window->video_mode.flags & VDM_CPU) > 0))
        clear_texture(texture, 0x00000000);

    cnrgui_alignement temp_align;
    Vector2 temp_position;
    Vector2 computed_upscale = (Vector2){upscale->x / resolution->x, upscale->y / resolution->y};
    Texture *temp_texture;

    if (((window->video_mode.flags & VDM_OPENGL) > 0)) {
        glViewport(position->x, position->y, window->video_mode.size.x, window->video_mode.size.y);
        if (!has_attr(__this, "gl_quad")) {
            PREP_INIT(); INIT_CUSTOM_ALLOCATION(__this, new_quad2d(), delete_quad, "gl_quad");
        }
        gl_quad = get_attr(__this, "gl_quad")->as.ptr;
    }

    for (struct list_iterator_s it = list_get_iterator(elements); !list_iterator_isend(&it); list_iterator_next(&it)) {
        if (list_iterator_value_isnull(&it))
            continue;
        
        temp = it.val.as.ptr;

        if (has_attr(temp, "texture")) {
            temp_position = get_attr(temp, "position")->as.vec2;
            temp_align = get_attr(temp, "align")->as.i;
            temp_texture = get_attr(temp, "texture")->as.ptr;

            if (temp_align == GUI_ALIGN_MIDDLE)
                temp_position.x += (resolution->x / 2 - temp_texture->size.x / 2);
            if (temp_align == GUI_ALIGN_RIGHT)
                temp_position.x = (resolution->x - temp_texture->size.x) - temp_position.x;

            if (((window->video_mode.flags & VDM_CPU) > 0))
                blit(temp_texture, texture, NULL, &temp_position);
            else if ((window->video_mode.flags & VDM_GPU) > 0) {
                if (temp_texture->api == R_API_NONE)
                    temp_texture->api = R_API_SDL;
                draw_texture(temp_texture, window->renderer, NULL, &(Vector2){.x = position->x + temp_position.x * computed_upscale.x, .y = position->y + temp_position.y * computed_upscale.y}, &computed_upscale, 0);
             } else if ((window->video_mode.flags & VDM_OPENGL) > 0) {
                if (temp_texture->api == R_API_NONE)
                    temp_texture->api = R_API_GL;

                draw_texture_gl(
                    temp_texture, gl_quad,
                    &(Vector2){temp_position.x * computed_upscale.x, temp_position.y * computed_upscale.y},
                    &(Vector2){temp_texture->size.x * computed_upscale.x, temp_texture->size.y * computed_upscale.y},
                    (cncolor)0xffffffff,
                    upscale
                );
            } else
                continue;
                // not implemented
        }

        if (has_method(temp, "_draw"))
            (void)call_method(temp, "_draw", (cnany []){__this, window, NULL});
    }

    if (((window->video_mode.flags & VDM_CPU) > 0))
        blit_ratio(texture, window->texture, NULL, position, &computed_upscale);

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