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

    INIT_OBJECT_STATIC(__this, new_list(), NULL, "elements");

    INIT_STRING(__this, "name", "guiboard");

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

static void _cpu_rendering(const Texture *object_texture, Texture *dest_texture, const Vector2 *at)
{
    blit(object_texture, dest_texture, NULL, at);
}

static void _gpu_rendering(Texture *object_texture, SDL_Renderer *renderer, const Vector2 *at, const Vector2 *canva_ratio, const Vector2 *canva_position)
{
    if (object_texture->api == R_API_NONE)
        object_texture->api = R_API_SDL;
    draw_texture(
        object_texture,
        renderer,
        NULL, &(Vector2){
            .x = canva_position->x + at->x * canva_ratio->x,
            .y = canva_position->y + at->y * canva_ratio->y
        }, canva_ratio, 0
    );
}

static void _opengl_rendering(Texture *object_texture, Quad *gl_quad, const Vector2 *at, const Vector2 *canva_ratio, const Vector2 *canva_scale)
{
    if (object_texture->api == R_API_NONE)
        object_texture->api = R_API_GL;

    draw_texture_gl(
        object_texture, gl_quad,
        &(Vector2){at->x * canva_ratio->x, at->y * at->y},
        &(Vector2){object_texture->size.x * canva_ratio->x, object_texture->size.y * canva_ratio->y},
        (cncolor)0xffffffff,
        canva_scale
    );
}

static cn_value _render_object(Object *__this, void **args)
{
    if (!args && !args[0])
        return (null_value);

    gui_render_stack *render_stack = args[0]; 

    if (has_attr(render_stack->obj, "texture")) {
        cnrgui_alignement temp_align = get_attr(render_stack->obj, "align")->as.i;
        Vector2 temp_position = get_attr(render_stack->obj, "position")->as.vec2;
        Texture *temp_texture = get_attr(render_stack->obj, "texture")->as.ptr;

        if (temp_align == GUI_ALIGN_MIDDLE)
            temp_position.x += (render_stack->canva_size.x / 2 - temp_texture->size.x / 2);
        if (temp_align == GUI_ALIGN_RIGHT)
            temp_position.x = (render_stack->canva_size.x - temp_texture->size.x) - temp_position.x;

        if (((render_stack->window->video_mode.flags & VDM_CPU) > 0))
            _cpu_rendering(temp_texture, render_stack->cpu_texture, &temp_position);
        else if ((render_stack->window->video_mode.flags & VDM_GPU) > 0)
            _gpu_rendering(temp_texture, render_stack->window->renderer, &temp_position, &render_stack->canva_ratio, &render_stack->canva_position);
        else if ((render_stack->window->video_mode.flags & VDM_OPENGL) > 0)
            _opengl_rendering(temp_texture, render_stack->gl_quad, &temp_position, &render_stack->canva_ratio, &render_stack->canva_scale);
    }

    if (has_method(render_stack->obj, "_draw"))
        (void)call_method(render_stack->obj, "_draw", PACK_ARG(__this, render_stack->window));

    return (null_value);
}

static cn_value _draw(Object *__this, void **args)
{
    if (!args || !args[0])
        return (null_value);

    gui_render_stack render_stack;
    Object *elements = get_attr(__this, "elements")->as.ptr;

    render_stack.canva_scale = get_attr(__this, "upscale")->as.vec2;
    render_stack.canva_size = get_attr(__this, "resolution")->as.vec2;
    render_stack.canva_position = get_attr(__this, "position")->as.vec2;
    render_stack.canva_ratio = (Vector2){render_stack.canva_scale.x / render_stack.canva_size.x, render_stack.canva_scale.y / render_stack.canva_size.y};
    render_stack.window = args[0];
    render_stack.cpu_texture = NULL;
    render_stack.gl_quad = NULL;

    if (((render_stack.window->video_mode.flags & VDM_CPU) > 0)) {
        if (!has_attr(__this, "texture")) {
            PREP_INIT(); INIT_CUSTOM_ALLOCATION(__this, new_texture(&render_stack.canva_size, true), delete_texture, "texture");
        }

        render_stack.cpu_texture = get_attr(__this, "cpu_texture")->as.ptr;
        clear_texture(render_stack.cpu_texture, 0x00000000);
    }

    if (((render_stack.window->video_mode.flags & VDM_OPENGL) > 0)) {
        glViewport(render_stack.canva_position.x, render_stack.window->video_mode.size.y - render_stack.canva_position.y - render_stack.canva_scale.y, render_stack.canva_scale.x, render_stack.canva_scale.y);

        if (!has_attr(__this, "gl_quad")) {
            PREP_INIT(); INIT_CUSTOM_ALLOCATION(__this, new_quad2d(), delete_quad, "gl_quad");
        }
        render_stack.gl_quad = get_attr(__this, "gl_quad")->as.ptr;
    }

    for (struct list_iterator_s it = list_get_iterator(elements); !list_iterator_isend(&it); list_iterator_next(&it)) {
        if (list_iterator_value_isnull(&it))
            continue;
        
        render_stack.obj = it.val.as.ptr;

        _render_object(__this, PACK_ARG(&render_stack));
    }

    if (((render_stack.window->video_mode.flags & VDM_CPU) > 0))
        blit_ratio(render_stack.cpu_texture, render_stack.window->texture, NULL, &render_stack.canva_position, &render_stack.canva_ratio);

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
    DEL_CUSTOM_ALLOCAION(__this, delete_quad, "gl_quad")

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