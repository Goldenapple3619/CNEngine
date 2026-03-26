#include "librgui.h"

static cn_value _init(Object *__this, void **args)
{
    PREP_INIT()

    if (!args || !args[0])
        return (VALUE_ERR);

    struct text_mode_s *text_mode = args[0];

    if (call_method(__this->base, "_init", (cnany []){&text_mode->position, NULL}).as.i == VALUE_ERR.as.i)
        return (VALUE_ERR);

    SDL_Color c = (SDL_Color){.r = (text_mode->color & 0xff000000) >> 24,
            .g = (text_mode->color & 0x00ff0000) >> 16,
            .b = (text_mode->color & 0x0000ff00) >> 8,
            .a = (text_mode->color & 0x000000ff)};
    

    INIT_STRING(__this, text_mode->text, "text");
    INIT_INT(__this, text_mode->color, "color");
    INIT_INT(__this, text_mode->size, "size");

    INIT_CUSTOM_ALLOCATION(__this, TTF_OpenFont(text_mode->font_location, text_mode->size), TTF_CloseFont, "font");
    INIT_CUSTOM_ALLOCATION(__this, new_texture_from_surface(TTF_RenderUTF8_Blended(get_attr(__this, "font")->as.ptr, text_mode->text, c)), delete_texture, "texture");

    return (VALUE_OK);
}

static cn_value _set_text(Object *__this, void **args)
{
    if (!args || !args[0])
        return (VALUE_ERR);

    cncolor color = get_attr(__this, "color")->as.i;

    set_attr(__this, "text", CN_TYPE_STRING, args[0]);
    if (get_attr(__this, "texture")->as.ptr)
        (void)delete_texture(get_attr(__this, "texture")->as.ptr);
    set_attr(
        __this, "texture", CN_TYPE_GENERIC_UNIQ_PTR,
        new_texture_from_surface(TTF_RenderUTF8_Blended(
            get_attr(__this, "font")->as.ptr,
            args[0],
            (SDL_Color){.r = (color & 0xff000000) >> 24,
                .g = (color & 0x00ff0000) >> 16,
                .b = (color & 0x0000ff00) >> 8,
                .a = (color & 0x000000ff)}
        )));
    return (VALUE_OK);
}

static cn_value _draw(Object *__this, void **args)
{
    if (!args || !args[0])
        return (null_value);

    if (!has_attr(args[0], "texture"))
        return (null_value);
    
    blit(get_attr(__this, "texture")->as.ptr, get_attr(args[0], "texture")->as.ptr, NULL, &get_attr(__this, "position")->as.vec2);
    return (null_value);
}

static cn_value _del(Object *__this, void **args)
{
    (void)args;
    (void)__this;

    PREP_DEL();

    DEL_CUSTOM_ALLOCAION(__this, TTF_CloseFont, "font");
    DEL_CUSTOM_ALLOCAION(__this, delete_texture, "texture");

    return (null_value);
}

CN_API Object *new_text(void)
{
    Object *obj = new_object();

    if (!obj)
        return (NULL);

    SET_PARENT_CLASS_BUILD(obj, new_guiobject());
    CREATE_METHOD_CLASS_BUILD(obj, "_init", &_init);
    CREATE_METHOD_CLASS_BUILD(obj, "_draw", &_draw);
    CREATE_METHOD_CLASS_BUILD(obj, "set_text", &_set_text);
    CREATE_METHOD_CLASS_BUILD(obj, "_del", &_del);
    return (obj);
}