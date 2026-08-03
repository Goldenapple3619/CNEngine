

#include "scene_toolchain.h"

void delete_parsed_scene(struct scene_element_s *parsed_object)
{
    if (!parsed_object) {
        RAISE(ERR_INVALID_POINTER, "can't delete empty element");
        return;
    }
    if (parsed_object->id)
        (void)free(parsed_object->id);
    if (parsed_object->object_type)
        (void)free(parsed_object->object_type);
    if (parsed_object->parent_loaded)
        parsed_object->parent._o = NULL;
    else
        parsed_object->parent._n = NULL;
    if (parsed_object->attributes.content) {
        for (size_t i = 0; i < parsed_object->attributes.size; ++i) {
            if (((OBJAttrib *)parsed_object->attributes.content[i])->value.type == CN_TYPE_GENERIC_UNIQ_PTR)
                (void)free(((OBJAttrib *)parsed_object->attributes.content[i])->value.as.ptr);
        }
        (void)empty_generic_vector(&parsed_object->attributes, (expr_free)&delete_object_attribute);
    }
    parsed_object->parent_loaded = false;
    (void)free(parsed_object);
}

uint8_t fill_scene_element(xmlNode *node, struct scene_element_s *element)
{
    uuid_t uuid;
    xmlChar *temp_s;

    if (!element) {
        RAISE(ERR_INVALID_POINTER, "can't init empty element");
        return (1);
    }
    
    element->id = malloc(37);
    element->object_type = NULL;
    element->parent_loaded = true;
    element->parent._o = NULL;
    element->attributes.content = NULL;
    element->attributes.capacity = 0;
    element->attributes.size = 0;

    if (!element->id) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new uuid.");
        (void)delete_parsed_scene(element);
        return (1);
    }

    temp_s = xmlGetProp(node, (xmlChar *)"type");
    element->object_type = strdup((const char *)temp_s);
    xmlFree(temp_s);

    if (!element->object_type) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new type string.");
        (void)delete_parsed_scene(element);
        return (1);
    }

    (void)uuid_generate_random(uuid);
    (void)uuid_unparse_lower(uuid, element->id);

    return (0);
}

