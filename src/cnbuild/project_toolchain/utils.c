#include "project_toolchain.h"

char *resolve_path(char *base_path, const char *replace_with, const char *placeholder)
{
    char *pos;
    char *new_path;
    size_t prefix_len;
    size_t suffix_len;
    size_t root_len;

    if (!base_path || !replace_with) {
        RAISE(ERR_INVALID_POINTER, "can't resolve empty path/empty project root.");
        return (NULL);
    }

    pos = strstr(base_path, placeholder);

    if (!pos)
        return (base_path);

    prefix_len = (size_t)(pos - base_path);
    suffix_len = strlen(pos + strlen(placeholder));
    root_len = strlen(replace_with);

    new_path = malloc(prefix_len + root_len + suffix_len + 1);

    if (!new_path) {
        RAISE_FMT(ERR_OUT_OF_MEMORY, "failed to allocate new resolved path with '%s' & '%s'.", base_path, replace_with);
        (void)free(base_path);
        return (NULL);
    }

    (void)memcpy(new_path, base_path, prefix_len);
    (void)memcpy(new_path + prefix_len, replace_with, root_len);
    (void)memcpy(new_path + prefix_len + root_len,
           pos + strlen(placeholder),
           suffix_len + 1);

    (void)free(base_path);
    return new_path;
}