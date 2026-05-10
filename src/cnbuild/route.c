#include "build.h"

static char *strip_whitespace(const char *str) {
    char *result;
    size_t len;

    if (!str)
        return NULL;

    while (isspace((unsigned char)*str))
        str++;

    if (*str == '\0')
        return (strdup(""));

    const char *end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end))
        end--;

    len = end - str + 1;
    result = malloc(len + 1);
    if (!result)
        return (NULL);

    memcpy(result, str, len);
    result[len] = '\0';
    return (result);
}

void delete_parsed_gui(struct parsed_gui_element_data_s *parsed_gui)
{
    if (!parsed_gui)
        return;
    if (parsed_gui->id)
        (void)free(parsed_gui->id);
    if (parsed_gui->object_type)
        (void)free(parsed_gui->object_type);
    if (parsed_gui->parent)
        parsed_gui->parent = NULL;
    if (parsed_gui->text_content)
        (void)free(parsed_gui->text_content);
    if (parsed_gui->styles.content) {
        for (size_t i = 0; i < parsed_gui->styles.size; ++i)
            (void)free(parsed_gui->styles.content[i]);
        (void)free(parsed_gui->styles.content);
    }
    (void)free(parsed_gui);
}

uint8_t fill_gui_element(xmlNode *node, struct parsed_gui_element_data_s *element)
{
    xmlChar *content;
    char *striped;
    size_t old_len = 0;
    uuid_t uuid;

    if (!element)
        return (1);
    
    element->id = malloc(37);
    element->object_type = NULL;
    element->styles.keys = NULL;
    element->styles.content = NULL;
    element->styles.capacity = 0;
    element->styles.size = 0;
    element->text_content = NULL;

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

uint64_t generate_gui_node_section_size(struct engine_object_file_section_writer_ctx_s *self)
{
    return ((sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t)) * ((struct generic_vector_s *)self->_v)->size);
}

char *generate_gui_node_section_content(struct engine_object_file_section_writer_ctx_s *self, const struct engine_object_file_writer_ctx_s *wctx, struct generic_map_s *strndx)
{
    struct generic_vector_s *vec = self->_v;
    struct parsed_gui_element_data_s *temp;
    char *generated = malloc(sizeof(char) * generate_gui_node_section_size(self));
    void (*writter_u32)(char *, uint32_t) = wctx->write_infos.endian == ENGINE_WRT_LITTLE_ENDIAN ? &bufwr_u32_le : &bufwr_u32_be;
    uint64_t pos = 0;

    if (!generated)
        return (NULL);

    for (size_t i = 0; i < vec->size; ++i) {
        temp = vec->content[i];

        if (!temp->parent)
            writter_u32(generated + pos, add_str_table("__main_element", strndx));
        else
            writter_u32(generated + pos, add_str_table(temp->parent->id, strndx));
        pos += sizeof(uint32_t);
        writter_u32(generated + pos, add_str_table(temp->id, strndx));
        pos += sizeof(uint32_t);
        writter_u32(generated + pos, add_str_table(temp->object_type, strndx));
        pos += sizeof(uint32_t);
        if (temp->text_content)
            writter_u32(generated + pos, add_str_table(temp->text_content, strndx));
        else
            writter_u32(generated + pos, add_str_table("", strndx));
        pos += sizeof(uint32_t);
    }
    return (generated);
}


uint8_t element_gui_to_wctx(struct generic_vector_s *element, struct engine_object_file_writer_ctx_s *wctx)
{
    struct engine_object_file_section_writer_ctx_s *node_section;
    // struct engine_object_file_section_writer_ctx_s *style_section;

    node_section = new_writer_section("gui_nodes", element);
    if (!node_section)
        return (1);
    if (writer_ctx_add_section(wctx, node_section)) {
        delete_writer_section(node_section);
        return (1);
    }

    // style_section = new_writer_section(NULL, element);
    // if (!style_section)
    //     return (1);
    // if (writer_ctx_add_section(wctx, style_section)) {
    //     delete_writer_section(style_section);
    //     return (1);
    // }

    node_section->content_generator = &generate_gui_node_section_content;
    node_section->content_size_generator = &generate_gui_node_section_size;

    return (0);
}