uint8_t build_element_attribs(xmlNode *node, struct scene_element_s *element)
{
    xmlChar *temp_s;
    cn_type type;
    OBJAttrib *temp;
    char *text;

    for (xmlNode *node_child = node->children; node_child; node_child = node_child->next) {
        if (node_child->type != XML_ELEMENT_NODE)
            continue;

        temp_s = xmlGetProp(node, (xmlChar *)"as");

        type = typename_from_string((const char *)node_child->name);
        temp = create_object_attribute((const char *)temp_s, type, NULL);

        xmlFree(temp_s);

        if (!temp) {
            PROPAGATE_ERR();
            return (1);
        }

        text = string_from_node(node);

        if (!text) {
            PROPAGATE_ERR();
            return (1);
        }

        if (type != CN_TYPE_GENERIC_UNIQ_PTR) {
            if (value_from_string(text, type, &temp->value)) {
                PROPAGATE_ERR();
                return (1);
            }
        } else {
            _init_attribute_value(&temp->value, strdup(text));
        }

        (void)free(text);

        if (insert_generic_vector(&element->attributes, temp)) {
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

uint8_t build_scene_element(xmlNode *node, struct generic_vector_s *parsed_data, struct scene_element_s *parent)
{
    if (strcmp((const char *)node->name, "object")) {
        RAISE_FMT(ERR_INVALID_TYPE, "invalid node type, expected: 'object', got: '%s'.", (const char *)node->name);
        return (1);
    }

    struct scene_element_s *temp = malloc(sizeof(struct scene_element_s));

    if (!temp) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate scene element.");
        return (1);
    }

    if (fill_scene_element(node, temp) || insert_generic_vector(parsed_data, temp)) {
        PROPAGATE_ERR();
        if (temp)
            (void)delete_parsed_scene(temp);
        return (1);
    }

    temp->parent._o = parent;
    temp->parent_loaded = true;

    for (xmlNode *node_child = node->children; node_child; node_child = node_child->next) {
        if (node_child->type != XML_ELEMENT_NODE)
            continue;

        if (!strcmp((const char *)node_child->name, "name")) {
            if (temp->id) {
                (void)free(temp->id);
                temp->id = NULL;
            }

            temp->id = string_from_node(node_child);

            if (!temp->id) {
                PROPAGATE_ERR();
                return (1);
            }

        } else if (!strcmp((const char *)node_child->name, "attrs")) {
            if (build_element_attribs(node_child, temp)) {
                PROPAGATE_ERR();
                return (1);
            }
        } else if (!strcmp((const char *)node_child->name, "childs")) {
            for (xmlNode *subnode = node_child->children; subnode; subnode = subnode->next) {
                if (subnode->type != XML_ELEMENT_NODE)
                    continue;
                if (build_scene_element(subnode, parsed_data, temp)) {
                    PROPAGATE_ERR();
                    return (1);
                }
            }
        } else {
            RAISE_FMT(ERR_INVALID_TYPE, "invalid node type for: '%s', got: '%s'.", node->name, node_child->name);
            return (1);
        }
    }

    return (0);
}

struct generic_vector_s *parse_xml_scene(const char *file_path, struct engine_object_file_writer_ctx_s *wctx)
{
    xmlDoc *doc;
    xmlNode *root;
    struct generic_vector_s *parsed_data; 
    xmlChar *temp_s;

    doc = xmlReadFile(file_path, NULL, 0);

    if (!doc) {
        RAISE_FMT(ERR_OS, "failed to open and parse file '%s'.", file_path);
        return (NULL);
    }

    root = xmlDocGetRootElement(doc);

    if (strcmp((const char *)root->name, "scene")) {
        RAISE_FMT(ERR_INVALID_TYPE, "invalid root type, expected: 'scene', got: '%s' in '%s'.", root->name, file_path);
        (void)xmlFreeDoc(doc);
        return (NULL);
    }

    temp_s = xmlGetProp(root, (xmlChar *)"name");
    if (writer_ctx_set_object_name(wctx, (const char *)temp_s)) {
        PROPAGATE_ERR();
        (void)xmlFreeDoc(doc);
        return (NULL);
    }
    xmlFree(temp_s);

    parsed_data = new_generic_vector();

    if (!parsed_data) {
        PROPAGATE_ERR();
        (void)xmlFreeDoc(doc);
        return (NULL);
    }

    for (xmlNode *node = root->children; node; node = node->next) {
        if (node->type != XML_ELEMENT_NODE)
            continue;

        if (!strcmp((const char *)node->name, "content")) {

        } else if (!strcmp((const char *)node->name, "details")) {

        } else {
            RAISE_FMT(ERR_INVALID_TYPE, "invalid node type for: '%s', got: '%s'.", node->name, root->name);
            delete_generic_vector(parsed_data, (expr_free)&delete_parsed_scene);
            (void)xmlFreeDoc(doc);
            return (NULL);
        }
    }

    (void)xmlFreeDoc(doc);
    return (parsed_data);
}

struct generic_vector_s *parse_scene(const char *file_path, struct engine_object_file_writer_ctx_s *wctx, Object *asset_ctx)
{
    struct generic_vector_s *parsed_data;
    struct asset_registry *reg;

    reg = call_method(asset_ctx, "find_asset_by_name", PACK_ARG("scene")).as.ptr;

    if (!reg) {
        PROPAGATE_ERR();
        return (NULL);
    }

    parsed_data = parse_xml_scene(file_path, wctx);

    if (!parsed_data) {
        PROPAGATE_ERR();
        return (NULL);
    }

    for (size_t i = 0; i < reg->registered_sections.size; ++i) {
        if (section_registry_to_wctx_section(((struct section_registry *)reg->registered_sections.content[i])->name, parsed_data, reg->registered_sections.content[i], wctx)) {
            PROPAGATE_ERR();
            (void)delete_generic_vector(parsed_data, (expr_free)&delete_parsed_scene);
            return (NULL);
        }
    }

    return (parsed_data);
}