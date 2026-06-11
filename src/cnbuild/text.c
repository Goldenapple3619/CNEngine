#include "build.h"

char *strip_whitespace(const char *str)
{
    char *result;
    size_t len;

    if (!str)
        return NULL;

    while (isspace((unsigned char)*str))
        str++;

    if (*str == '\0')
        return (strdup(""));

    const char *end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end))
        end--;

    len = end - str + 1;
    result = malloc(len + 1);
    if (!result)
        return (NULL);

    memcpy(result, str, len);
    result[len] = '\0';
    return (result);
}
