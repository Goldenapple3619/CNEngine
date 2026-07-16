#include "../project_toolchain.h"

SubModule *new_submodule(void)
{
    SubModule *submodule = malloc(sizeof(SubModule));

    if (!submodule) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new submodule.");
        return (NULL);
    }

    memset(submodule, 0, sizeof(SubModule));
    return (submodule);
}

void delete_submodule_includes(SubModuleInclude *sminc)
{
    if (!sminc) {
        RAISE(ERR_INVALID_POINTER, "can't delete empty submoduleinclude.");
        return;
    }

    if (sminc->path)
        (void)free(sminc->path);

    (void)free(sminc);
}

void delete_submodule_libs(SubModuleLib *smlib)
{
    if (!smlib) {
        RAISE(ERR_INVALID_POINTER, "can't delete empty submodulelib.");
        return;
    }

    if (smlib->path)
        (void)free(smlib->path);
    if (smlib->name)
        (void)free(smlib->name);

    (void)free(smlib);
}

void clear_submodule_datas(SubModule *submodule)
{
    if (!submodule) {
        RAISE(ERR_INVALID_POINTER, "can't clear empty submodule data.");
        return;
    }

    if (submodule->name)
        (void)free(submodule->name);

    if (submodule->need.content)
        (void)empty_generic_vector(&submodule->need, &free);

    if (submodule->libs.content)
        (void)empty_generic_vector(&submodule->libs, (expr_free)&delete_submodule_libs);

    if (submodule->includes.content)
        (void)empty_generic_vector(&submodule->includes, (expr_free)&delete_submodule_includes);

    (void)memset(submodule, 0, sizeof(SubModule));
}

void delete_submodule(SubModule *submodule)
{
    if (!submodule) {
        RAISE(ERR_INVALID_POINTER, "can't delete empty submodule.");
        return;
    }
    (void)clear_submodule_datas(submodule);
    (void)free(submodule);
}

uint8_t parse_submodule_includes_xml(SubModule *submodule, xmlNode *node)
{
    SubModuleInclude *temp;
    xmlChar *temp_s;

    for (xmlNode *node_child = node->children; node_child; node_child = node_child->next) {
        if (node_child->type != XML_ELEMENT_NODE)
            continue;

        if (!strcmp((const char *)node_child->name, "dir")) {
            temp = malloc(sizeof(SubModuleInclude));

            if (!temp) {
                RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new SubModuleInclude.");
                return (1);
            }

            temp->isdir = true;
        } else if (!strcmp((const char *)node_child->name, "file")) {
            temp = malloc(sizeof(SubModuleInclude));

            if (!temp) {
                RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new SubModuleInclude.");
                return (1);
            }

            temp->isdir = false;
        } else {
            RAISE_FMT(ERR_INVALID_TYPE, "invalid element '%s' in '%s'.", node_child->name, node->name);
            return (1);
        }

        temp->path = string_from_node(node_child);

        if (!temp->path) {
            PROPAGATE_ERR();
            (void)delete_submodule_includes(temp);
            return (1);
        }

        temp_s = xmlGetProp(node_child, (xmlChar *)"error");

        if (!temp_s) {
            temp->skip_error = false;
        } else {
            temp->skip_error = (!strcmp((char *)temp_s, "skip") ? true : false);
            xmlFree(temp_s);
        }

        temp_s = xmlGetProp(node_child, (xmlChar *)"overwrite");

        if (!temp_s) {
            temp->overwrite = false;
        } else {
            temp->overwrite = (!strcmp((char *)temp_s, "true") ? true : false);
            xmlFree(temp_s);
        }

        if (insert_generic_vector(&submodule->includes, temp)) {
            PROPAGATE_ERR();
            (void)delete_submodule_includes(temp);
            return (1);
        }
    }

    return (0);
}

