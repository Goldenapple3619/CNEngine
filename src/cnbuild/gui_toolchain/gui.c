#include "gui_toolchain.h"

void delete_parsed_gui(struct gui_element_s *parsed_gui)
{
    if (!parsed_gui) {
        RAISE(ERR_INVALID_POINTER, "can't delete empty gui element.");
        return;
    }
    if (parsed_gui->id)
        (void)free(parsed_gui->id);
    if (parsed_gui->object_type)
        (void)free(parsed_gui->object_type);
    if (parsed_gui->parent_loaded)
        parsed_gui->parent._o = NULL;
    else
        parsed_gui->parent._n = NULL;
    if (parsed_gui->text_content)
        (void)free(parsed_gui->text_content);
    if (parsed_gui->styles.content)
        (void)empty_generic_map(&parsed_gui->styles, &free);
    if (parsed_gui->connectors.content)
        (void)empty_generic_vector(&parsed_gui->connectors, &free);
    parsed_gui->parent_loaded = false;
    (void)free(parsed_gui);
}

uint8_t fill_gui_element(xmlNode *node, struct gui_element_s *element)
{
    uuid_t uuid;

    if (!element) {
        RAISE(ERR_INVALID_POINTER, "can't fill empty gui element.");
        return (1);
    }
    
    element->id = malloc(37);
    element->object_type = NULL;
    element->parent_loaded = true;
    element->parent._o = NULL;
    element->styles.keys = NULL;
    element->styles.content = NULL;
    element->styles.capacity = 0;
    element->styles.size = 0;
    element->text_content = NULL;
    element->connectors.content = NULL;
    element->connectors.capacity = 0;
    element->connectors.size = 0;

    if (!element->id) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate uuid.");
        (void)delete_parsed_gui(element);
        return (1);
    }

    element->object_type = strdup((const char *)node->name);

    if (!element->object_type) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate str object_type.");
        (void)delete_parsed_gui(element);
        return (1);
    }

    (void)uuid_generate_random(uuid);
    (void)uuid_unparse_lower(uuid, element->id);

    element->text_content = string_from_node(node);

    if (!element->text_content) {
        PROPAGATE_ERR();
        (void)delete_parsed_gui(element);
        return (1);
    }

    return (0);
}

uint8_t build_gui_element(xmlNode *node, struct generic_vector_s *parsed_data, struct gui_element_s *parent)
{
    struct gui_element_s *temp = malloc(sizeof(struct gui_element_s));

    if (!temp) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate gui element.");
        return (1);
    }

    if (fill_gui_element(node, temp) || insert_generic_vector(parsed_data, temp)) {
        PROPAGATE_ERR();
        (void)delete_parsed_gui(temp);
        return (1);
    }

    temp->parent._o = parent;
    temp->parent_loaded = true;

    for (xmlNode *node_child = node->children; node_child; node_child = node_child->next) {
        if (node_child->type != XML_ELEMENT_NODE)
            continue;
        if (build_gui_element(node_child, parsed_data, temp)) {
            PROPAGATE_ERR();
            return (1);
        }
    }

    return (0);
}

struct generic_vector_s *parse_xml_gui(const char *file_path, struct engine_object_file_writer_ctx_s *wctx)
{
    xmlDoc *doc;
    xmlNode *root;
    xmlChar *temp_s;
    struct generic_vector_s *parsed_data; 

    doc = xmlReadFile(file_path, NULL, 0);

    if (!doc) {
        RAISE_FMT(ERR_OS, "failed to open and parse file '%s'.", file_path);
        return (NULL);
    }

    root = xmlDocGetRootElement(doc);

    if (strcmp((const char *)root->name, "gui")) {
        RAISE_FMT(ERR_INVALID_TYPE, "invalid root type, expected: 'gui', got: '%s' in '%s'.", root->name, file_path);
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
            for (xmlNode *node_child = node->children; node_child; node_child = node_child->next) {
                if (node_child->type != XML_ELEMENT_NODE)
                    continue;
                if (build_gui_element(node_child, parsed_data, NULL)) {
                    PROPAGATE_ERR();
                    delete_generic_vector(parsed_data, (expr_free)&delete_parsed_gui);
                    (void)xmlFreeDoc(doc);
                    return (NULL);
                }
            }
        } else if (!strcmp((const char *)node->name, "connectors")) {
            for (xmlNode *node_child = node->children; node_child; node_child = node_child->next) {
                if (node_child->type != XML_ELEMENT_NODE)
                    continue;
            }
        } else {
            RAISE_FMT(ERR_INVALID_TYPE, "invalid node type for: '%s', got: '%s'.", node->name, root->name);
            delete_generic_vector(parsed_data, (expr_free)&delete_parsed_gui);
            (void)xmlFreeDoc(doc);
            return (NULL);
        }
    }

    (void)xmlFreeDoc(doc);
    return (parsed_data);
}

struct generic_vector_s *parse_gui(const char *file_path, struct engine_object_file_writer_ctx_s *wctx, Object *asset_ctx)
{
    struct generic_vector_s *parsed_data;
    struct asset_registry *reg;

    reg = call_method(asset_ctx, "find_asset_by_name", PACK_ARG("gui")).as.ptr;

    if (!reg) {
        PROPAGATE_ERR();
        return (NULL);
    }

    parsed_data = parse_xml_gui(file_path, wctx);

    if (!parsed_data) {
        PROPAGATE_ERR();
        return (NULL);
    }

    for (size_t i = 0; i < reg->registered_sections.size; ++i) {
        if (section_registry_to_wctx_section(((struct section_registry *)reg->registered_sections.content[i])->name, parsed_data, reg->registered_sections.content[i], wctx)) {
            PROPAGATE_ERR();
            (void)delete_generic_vector(parsed_data, (expr_free)&delete_parsed_gui);
            return (NULL);
        }
    }

    return (parsed_data);
}