uint8_t build_gui_element(xmlNode *node, struct generic_vector_s *parsed_data, struct parsed_gui_element_data_s *parent)
{
    struct parsed_gui_element_data_s *temp = malloc(sizeof(struct parsed_gui_element_data_s));

    if (fill_gui_element(node, temp) || insert_generic_vector(parsed_data, temp)) {
        if (temp)
            (void)delete_parsed_gui(temp);
        return (1);
    }

    temp->parent = parent;

    for (xmlNode *node_child = node->children; node_child; node_child = node_child->next) {
        if (node_child->type == XML_ELEMENT_NODE)
            if (build_gui_element(node_child, parsed_data, temp))
                return (1);
    }

    return (0);
}

uint8_t parse_gui(const char *file_path, struct engine_object_file_writer_ctx_s *wctx)
{
    struct generic_vector_s *parsed_data = new_generic_vector(); 
    xmlDoc *doc;
    xmlNode *root;
    xmlChar *temp_s;

    if (!parsed_data)
        return (1);

    doc = xmlReadFile(file_path, NULL, 0);

    if (!doc) {
        fprintf(stderr, "%s: failed to open and parse file.\n", file_path);
        return (1);
    }

    root = xmlDocGetRootElement(doc);

    if (strcmp((const char *)root->name, "gui")) {
        fprintf(stderr, "%s: invalid root element '%s', expecting 'gui'.\n", file_path, root->name);
        return (1);
    }

    temp_s = xmlGetProp(root, (xmlChar *)"name");
    if (writer_ctx_set_object_name(wctx, (const char *)temp_s)) {
        fprintf(stderr, "string allocation failed.\n");
        return (1);
    }
    xmlFree(temp_s);

    for (xmlNode *node = root->children; node; node = node->next) {
        if (node->type != XML_ELEMENT_NODE)
            continue;
        if (!strcmp((const char *)node->name, "content")) {
            for (xmlNode *node_child = node->children; node_child; node_child = node_child->next) {
                if (node_child->type != XML_ELEMENT_NODE)
                    continue;
                if (build_gui_element(node_child, parsed_data, NULL)) {
                    delete_generic_vector(parsed_data, (void(*)(void *))&delete_parsed_gui);
                    return (1);
                }
            }
        } else if (!strcmp((const char *)node->name, "connectors")) {
            for (xmlNode *node_child = node->children; node_child; node_child = node_child->next) {
                if (node_child->type != XML_ELEMENT_NODE)
                    continue;
            }
        } else {
            fprintf(stderr, "%s: invalid element '%s'.\n", file_path, root->name);
            delete_generic_vector(parsed_data, (void(*)(void *))&delete_parsed_gui);
            return (1);
        }
    }

    (void)xmlFreeDoc(doc);
    (void)xmlCleanupParser();

    if (element_gui_to_wctx(parsed_data, wctx)) {
        delete_generic_vector(parsed_data, (void(*)(void *))&delete_parsed_gui);
        return (1);
    }

    return (0);
}

uint8_t build_get_args(size_t argc, char **argv, struct build_args_s *args)
{
    if (!args)
        return (0);
    args->output_file = strdup("output.cno");
    if (!args->output_file) {
        fprintf(stderr, "failed to allocate string while parsing args.\n");
        return (1);
    }
    args->input_files.capacity = 0;
    args->input_files.size = 0;
    args->padding = ENGINE_WRT_ALIGN8_FLAG;
    args->endian = ENGINE_WRT_BIG_ENDIAN;

    for (size_t i = 0; i < argc; ++i) {
        if (strstr(argv[i], "--align=") == argv[i]) {
            switch (atoi(argv[i] + strlen("--align="))) {
                case 0:
                case 1:
                    args->padding = ENGINE_WRT_ALIGN1_FLAG;
                    break;
                case 4:
                    args->padding = ENGINE_WRT_ALIGN4_FLAG;
                    break;
                case 8:
                    args->padding = ENGINE_WRT_ALIGN8_FLAG;
                    break;
                case 16:
                    args->padding = ENGINE_WRT_ALIGN16_FLAG;
                    break;
                case 64:
                    args->padding = ENGINE_WRT_ALIGN64_FLAG;
                    break;
                case 4096:
                    args->padding = ENGINE_WRT_ALIGN4096_FLAG;
                    break;
                default:
                    fprintf(stderr, "invalid alignement specified '%s'.\n", argv[i] + strlen("--align="));
                    return (1);
            }

            continue;
        }

        if (strstr(argv[i], "--endian=") == argv[i]) {
            if (!strcmp(argv[i] + strlen("--endian="), "big"))
                args->endian = ENGINE_WRT_BIG_ENDIAN;
            else if (!strcmp(argv[i] + strlen("--endian="), "little"))
                args->endian = ENGINE_WRT_LITTLE_ENDIAN;
            else {
                fprintf(stderr, "invalid endian provided '%s'.\n", argv[i] + strlen("--endian="));
                return (1);
            }

            continue;
        }

        if (!strcmp(argv[i], "-o")) {
            ++i;
            args->output_file = strdup(argv[i]);

            if (!args->output_file) {
                fprintf(stderr, "failed to allocate string while parsing args.\n");
                return (1);
            }

            continue;
        }

        if (insert_generic_vector(&args->input_files, argv[i])) {
            fprintf(stderr, "failed to allocate vector while parsing args.\n");
            return (1);
        }
    }

    return (0);
}

