#include "object_toolchain.h"

void delete_object_element_data(struct object_element_s *obj_data)
{
    if (!obj_data) {
        RAISE(ERR_INVALID_POINTER, "can't delete empty object generation data.");
        return;
    }

    if (obj_data->id)
        (void)free(obj_data->id);
    if (obj_data->base)
        (void)free(obj_data->base);
    if (obj_data->attributes.content) {
        for (size_t i = 0; i < obj_data->attributes.size; ++i) {
            if (((OBJAttrib *)obj_data->attributes.content[i])->value.type == CN_TYPE_GENERIC_UNIQ_PTR)
                (void)free(((OBJAttrib *)obj_data->attributes.content[i])->value.as.ptr);
        }
        (void)empty_generic_vector(&obj_data->attributes, (expr_free)&delete_object_attribute);
    }
    if (obj_data->methods.content) {
        for (size_t i = 0; i < obj_data->methods.size; ++i) {
            (void)free(((OBJAttrib *)obj_data->methods.content[i])->value.as.ptr);
        }
        (void)empty_generic_vector(&obj_data->methods, (expr_free)&delete_object_attribute);
    }
    (void)free(obj_data);
}

uint8_t parse_object_element_methods_xml(xmlNode *node, struct object_element_s *obj_data)
{
    xmlChar *temp_s;
    OBJAttrib *temp;
    char *symbol;

    for (xmlNode *node_child = node->children; node_child; node_child = node_child->next) {
        if (node_child->type != XML_ELEMENT_NODE)
            continue;

        if (!strcmp((const char *)node_child->name, "symbol")) {
            symbol = string_from_node(node_child);

            if (!symbol) {
                PROPAGATE_ERR();
                return (1);
            }

            temp_s = xmlGetProp(node_child, (xmlChar *)"name");

            if (!temp_s) {
                RAISE_FMT(ERR_INVALID_POINTER, "all symbol should be attached to a name (%s).", symbol);
                (void)free(symbol);
                return (1);
            }

            temp = create_object_attribute(symbol, CN_TYPE_FUNCTION, (cnany)strdup((const char *)temp_s));

            (void)xmlFree(temp_s);
            (void)free(symbol);

            if (!temp) {
                PROPAGATE_ERR();
                return (1);
            }

            if (insert_generic_vector(&obj_data->methods, temp)) {
                PROPAGATE_ERR();
                (void)free(temp->value.as.ptr);
                (void)delete_object_attribute(temp);
                return (1);
            }
        } else {
            RAISE_FMT(ERR_INVALID_TYPE, "invalid element '%s' in '%s'.", node_child->name, node->name);
            return (1);
        }
    }

    return (0);
}

uint8_t parse_object_element_attributes_xml(xmlNode *node, struct object_element_s *obj_data)
{
    xmlChar *temp_s;
    cn_type type;
    OBJAttrib *temp;
    char *name;

    for (xmlNode *node_child = node->children; node_child; node_child = node_child->next) {
        if (node_child->type != XML_ELEMENT_NODE)
            continue;

        type = typename_from_string((const char *)node_child->name);
        name = string_from_node(node_child);

        if (!name) {
            PROPAGATE_ERR();
            return (1);
        }

        temp = create_object_attribute(name, type, NULL);

        (void)free(name);

        if (!temp) {
            PROPAGATE_ERR();
            return (1);
        }

        temp_s = xmlGetProp(node_child, (xmlChar *)"default");

        if (temp_s) {
            if (type != CN_TYPE_GENERIC_UNIQ_PTR) {
                if (value_from_string((const char *)temp_s, type, &temp->value)) {
                    PROPAGATE_ERR();
                    return (1);
                }
            } else {
                _init_attribute_value(&temp->value, strdup((const char *)temp_s));
            }
        }

        (void)xmlFree(temp_s);

        if (insert_generic_vector(&obj_data->attributes, temp)) {
            PROPAGATE_ERR();
            if (temp->value.type != CN_TYPE_GENERIC_UNIQ_PTR)
                delete_object_attribute(temp);
            else {
                (void)free(temp->value.as.ptr);
                temp->value.as.ptr = NULL;
                delete_object_attribute(temp);
            }
            return (1);
        }
    }

    return (0);
}

