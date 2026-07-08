#include "libcncore.h"

CN_API String *new_str_from_const(const char *c_str)
{
    String *str = malloc(sizeof(String));

    if (!str) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate String.");
        return (NULL);
    }

    str->c_str = c_str ? strdup(c_str) : NULL;

    if (c_str && !str->c_str) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate cstring.");
        (void)free(str);
        return (NULL);
    }

    str->size = c_str ? strlen(c_str) : 0;

    return (str);
}

CN_API String *new_str(char *c_str)
{
    String *str = malloc(sizeof(String));

    if (!str) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate String.");
        return (NULL);
    }

    str->c_str = c_str ? c_str : NULL;
    str->size = c_str ? strlen(c_str) : 0;

    return (str);
}

CN_API cnbool str_is_empty(const String *str)
{
    if (!str) {
        RAISE(ERR_INVALID_POINTER, "can't check if null string is empty.");
        return (true);
    }

    return (str->size ? true : false);
}

CN_API cnbool str_is_null(const String *str)
{
    if (!str) {
        RAISE(ERR_INVALID_POINTER, "can't check if null string contain null c_str.");
        return (true);
    }

    return (!str->c_str ? true : false);
}

CN_API void str_override(String *str, char *c_str)
{
    if (!str) {
        RAISE(ERR_INVALID_POINTER, "can't override empty str content.");
        return;
    }

    if (str->c_str)
        (void)free(str->c_str);

    str->c_str = c_str;
    str->size = c_str ? strlen(c_str) : 0;
}

CN_API uint8_t str_rcadd_mv(String *str, char *c_str)
{
    if (!str) {
        RAISE(ERR_INVALID_POINTER, "can't add_move empty str.");
        return (1);
    }

    if (!c_str) {
        RAISE(ERR_INVALID_POINTER, "can't add_move empty c_str to str.");
        return (1);
    }

    size_t len = str->size + strlen(c_str);
    char *new_cstr = malloc(sizeof(char) * (len + 1));

    if (!new_cstr) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new c_str.");
        return (1);
    }

    (void)strcpy(new_cstr, str->c_str ? str->c_str : "");
    (void)strcpy(new_cstr + str->size, c_str);
    (void)free(c_str);

    (void)str_override(str, new_cstr);
    return (0);
}

CN_API uint8_t str_rcadd_cp(String *str, const char *c_str)
{
    if (!str) {
        RAISE(ERR_INVALID_POINTER, "can't add_copy empty str.");
        return (1);
    }

    if (!c_str) {
        RAISE(ERR_INVALID_POINTER, "can't add_copy empty c_str to str.");
        return (1);
    }

    size_t len = str->size + strlen(c_str);
    char *new_cstr = malloc(sizeof(char) * (len + 1));

    if (!new_cstr) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new c_str.");
        return (1);
    }

    (void)strcpy(new_cstr, str->c_str ? str->c_str : "");
    (void)strcpy(new_cstr + str->size, c_str);

    (void)str_override(str, new_cstr);
    return (0);
}

CN_API void delete_str(String *str)
{
    if (!str) {
        RAISE(ERR_INVALID_POINTER, "can't delete empty string.");
        return;
    }

    if (str->c_str) {
        (void)free(str->c_str);
        str->c_str = NULL;
    }
    str->size = 0;
    (void)free(str);
}
