#include "../project_toolchain.h"

CNProject *parse_project_xml(const char *file_path, const char *project_root)
{
    CNProject *parsed_data; 
    xmlDoc *doc;
    xmlNode *root;

    doc = xmlReadFile(file_path, NULL, 0);

    if (!doc) {
        fprintf(stderr, "%s: failed to open and parse file.\n", file_path);
        return (NULL);
    }

    root = xmlDocGetRootElement(doc);

    if (strcmp((const char *)root->name, "project")) {
        fprintf(stderr, "%s: invalid root element '%s', expecting 'gui'.\n", file_path, root->name);
        (void)xmlFreeDoc(doc);
        (void)xmlCleanupParser();
        return (NULL);
    }

    parsed_data = new_cnproject();

    if (!parsed_data) {
        (void)xmlFreeDoc(doc);
        (void)xmlCleanupParser();
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
                (void)delete_cnproject(parsed_data);
                (void)xmlFreeDoc(doc);
                (void)xmlCleanupParser();
                return (NULL);
            }
        } else if (!strcmp((const char *)node->name, "version")) {
            if (parsed_data->version_name)
                (void)free(parsed_data->version_name);

            parsed_data->version_name = string_from_node(node);

            if (!parsed_data->version_name) {
                (void)delete_cnproject(parsed_data);
                (void)xmlFreeDoc(doc);
                (void)xmlCleanupParser();
                return (NULL);
            }
        } else if (!strcmp((const char *)node->name, "builds")) {
            if (parse_cnbuilds_xml(parsed_data, node)) {
                (void)delete_cnproject(parsed_data);
                (void)xmlFreeDoc(doc);
                (void)xmlCleanupParser();
                return (NULL);
            }
        } else if (!strcmp((const char *)node->name, "ressources")) {
            if (parse_cnressources_xml(parsed_data, node, project_root)) {
                (void)delete_cnproject(parsed_data);
                (void)xmlFreeDoc(doc);
                (void)xmlCleanupParser();
                return (NULL);
            }
        } else {
            fprintf(stderr, "%s: invalid element '%s'.\n", file_path, root->name);
            (void)delete_cnproject(parsed_data);
            (void)xmlFreeDoc(doc);
            (void)xmlCleanupParser();
            return (NULL);
        }
    }

    (void)xmlFreeDoc(doc);
    (void)xmlCleanupParser();

    return (parsed_data);
}
