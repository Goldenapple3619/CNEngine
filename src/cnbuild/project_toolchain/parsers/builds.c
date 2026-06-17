#include "../project_toolchain.h"

uint8_t parse_cnbuilds_xml(CNProject *project, xmlNode *node)
{
    CNBuild *build;

    (void)project;
    (void)build;
    for (xmlNode *node_child = node->children; node_child; node_child = node_child->next) {
        if (node_child->type != XML_ELEMENT_NODE)
            continue;

        if (!strcmp((const char *)node_child->name, "binary")) {
            for (xmlNode *build_content_node = node_child->children; build_content_node; build_content_node = build_content_node->next) {
                if (build_content_node->type != XML_ELEMENT_NODE)
                    continue;

                if (!strcmp((const char *)build_content_node->name, "dependencies")) {

                } else if (!strcmp((const char *)build_content_node->name, "name")) {

                } else if (!strcmp((const char *)build_content_node->name, "entry")) {

                } else {
                    fprintf(stderr, "invalid element '%s' in '%s'.\n", build_content_node->name, node_child->name);
                    return (1);
                }
            }
        } else {
            fprintf(stderr, "invalid element '%s' in '%s'.\n", node_child->name, node->name);
            return (1);
        }
    }

    return (0);
}