uint8_t parse_submodule_libs_xml(SubModule *submodule, xmlNode *node)
{
    SubModuleLib *temp;
    xmlChar *temp_s;

    for (xmlNode *node_child = node->children; node_child; node_child = node_child->next) {
        if (node_child->type != XML_ELEMENT_NODE)
            continue;

        if (!strcmp((const char *)node_child->name, "dir")) {
            temp = malloc(sizeof(SubModuleLib));

            if (!temp) {
                RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new SubModuleLib.");
                return (1);
            }

            temp->isdir = true;
        } else if (!strcmp((const char *)node_child->name, "file")) {
            temp = malloc(sizeof(SubModuleLib));

            if (!temp) {
                RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new SubModuleLib.");
                return (1);
            }

            temp->isdir = false;
        } else {
            RAISE_FMT(ERR_INVALID_TYPE, "invalid element '%s' in '%s'.", node_child->name, node->name);
            return (1);
        }

        temp->path = string_from_node(node_child);

        if (!temp->path) {
            PROPAGATE_ERR();
            (void)delete_submodule_libs(temp);
            return (1);
        }

        temp_s = xmlGetProp(node_child, (xmlChar *)"error");

        if (!temp_s) {
            temp->skip_error = false;
        } else {
            temp->skip_error = (!strcmp((char *)temp_s, "skip") ? true : false);
            xmlFree(temp_s);
        }

        temp_s = xmlGetProp(node_child, (xmlChar *)"overwrite");

        if (!temp_s) {
            temp->overwrite = false;
        } else {
            temp->overwrite = (!strcmp((char *)temp_s, "true") ? true : false);
            xmlFree(temp_s);
        }

        temp_s = xmlGetProp(node_child, (xmlChar *)"link");

        if (!temp_s) {
            temp->link = false;
        } else {
            temp->link = (!strcmp((char *)temp_s, "true") ? true : false);
            xmlFree(temp_s);
        }

        temp_s = xmlGetProp(node_child, (xmlChar *)"name");

        if (!temp_s) {
            temp->name = NULL;
        } else {
            temp->name = strdup((char *)temp_s);
            xmlFree(temp_s);

            if (!temp->name) {
                RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new SubModuleLibrary name.");
                (void)delete_submodule_libs(temp);
                return (1);
            }
        }

        if (insert_generic_vector(&submodule->libs, temp)) {
            PROPAGATE_ERR();
            (void)delete_submodule_libs(temp);
            return (1);
        }
    }

    return (0);
}

uint8_t parse_submodules_xml(SubModule *submodule, const char *xml_path)
{
    if (!submodule) {
        RAISE(ERR_INVALID_POINTER, "can't put parsed submodule data into nothing.");
        return (1);
    }

    if (!xml_path) {
        RAISE(ERR_INVALID_POINTER, "can't parse empty xml file.");
        return (1);
    }

    (void)clear_submodule_datas(submodule);

    xmlDoc *doc;
    xmlNode *root;
    char *temp_str;

    doc = xmlReadFile(xml_path, NULL, 0);

    if (!doc) {
        RAISE_FMT(ERR_OS, "failed to open and parse file '%s'.", xml_path);
        return (1);
    }

    root = xmlDocGetRootElement(doc);

    if (strcmp((const char *)root->name, "module")) {
        RAISE_FMT(ERR_INVALID_TYPE, "invalid element '%s' in '%s'.", root->name, xml_path);
        (void)xmlFreeDoc(doc);
        (void)xmlCleanupParser();
        return (1);
    }

    for (xmlNode *node = root->children; node; node = node->next) {
        if (node->type != XML_ELEMENT_NODE)
            continue;
        if (!strcmp((const char *)node->name, "name")) {
            if (submodule->name)
                (void)free(submodule->name);

            submodule->name = string_from_node(node);

            if (!submodule->name) {
                PROPAGATE_ERR();
                (void)xmlFreeDoc(doc);
                (void)xmlCleanupParser();
                return (1);
            }
        } else if (!strcmp((const char *)node->name, "includes")) {
            if (parse_submodule_includes_xml(submodule, node)) {
                PROPAGATE_ERR();
                (void)xmlFreeDoc(doc);
                (void)xmlCleanupParser();
                return (1);
            }
        } else if (!strcmp((const char *)node->name, "libs")) {
            if (parse_submodule_libs_xml(submodule, node)) {
                PROPAGATE_ERR();
                (void)xmlFreeDoc(doc);
                (void)xmlCleanupParser();
                return (1);
            }
        } else if (!strcmp((const char *)node->name, "need")) {
            for (xmlNode *node_child = node->children; node_child; node_child = node_child->next) {
                if (node_child->type != XML_ELEMENT_NODE)
                    continue;
                    
                if (!strcmp((const char *)node_child->name, "module")) {
                    temp_str = string_from_node(node);

                    if (!temp_str) {
                        PROPAGATE_ERR();
                        (void)xmlFreeDoc(doc);
                        (void)xmlCleanupParser();
                        return (1);
                    }

                    if (insert_generic_vector(&submodule->need, temp_str)) {
                        PROPAGATE_ERR();
                        (void)free(temp_str);
                        (void)xmlFreeDoc(doc);
                        (void)xmlCleanupParser();
                        return (1);
                    };
                } else {
                    RAISE_FMT(ERR_INVALID_TYPE, "invalid element '%s' in '%s'.", node_child->name, node->name);
                    (void)xmlFreeDoc(doc);
                    (void)xmlCleanupParser();
                    return (1);
                }
            }
        } else {
            RAISE_FMT(ERR_INVALID_TYPE, "invalid element '%s' in '%s'.", node->name, root->name);
            (void)xmlFreeDoc(doc);
            (void)xmlCleanupParser();
            return (1);
        }
    }

    (void)xmlFreeDoc(doc);
    (void)xmlCleanupParser();

    return (0);
}
