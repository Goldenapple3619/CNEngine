#include "librgui.h"

static cn_value _init(Object *__this, void **args)
{
    PREP_INIT()

    if (!args || !args[0]) {
        RAISE(ERR_INVALID_POINTER, "can't init text with no text mode.");
        return (VALUE_ERR);
    }

    struct text_mode_s *text_mode = args[0];

    if (call_method(__this->base, "_init", PACK_ARG(&text_mode->parent_mode)).as.i == VALUE_ERR.as.i) {
        PROPAGATE_ERR();
        return (VALUE_ERR);
    }

    SDL_Color c = (SDL_Color){.r = (text_mode->color & 0xff000000) >> 24,
            .g = (text_mode->color & 0x00ff0000) >> 16,
            .b = (text_mode->color & 0x0000ff00) >> 8,
            .a = (text_mode->color & 0x000000ff)};
    

    INIT_STRING(__this, text_mode->text, "text");
    INIT_INT(__this, text_mode->color, "color");
    INIT_INT(__this, text_mode->font_size, "font_size");

    INIT_CUSTOM_ALLOCATION(__this, TTF_OpenFont(text_mode->font_location, text_mode->font_size), TTF_CloseFont, "font");
    INIT_CUSTOM_ALLOCATION(__this, new_texture_from_surface(TTF_RenderUTF8_Blended(get_attr(__this, "font")->as.ptr, text_mode->text, c)), delete_texture, "texture");

    INIT_VEC2(__this, ((Texture *)get_attr(__this, "texture")->as.ptr)->size, "size");

    return (VALUE_OK);
}

static cn_value _set_text(Object *__this, void **args)
{
    if (!args || !args[0]) {
        RAISE(ERR_INVALID_POINTER, "can't set text with no text.");
        return (VALUE_ERR);
    }
    if (!strcmp(args[0], get_attr(__this, "text")->as.ptr))
        return (VALUE_OK);

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

    if (!obj) {
        PROPAGATE_ERR();
        return (NULL);
    }

    SET_PARENT_CLASS_BUILD_STATIC(obj, new_guiobject());
    CREATE_METHOD_CLASS_BUILD(obj, "_init", &_init);
    CREATE_METHOD_CLASS_BUILD(obj, "set_text", &_set_text);
    CREATE_METHOD_CLASS_BUILD(obj, "_del", &_del);
    return (obj);
}