void reset_args(struct build_args_s *args)
{
    if (args->output_file)
        (void)free(args->output_file);
    if (args->input_files.content)
        (void)free(args->input_files.content);
    args->endian = ENGINE_WRT_BIG_ENDIAN;
    args->padding = ENGINE_WRT_ALIGN8_FLAG;
    args->output_file = NULL;
    args->input_files.content = NULL;
    args->input_files.capacity = 0;
    args->input_files.size = 0;
}

void delete_parsed_data(struct engine_object_file_writer_ctx_s *wctx)
{
    struct generic_vector_s *parsed_data;

    for (size_t i = 0; i < wctx->sections.size; ++i) {
        if (!((struct engine_object_file_section_writer_ctx_s *)wctx->sections.content[i])->_v)
            continue;
        if (((struct engine_object_file_section_writer_ctx_s *)wctx->sections.content[i])->write_infos.type == ENGINE_SEC_GUI_NODES) {
            parsed_data = ((struct engine_object_file_section_writer_ctx_s *)wctx->sections.content[i])->_v;

            for (size_t j = 0; j < parsed_data->size; ++j)
                (void)delete_parsed_gui(parsed_data->content[j]);
            (void)free(parsed_data->content);
        }
    }
}

int build_gui(size_t argc, char **argv)
{
    struct engine_object_file_writer_ctx_s *wctx;
    struct build_args_s build_args = {0};
    FILE *fp;

    if (build_get_args(argc - 3, argv + 3, &build_args)) {
        (void)reset_args(&build_args);
        return (1);
    }

    if (build_args.input_files.size == 0) {
        fprintf(stderr, "missing input file.\n");
        (void)reset_args(&build_args);
        return (1);
    }

    if (build_args.input_files.size > 1) {
        fprintf(stderr, "too much input files.\n");
        (void)reset_args(&build_args);
        return (1);
    }

    wctx = new_writer_ctx(NULL);

    if (!wctx) {
        fprintf(stderr, "writter ctx allocation failed.\n");
        (void)reset_args(&build_args);
        return (1);
    }

    wctx->write_infos.endian = build_args.endian;
    wctx->write_infos.flags = build_args.padding;

    if (parse_gui(build_args.input_files.content[0], wctx)) {
        (void)delete_parsed_data(wctx);
        (void)delete_writer_ctx(wctx);
        (void)reset_args(&build_args);
        return (1);
    }

    fp = fopen(build_args.output_file, "w");

    if (!fp) {
        fprintf(stderr, "%s: failed to open output file.\n", build_args.output_file);
        (void)delete_parsed_data(wctx);
        (void)delete_writer_ctx(wctx);
        (void)reset_args(&build_args);
        return (1);
    }
    
    if (write_object_file(fp, wctx)) {
        fprintf(stderr, "%s: failed to write output file.\n", build_args.output_file);
        (void)delete_parsed_data(wctx);
        (void)delete_writer_ctx(wctx);
        (void)reset_args(&build_args);
        (void)fclose(fp);
        return (1);
    }

    (void)fclose(fp);
    (void)delete_writer_ctx(wctx);
    (void)reset_args(&build_args);

    return (0);
}

int build(size_t argc, char **argv)
{
    size_t i = 0;

    if (argc < 3) {
        fprintf(stderr, "%s: asset build toolchain missing.", argv[0]);
        return (1);
    }

    while ((*(build_types + i)).name) {
        if (!strcmp((*(build_types + i)).name, argv[2]))
            return ((*(build_types + i)).callback(argc, argv));
        ++i;
    };

    fprintf(stderr, "%s: invalid build toolchain '%s'.", argv[0], argv[2]);

    return (1);
}
