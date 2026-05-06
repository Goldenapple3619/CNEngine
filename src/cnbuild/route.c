#include "build.h"

uint8_t write_object_file(const struct engine_object_file_s *object_file_write_ctx)
{
    (void)object_file_write_ctx;
    return (0);
}

int build_gui(size_t argc, char **argv)
{
    xmlDoc *doc;
    xmlNode *root;

    if (argc < 4) {
        fprintf(stderr, "%s: no file provided.", argv[0]);
        return (1);
    }

    doc = xmlReadFile(argv[3], NULL, 0);

    if (!doc) {
        fprintf(stderr, "%s: failed to open and parse file '%s'.", argv[0], argv[3]);
        return (1);
    }

    root = xmlDocGetRootElement(doc);

    if (strcmp((const char *)root->name, "gui")) {
        fprintf(stderr, "%s: invalid root element '%s', expecting 'gui'.", argv[3], root->name);
        return (1);
    }

    root->properties;

    for (xmlNode *node = root->children; node; node = node->next) {
        if (node->type == XML_ELEMENT_NODE) {
            xmlChar *content = xmlNodeGetContent(node);

            printf("node: %s\n", node->name);
            printf("content: %s\n", content);
            xmlFree(content);
        }
    }

    (void)xmlFreeDoc(doc);
    (void)xmlCleanupParser();
    return 0;
}

int build(size_t argc, char **argv)
{
    size_t i = 0;

    if (argc < 3) {
        fprintf(stderr, "%s: asset build toolchain missing.", argv[0]);
        return (1);
    }

    while ((*(build_types + i)).name) {
        if (!strcmp((*(build_types + i)).name, argv[2]))
            return ((*(build_types + i)).callback(argc, argv));
        ++i;
    };

    fprintf(stderr, "%s: invalid build toolchain '%s'.", argv[0], argv[2]);

    return (1);
}
