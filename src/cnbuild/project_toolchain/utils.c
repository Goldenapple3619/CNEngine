#include "project_toolchain.h"

char *resolve_path(char *base_path, const char *project_root)
{
    const char *placeholder = "${project_root}";
    char *pos;
    char *new_path;
    size_t prefix_len;
    size_t suffix_len;
    size_t root_len;

    if (!base_path || !project_root) {
        return (NULL);
    }

    pos = strstr(base_path, placeholder);

    if (!pos)
        return (base_path);

    prefix_len = (size_t)(pos - base_path);
    suffix_len = strlen(pos + strlen(placeholder));
    root_len = strlen(project_root);

    new_path = malloc(prefix_len + root_len + suffix_len + 1);

    if (!new_path) {
        (void)free(base_path);
        return (NULL);
    }

    (void)memcpy(new_path, base_path, prefix_len);
    (void)memcpy(new_path + prefix_len, project_root, root_len);
    (void)memcpy(new_path + prefix_len + root_len,
           pos + strlen(placeholder),
           suffix_len + 1);

    (void)free(base_path);
    return new_path;
}