#include "build.h"

void build_gui_element(xmlNode *node)
{
    xmlChar *content;

    for (xmlNode *node_child = node->children; node_child; node_child = node_child->next) {
        if (node_child->type == XML_ELEMENT_NODE)
            build_gui_element(node_child);
        if (node_child->type == XML_TEXT_NODE) {
            content = xmlNodeGetContent(node_child);
            printf("  %s  -  %s\n", node_child->parent->name, content);
            xmlFree(content);
        }
    }
}

int build_gui(size_t argc, char **argv)
{
    xmlDoc *doc;
    xmlNode *root;
    xmlChar *temp_s;
    struct engine_object_file_writer_ctx_s *wctx = new_writer_ctx(NULL);

    if (!wctx) {
        fprintf(stderr, "writter ctx allocation failed.\n");
        return (1);
    }

    if (argc < 4) {
        fprintf(stderr, "%s: no file provided.\n", argv[0]);
        return (1);
    }

    doc = xmlReadFile(argv[3], NULL, 0);

    if (!doc) {
        fprintf(stderr, "%s: failed to open and parse file '%s'.\n", argv[0], argv[3]);
        return (1);
    }

    root = xmlDocGetRootElement(doc);

    if (strcmp((const char *)root->name, "gui")) {
        fprintf(stderr, "%s: invalid root element '%s', expecting 'gui'.\n", argv[3], root->name);
        return (1);
    }

    temp_s = xmlGetProp(root, (xmlChar *)"name");
    if (writer_ctx_set_object_name(wctx, (const char *)temp_s)) {
        fprintf(stderr, "string allocation failed.\n");
        return (1);
    }
    xmlFree(temp_s);

    for (xmlNode *node = root->children; node; node = node->next) {
        if (node->type != XML_ELEMENT_NODE)
            continue;
        if (!strcmp((const char *)node->name, "content")) {
            for (xmlNode *node_child = node->children; node_child; node_child = node_child->next) {
                if (node_child->type != XML_ELEMENT_NODE)
                    continue;
                build_gui_element(node_child);
            }
        } else if (!strcmp((const char *)node->name, "connectors")) {
            for (xmlNode *node_child = node->children; node_child; node_child = node_child->next) {
                if (node_child->type != XML_ELEMENT_NODE)
                    continue;
            }
        } else {
            fprintf(stderr, "%s: invalid element '%s'.\n", argv[3], root->name);
            return (1);
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
