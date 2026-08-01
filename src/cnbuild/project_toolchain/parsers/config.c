#include "../project_toolchain.h"
#include <string.h>

uint8_t parse_config_generator_xml(EngineConfig *config, xmlNode *node, const char *engine_root)
{
    EngineGeneratorItem *item;
    xmlChar *temp_s;

    for (xmlNode *node_child = node->children; node_child; node_child = node_child->next) {
        if (node_child->type != XML_ELEMENT_NODE)
            continue;

        if (!strcmp((const char *)node_child->name, "include")) {
            item = new_generator_item();

            if (!item) {
                PROPAGATE_ERR();
                return (1);
            }

            item->type = GENT_INCLUDE;
            item->location = string_from_node(node_child);

            if (!item->location) {
                PROPAGATE_ERR();
                (void)delete_generator_item(item);
                return (1);
            }

            item->location =  resolve_path(item->location, engine_root, "${ENGINE_ROOT}");

            if (!item->location) {
                PROPAGATE_ERR();
                (void)delete_generator_item(item);
                return (1);
            }

            temp_s = xmlGetProp(node_child, (xmlChar *)"forsubmodule");

            if (temp_s) {
                item->forsubmodule = strdup((char *)temp_s);
                (void)xmlFree(temp_s);

                if (!item->forsubmodule) {
                    RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new forsubmodule string.");
                    (void)delete_generator_item(item);
                    return (1);
                }
            }

            if (insert_generic_vector(&config->generator, item)) {
                PROPAGATE_ERR();
                (void)delete_generator_item(item);
                return (1);
            }
        } else if (!strcmp((const char *)node_child->name, "src")) {
            item = new_generator_item();

            if (!item) {
                PROPAGATE_ERR();
                return (1);
            }

            item->type = GENT_SRC;
            item->location = string_from_node(node_child);

            if (!item->location) {
                PROPAGATE_ERR();
                (void)delete_generator_item(item);
                return (1);
            }

            item->location =  resolve_path(item->location, engine_root, "${ENGINE_ROOT}");

            if (!item->location) {
                PROPAGATE_ERR();
                (void)delete_generator_item(item);
                return (1);
            }

            temp_s = xmlGetProp(node_child, (xmlChar *)"forsubmodule");

            if (temp_s) {
                item->forsubmodule = strdup((char *)temp_s);
                (void)xmlFree(temp_s);

                if (!item->forsubmodule) {
                    RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new forsubmodule string.");
                    (void)delete_generator_item(item);
                    return (1);
                }
            }

            if (insert_generic_vector(&config->generator, item)) {
                PROPAGATE_ERR();
                (void)delete_generator_item(item);
                return (1);
            }
        } else {
            RAISE_FMT(ERR_INVALID_TYPE, "invalid element '%s' in '%s'.", node_child->name, node->name)
            return (1);
        }
    }

    return (0);
}

uint8_t parse_config_ressources_xml(EngineConfig *config, xmlNode *node, const char *engine_root)
{
    xmlChar *temp_s;
    EngineRessourceSet *set;

    for (xmlNode *node_child = node->children; node_child; node_child = node_child->next) {
        if (node_child->type != XML_ELEMENT_NODE)
            continue;
        if (!strcmp((const char *)node_child->name, "set")) {
            set = new_ressource_set();

            if (!set) {
                PROPAGATE_ERR();
                return (1);
            }

            temp_s = xmlGetProp(node_child, (xmlChar *)"machine");

            if (!temp_s) {
                set->machine = CNBUILD_SYS_HOST;
            } else {
                set->machine = os_from_string((const char *)temp_s);
                xmlFree(temp_s);
            }

            temp_s = xmlGetProp(node_child, (xmlChar *)"architecture");

            if (!temp_s) {
                set->architecture = CNBUILD_ARCH_HOST;
            } else {
                set->architecture = arch_from_string((const char *)temp_s);
                xmlFree(temp_s);
            }

            for (xmlNode *set_content = node_child->children; set_content; set_content = set_content->next) {
                if (set_content->type != XML_ELEMENT_NODE)
                    continue;
                if (!strcmp((const char *)set_content->name, "includes")) {
                    if (set->include_path)
                        (void)free(set->include_path);

                    set->include_path = string_from_node(set_content);

                    if (!set->include_path) {
                        PROPAGATE_ERR();
                        (void)delete_ressource_set(set);
                        return (1);
                    }

                    set->include_path =  resolve_path(set->include_path, engine_root, "${ENGINE_ROOT}");

                    if (!set->include_path) {
                        PROPAGATE_ERR();
                        (void)delete_ressource_set(set);
                        return (1);
                    }
                } else if (!strcmp((const char *)set_content->name, "libs")) {
                    if (set->lib_path)
                        (void)free(set->lib_path);

                    set->lib_path = string_from_node(set_content);

                    if (!set->lib_path) {
                        PROPAGATE_ERR();
                        (void)delete_ressource_set(set);
                        return (1);
                    }

                    set->lib_path =  resolve_path(set->lib_path, engine_root, "${ENGINE_ROOT}");

                    if (!set->lib_path) {
                        PROPAGATE_ERR();
                        (void)delete_ressource_set(set);
                        return (1);
                    }
                } else if (!strcmp((const char *)set_content->name, "toolchain")) {
                    if (set->toolchain)
                        (void)free(set->toolchain);

                    set->toolchain = string_from_node(set_content);

                    if (!set->toolchain) {
                        PROPAGATE_ERR();
                        (void)delete_ressource_set(set);
                        return (1);
                    }

                    set->toolchain =  resolve_path(set->toolchain, engine_root, "${ENGINE_ROOT}");

                    if (!set->toolchain) {
                        PROPAGATE_ERR();
                        (void)delete_ressource_set(set);
                        return (1);
                    }
                } else {
                    RAISE_FMT(ERR_INVALID_TYPE, "invalid element '%s' in '%s'.", set_content->name, node_child->name);
                    (void)delete_ressource_set(set);
                    return (1);
                }
            }

            if (insert_generic_vector(&config->ressources, set)) {
                PROPAGATE_ERR();
                (void)delete_ressource_set(set);
                return (1);
            }
        } else {
            RAISE_FMT(ERR_INVALID_TYPE, "invalid element '%s' in '%s'.", node_child->name, node->name);
            return (1);
        }
    }

    return (0);
}

