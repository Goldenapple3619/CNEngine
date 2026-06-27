

#include "scene_toolchain.h"

void delete_parsed_scene(struct scene_element_s *parsed_object)
{
    if (!parsed_object)
        return;
    if (parsed_object->id)
        (void)free(parsed_object->id);
    if (parsed_object->object_type)
        (void)free(parsed_object->object_type);
    if (parsed_object->parent_loaded)
        parsed_object->parent._o = NULL;
    else
        parsed_object->parent._n = NULL;
    if (parsed_object->attributes.content)
        (void)empty_generic_vector(&parsed_object->attributes, (expr_free)&delete_object_attribute);
    parsed_object->parent_loaded = false;
    (void)free(parsed_object);
}

struct generic_vector_s *parse_xml_scene(const char *file_path, struct engine_object_file_writer_ctx_s *wctx)
{
    xmlDoc *doc;
    xmlNode *root;
    struct generic_vector_s *parsed_data; 
    xmlChar *temp_s;

    doc = xmlReadFile(file_path, NULL, 0);

    if (!doc) {
        fprintf(stderr, "%s: failed to open and parse file.\n", file_path);
        return (NULL);
    }

    root = xmlDocGetRootElement(doc);

    if (strcmp((const char *)root->name, "scene")) {
        fprintf(stderr, "%s: invalid root element '%s', expecting 'scene'.\n", file_path, root->name);
        (void)xmlFreeDoc(doc);
        (void)xmlCleanupParser();
        return (NULL);
    }

    temp_s = xmlGetProp(root, (xmlChar *)"name");
    if (writer_ctx_set_object_name(wctx, (const char *)temp_s)) {
        fprintf(stderr, "string allocation failed.\n");
        (void)xmlFreeDoc(doc);
        (void)xmlCleanupParser();
        return (NULL);
    }
    xmlFree(temp_s);

    parsed_data = new_generic_vector();

    if (!parsed_data) {
        (void)xmlFreeDoc(doc);
        (void)xmlCleanupParser();
        return (NULL);
    }

    for (xmlNode *node = root->children; node; node = node->next) {
        if (node->type != XML_ELEMENT_NODE)
            continue;
    }

    (void)xmlFreeDoc(doc);
    (void)xmlCleanupParser();
    return (parsed_data);
}

struct generic_vector_s *parse_scene(const char *file_path, struct engine_object_file_writer_ctx_s *wctx, Object *asset_ctx)
{
    struct generic_vector_s *parsed_data;
    struct asset_registry *reg;

    reg = call_method(asset_ctx, "find_asset_by_name", PACK_ARG("scene")).as.ptr;

    if (!reg) {
        fprintf(stderr, "failed to fetch asset %s.\n", "scene");
        return (NULL);
    }

    parsed_data = parse_xml_scene(file_path, wctx);

    if (!parsed_data) {
        return (NULL);
    }

    for (size_t i = 0; i < reg->registered_sections.size; ++i) {
        if (section_registry_to_wctx_section(((struct section_registry *)reg->registered_sections.content[i])->name, parsed_data, reg->registered_sections.content[i], wctx)) {
            (void)delete_generic_vector(parsed_data, (expr_free)&delete_parsed_scene);
            return (NULL);
        }
    }

    return (parsed_data);
}