#include "../project_toolchain.h"

CNProject *parse_project_xml(const EngineConfig *config, const char *file_path, const char *project_root)
{
    CNProject *parsed_data;
    xmlDoc *doc;
    xmlNode *root;

    doc = xmlReadFile(file_path, NULL, 0);

    if (!doc) {
        RAISE_FMT(ERR_OS, "failed to open and parse file '%s'.", file_path);
        return (NULL);
    }

    root = xmlDocGetRootElement(doc);

    if (strcmp((const char *)root->name, "project")) {
        RAISE_FMT(ERR_INVALID_TYPE, "invalid element '%s' in '%s'.", root->name, file_path);
        (void)xmlFreeDoc(doc);
        return (NULL);
    }

    parsed_data = new_cnproject();

    if (!parsed_data) {
        PROPAGATE_ERR();
        (void)xmlFreeDoc(doc);
        return (NULL);
    }

    parsed_data->root = strdup(project_root);

    if (!parsed_data->root) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new project root string.");
        (void)delete_cnproject(parsed_data);
        (void)xmlFreeDoc(doc);
        return (NULL);
    }

    for (xmlNode *node = root->children; node; node = node->next) {
        if (node->type != XML_ELEMENT_NODE)
            continue;
        if (!strcmp((const char *)node->name, "name")) {
            if (parsed_data->name)
                (void)free(parsed_data->name);

            parsed_data->name = string_from_node(node);

            if (!parsed_data->name) {
                PROPAGATE_ERR();
                (void)delete_cnproject(parsed_data);
                (void)xmlFreeDoc(doc);
                return (NULL);
            }
        } else if (!strcmp((const char *)node->name, "version")) {
            if (parsed_data->version_name)
                (void)free(parsed_data->version_name);

            parsed_data->version_name = string_from_node(node);

            if (!parsed_data->version_name) {
                PROPAGATE_ERR();
                (void)delete_cnproject(parsed_data);
                (void)xmlFreeDoc(doc);
                return (NULL);
            }
        } else if (!strcmp((const char *)node->name, "builds")) {
            if (parse_cnbuilds_xml(config, parsed_data, node)) {
                PROPAGATE_ERR();
                (void)delete_cnproject(parsed_data);
                (void)xmlFreeDoc(doc);
                return (NULL);
            }
        } else if (!strcmp((const char *)node->name, "ressources")) {
            if (parse_cnressources_xml(parsed_data, node, project_root)) {
                PROPAGATE_ERR();
                (void)delete_cnproject(parsed_data);
                (void)xmlFreeDoc(doc);
                return (NULL);
            }
        } else {
            RAISE_FMT(ERR_INVALID_TYPE, "invalid element '%s' in '%s'.", node->name, root->name);
            (void)delete_cnproject(parsed_data);
            (void)xmlFreeDoc(doc);
            return (NULL);
        }
    }

    (void)xmlFreeDoc(doc);
    return (parsed_data);
}