EngineConfig *parse_config_xml(const char *file_path, const char *engine_root)
{
    EngineConfig *parsed_data;
    xmlDoc *doc;
    xmlNode *root;

    doc = xmlReadFile(file_path, NULL, 0);

    if (!doc) {
        RAISE_FMT(ERR_OS, "failed to open and parse file '%s'.", file_path);
        return (NULL);
    }

    root = xmlDocGetRootElement(doc);

    if (strcmp((const char *)root->name, "engine")) {
        RAISE_FMT(ERR_INVALID_TYPE, "invalid element '%s' in '%s' expected 'engine'.", root->name, file_path);
        (void)xmlFreeDoc(doc);
        (void)xmlCleanupParser();
        return (NULL);
    }

    parsed_data = new_engine_config();

    if (!parsed_data) {
        PROPAGATE_ERR();
        (void)xmlFreeDoc(doc);
        (void)xmlCleanupParser();
        return (NULL);
    }

    for (xmlNode *node = root->children; node; node = node->next) {
        if (node->type != XML_ELEMENT_NODE)
            continue;
        if (!strcmp((const char *)node->name, "submodules")) {
            if (parsed_data->submodules_location)
                (void)free(parsed_data->submodules_location);

            parsed_data->submodules_location = string_from_node(node);

            if (!parsed_data->submodules_location) {
                PROPAGATE_ERR();
                (void)delete_engine_config(parsed_data);
                (void)xmlFreeDoc(doc);
                (void)xmlCleanupParser();
                return (NULL);
            }

            parsed_data->submodules_location =  resolve_path(parsed_data->submodules_location, engine_root, "${ENGINE_ROOT}");

            if (!parsed_data->submodules_location) {
                PROPAGATE_ERR();
                (void)delete_engine_config(parsed_data);
                (void)xmlFreeDoc(doc);
                (void)xmlCleanupParser();
                return (NULL);
            }

        } else if (!strcmp((const char *)node->name, "ressources")) {
            if (parse_config_ressources_xml(parsed_data, node, engine_root)) {
                PROPAGATE_ERR();
                (void)delete_engine_config(parsed_data);
                (void)xmlFreeDoc(doc);
                (void)xmlCleanupParser();
                return (NULL);
            }
        } else if (!strcmp((const char *)node->name, "generator")) {
            if (parse_config_generator_xml(parsed_data, node, engine_root)) {
                PROPAGATE_ERR();
                (void)delete_engine_config(parsed_data);
                (void)xmlFreeDoc(doc);
                (void)xmlCleanupParser();
                return (NULL);
            }
        } else {
            RAISE_FMT(ERR_INVALID_TYPE, "invalid element '%s' in '%s'.", node->name, root->name);
            (void)delete_engine_config(parsed_data);
            (void)xmlFreeDoc(doc);
            (void)xmlCleanupParser();
            return (NULL);
        }
    }

    (void)xmlFreeDoc(doc);
    (void)xmlCleanupParser();

    return (parsed_data);
}
