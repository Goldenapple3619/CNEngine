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

char *string_from_node(xmlNode *node)
{
    xmlChar *content;
    char *striped;
    size_t old_len = 0;
    char *text_content = strdup("");

    if (!text_content)
        return (NULL);

    for (xmlNode *node_child = node->children; node_child; node_child = node_child->next) {
        if (node_child->type == XML_TEXT_NODE) {
            content = xmlNodeGetContent(node_child);
            striped = strip_whitespace((const char *)content);
            if (!striped) {
                if (content)
                    (void)xmlFree(content);
                (void)free(text_content);
                return (NULL);
            }
            if (!strlen(striped)) {
                (void)free(striped);
                (void)xmlFree(content);
                continue;
            }
            if (text_content)
                old_len = strlen(text_content);
            else
                old_len = 0;

            text_content = realloc(text_content, sizeof(char) * (old_len + strlen((const char *)striped) + 1));

            if (!text_content) {
                (void)free(striped);
                (void)xmlFree(content);
                return (NULL);
            }

            memcpy(text_content + old_len, striped, sizeof(char) * (strlen(striped) + 1));
            (void)free(striped);
            (void)xmlFree(content);
        }
    }

    return (text_content);
}
