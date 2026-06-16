#include "gui_toolchain.h"

void delete_parsed_gui(struct gui_element_s *parsed_gui)
{
    if (!parsed_gui)
        return;
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
    xmlChar *content;
    char *striped;
    size_t old_len = 0;
    uuid_t uuid;

    if (!element)
        return (1);
    
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
        (void)delete_parsed_gui(element);
        return (1);
    }

    element->object_type = strdup((const char *)node->name);

    if (!element->object_type) {
        (void)delete_parsed_gui(element);
        return (1);
    }

    (void)uuid_generate_random(uuid);
    (void)uuid_unparse_lower(uuid, element->id);

    for (xmlNode *node_child = node->children; node_child; node_child = node_child->next) {
        if (node_child->type == XML_TEXT_NODE) {
            content = xmlNodeGetContent(node_child);
            striped = strip_whitespace((const char *)content);
            if (!striped) {
                if (content)
                    (void)xmlFree(content);
                (void)delete_parsed_gui(element);
                return (1);
            }
            if (!strlen(striped)) {
                (void)free(striped);
                (void)xmlFree(content);
                continue;
            }
            if (element->text_content)
                old_len = strlen(element->text_content);
            else
                old_len = 0;
            element->text_content = realloc(element->text_content, sizeof(char) * (old_len + strlen((const char *)striped) + 1));

            if (!element->text_content) {
                (void)free(striped);
                (void)xmlFree(content);
                (void)delete_parsed_gui(element);
                return (1);
            }

            memcpy(element->text_content + old_len, striped, sizeof(char) * (strlen(striped) + 1));
            (void)free(striped);
            (void)xmlFree(content);
        }
    }

    return (0);
}

uint8_t build_gui_element(xmlNode *node, struct generic_vector_s *parsed_data, struct gui_element_s *parent)
{
    struct gui_element_s *temp = malloc(sizeof(struct gui_element_s));

    if (fill_gui_element(node, temp) || insert_generic_vector(parsed_data, temp)) {
        if (temp)
            (void)delete_parsed_gui(temp);
        return (1);
    }

    temp->parent._o = parent;
    temp->parent_loaded = true;

    for (xmlNode *node_child = node->children; node_child; node_child = node_child->next) {
        if (node_child->type == XML_ELEMENT_NODE)
            if (build_gui_element(node_child, parsed_data, temp))
                return (1);
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
        fprintf(stderr, "%s: failed to open and parse file.\n", file_path);
        return (NULL);
    }

    root = xmlDocGetRootElement(doc);

    if (strcmp((const char *)root->name, "gui")) {
        fprintf(stderr, "%s: invalid root element '%s', expecting 'gui'.\n", file_path, root->name);
        return (NULL);
    }

    temp_s = xmlGetProp(root, (xmlChar *)"name");
    if (writer_ctx_set_object_name(wctx, (const char *)temp_s)) {
        fprintf(stderr, "string allocation failed.\n");
        return (NULL);
    }
    xmlFree(temp_s);

    parsed_data = new_generic_vector();

    if (!parsed_data)
        return (NULL);

    for (xmlNode *node = root->children; node; node = node->next) {
        if (node->type != XML_ELEMENT_NODE)
            continue;
        if (!strcmp((const char *)node->name, "content")) {
            for (xmlNode *node_child = node->children; node_child; node_child = node_child->next) {
                if (node_child->type != XML_ELEMENT_NODE)
                    continue;
                if (build_gui_element(node_child, parsed_data, NULL)) {
                    delete_generic_vector(parsed_data, (expr_free)&delete_parsed_gui);
                    return (NULL);
                }
            }
        } else if (!strcmp((const char *)node->name, "connectors")) {
            for (xmlNode *node_child = node->children; node_child; node_child = node_child->next) {
                if (node_child->type != XML_ELEMENT_NODE)
                    continue;
            }
        } else {
            fprintf(stderr, "%s: invalid element '%s'.\n", file_path, root->name);
            delete_generic_vector(parsed_data, (expr_free)&delete_parsed_gui);
            return (NULL);
        }
    }

    (void)xmlFreeDoc(doc);
    (void)xmlCleanupParser();

    return (parsed_data);
}

struct generic_vector_s *parse_gui(const char *file_path, struct engine_object_file_writer_ctx_s *wctx, Object *asset_ctx)
{
    struct generic_vector_s *parsed_data;
    struct asset_registry *reg;

    reg = call_method(asset_ctx, "find_asset_by_name", PACK_ARG("gui")).as.ptr;

    if (!reg) {
        fprintf(stderr, "failed to fetch asset %s.\n", "gui");
        return (NULL);
    }

    parsed_data = parse_xml_gui(file_path, wctx);

    if (!parsed_data) {
        return (NULL);
    }

    for (size_t i = 0; i < reg->registered_sections.size; ++i) {
        if (section_registry_to_wctx_section(((struct section_registry *)reg->registered_sections.content[i])->name, parsed_data, reg->registered_sections.content[i], wctx)) {
            (void)delete_generic_vector(parsed_data, (expr_free)&delete_parsed_gui);
            return (NULL);
        }
    }

    return (parsed_data);
}