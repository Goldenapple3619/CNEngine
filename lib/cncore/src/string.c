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

CN_API uint8_t str_override_cp(String *str, const char *c_str)
{
    if (!str) {
        RAISE(ERR_INVALID_POINTER, "can't override empty str content.");
        return (1);
    }

    if (str->c_str)
        (void)free(str->c_str);

    if (!c_str) {
        str->c_str = NULL;
        str->size = 0;
        return (0);
    }

    str->c_str = strdup(c_str);
    str->size = str->c_str ? strlen(str->c_str) : 0;

    if (!str->c_str) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new c_str.");
        return (1);
    }

    return (0);
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

CN_API void empty_str(String *str)
{
    if (!str) {
        RAISE(ERR_INVALID_POINTER, "can't empty empty string.");
        return;
    }

    if (str->c_str) {
        (void)free(str->c_str);
        str->c_str = NULL;
    }
    str->size = 0;
}

CN_API uint8_t str_replace(String *str, const char *to_replace, const char *with, size_t count)
{
    if (!str) {
        RAISE(ERR_INVALID_POINTER, "can't replace in empty string.");
        return (1);
    }

    if (!to_replace || !*to_replace) {
        RAISE(ERR_INVALID_POINTER, "can't replace an empty or null token.");
        return (1);
    }

    if (!str->c_str) {
        return (0);
    }

    const char *with_str = with ? with : "";
    const char *scan = str->c_str;
    const char *found;
    char *new_cstr;

    size_t to_replace_len = strlen(to_replace);
    size_t with_len = strlen(with_str);
    size_t occurrences = 0;
    size_t new_size;

    while ((count == 0 || occurrences < count) && (found = strstr(scan, to_replace))) {
        occurrences++;
        scan = found + to_replace_len;
    }

    if (!occurrences) {
        return (0);
    }

    new_size = str->size - (occurrences * to_replace_len) + (occurrences * with_len);
    new_cstr = malloc(sizeof(char) * (new_size + 1));

    if (!new_cstr) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new c_str.");
        return (1);
    }

    char *dst = new_cstr;
    const char *src = str->c_str;
    size_t replaced = 0;
    size_t chunk_len;
    size_t remaining;

    while ((count == 0 || replaced < count) && (found = strstr(src, to_replace))) {
        chunk_len = (size_t)(found - src);
        (void)memcpy(dst, src, chunk_len);
        dst += chunk_len;
        (void)memcpy(dst, with_str, with_len);
        dst += with_len;
        src = found + to_replace_len;
        replaced++;
    }

    remaining = strlen(src);
    (void)memcpy(dst, src, remaining);
    dst += remaining;
    *dst = '\0';

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