struct object_element_s *parse_object_element_data_xml(const char *object_path)
{
    struct object_element_s *obj_data;
    xmlDoc *doc;
    xmlNode *root;
    xmlChar *temp_s;

    doc = xmlReadFile(object_path, NULL, 0);

    if (!doc) {
        RAISE_FMT(ERR_OS, "failed to open and parse file '%s'.", object_path);
        return (NULL);
    }

    root = xmlDocGetRootElement(doc);

    if (strcmp((const char *)root->name, "object")) {
        RAISE_FMT(ERR_INVALID_TYPE, "invalid element '%s' in '%s'.", root->name, object_path);
        (void)xmlFreeDoc(doc);
        return (NULL);
    }

    obj_data = malloc(sizeof(struct object_element_s));

    if (!obj_data) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate object data.");
        (void)xmlFreeDoc(doc);
        return (NULL);
    }

    temp_s = xmlGetProp(root, (xmlChar *)"name");

    if (!temp_s) {
        obj_data->id = NULL;
    } else {
        obj_data->id = strdup((char *)temp_s);
        xmlFree(temp_s);
    }

    temp_s = xmlGetProp(root, (xmlChar *)"base");

    if (!temp_s) {
        obj_data->base = NULL;
    } else {
        obj_data->base = strdup((char *)temp_s);
        xmlFree(temp_s);
    }

    (void)init_generic_vector(&obj_data->attributes);
    (void)init_generic_vector(&obj_data->methods);

    for (xmlNode *node = root->children; node; node = node->next) {
        if (node->type != XML_ELEMENT_NODE)
            continue;
        if (!strcmp((const char *)node->name, "generate_in")) {
        } else if (!strcmp((const char *)node->name, "methods")) {
            if (parse_object_element_methods_xml(node, obj_data)) {
                PROPAGATE_ERR();
                (void)delete_object_element_data(obj_data);
                (void)xmlFreeDoc(doc);
                return (NULL);
            }
        } else if (!strcmp((const char *)node->name, "attrs")) {
            if (parse_object_element_attributes_xml(node, obj_data)) {
                PROPAGATE_ERR();
                (void)delete_object_element_data(obj_data);
                (void)xmlFreeDoc(doc);
                return (NULL);
            }
        } else {
            RAISE_FMT(ERR_INVALID_TYPE, "invalid element '%s' in '%s'.", node->name, root->name);
            (void)delete_object_element_data(obj_data);
            (void)xmlFreeDoc(doc);
            return (NULL);
        }
    }

    (void)xmlFreeDoc(doc);
    return (obj_data);
}

struct object_element_s *parse_object(const char *object_path, struct engine_object_file_writer_ctx_s *wctx, Object *asset_ctx)
{
    struct object_element_s *parsed_data;
    struct asset_registry *reg;

    reg = call_method(asset_ctx, "find_asset_by_name", PACK_ARG("object")).as.ptr;

    if (!reg) {
        PROPAGATE_ERR();
        return (NULL);
    }

    parsed_data = parse_object_element_data_xml(object_path);

    if (!parsed_data) {
        PROPAGATE_ERR();
        return (NULL);
    }

    if (writer_ctx_set_object_name(wctx, parsed_data->id ? parsed_data->id : "unamed_object")) {
        PROPAGATE_ERR();
        (void)delete_object_element_data(parsed_data);
        return (NULL);
    }

    for (size_t i = 0; i < reg->registered_sections.size; ++i) {
        if (section_registry_to_wctx_section(((struct section_registry *)reg->registered_sections.content[i])->name, parsed_data, reg->registered_sections.content[i], wctx)) {
            PROPAGATE_ERR();
            (void)delete_object_element_data(parsed_data);
            return (NULL);
        }
    }

    return (parsed_data);
}