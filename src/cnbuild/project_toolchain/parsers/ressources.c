#include "../project_toolchain.h"

uint8_t parse_cnressources_xml(CNProject *project, xmlNode *node, const char *project_root)
{
    char *res_path;
    cnasset_type tp;
    CNAsset *temp_res;
    xmlChar *temp_s;

    for (xmlNode *node_child = node->children; node_child; node_child = node_child->next) {
        if (node_child->type != XML_ELEMENT_NODE)
            continue;

        res_path = string_from_node(node_child);

        if (!res_path)
            return (1);

        res_path = resolve_path(res_path, project_root);

        if (!res_path)
            return (1);

        temp_s = xmlGetProp(node_child, (xmlChar *)"type");

        if (!temp_s) {
            fprintf(stderr, "missing element type for '%s'.\n", node_child->name);
            (void)free(res_path);
            return (1);
        }

        tp = tp_from_string((const char *)temp_s);
        xmlFree(temp_s);

        if (tp < 0) {
            fprintf(stderr, "invalid element type for '%s'.\n", node_child->name);
            (void)free(res_path);
            return (1);
        }

        if (!strcmp((const char *)node_child->name, "dir")) {
            if (cnressources_walk_path(project, res_path, tp)) {
                (void)free(res_path);
                return (1);
            }
        } else if (!strcmp((const char *)node_child->name, "file")) {
            temp_res = new_cnasset(res_path, tp);

            if (!temp_res)
                return (1);

            if (insert_generic_vector(&project->content, temp_res)) {
                (void)delete_cnasset(temp_res);
                return (1);
            }
        } else {
            fprintf(stderr, "invalid element '%s' in '%s'.\n", node_child->name, node->name);
            (void)free(res_path);
            return (1);
        }

        (void)free(res_path);
    }

    return (0);
}
