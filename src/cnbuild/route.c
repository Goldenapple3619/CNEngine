#include "build.h"

uint8_t build_gui_element(xmlNode *node)
{
    xmlChar *content;

    for (xmlNode *node_child = node->children; node_child; node_child = node_child->next) {
        if (node_child->type == XML_ELEMENT_NODE)
            if (build_gui_element(node_child))
                return (1);
        if (node_child->type == XML_TEXT_NODE) {
            content = xmlNodeGetContent(node_child);
            if (!content)
                return (1);
            // printf("  %s  -  %s\n", node_child->parent->name, content);
            xmlFree(content);
        }
    }

    return (0);
}

uint8_t parse_gui(const char *file_path, struct engine_object_file_writer_ctx_s *wctx)
{
    xmlDoc *doc;
    xmlNode *root;
    xmlChar *temp_s;

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
                if (build_gui_element(node_child))
                    return (1);
            }
        } else if (!strcmp((const char *)node->name, "connectors")) {
            for (xmlNode *node_child = node->children; node_child; node_child = node_child->next) {
                if (node_child->type != XML_ELEMENT_NODE)
                    continue;
            }
        } else {
            fprintf(stderr, "%s: invalid element '%s'.\n", file_path, root->name);
            return (1);
        }
    }

    (void)xmlFreeDoc(doc);
    (void)xmlCleanupParser();
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
        (void)delete_writer_ctx(wctx);
        (void)reset_args(&build_args);
        return (1);
    }

    fp = fopen(build_args.output_file, "w");

    if (!fp) {
        fprintf(stderr, "%s: failed to open output file.\n", build_args.output_file);
        (void)delete_writer_ctx(wctx);
        (void)reset_args(&build_args);
        return (1);
    }
    
    if (write_object_file(fp, wctx)) {
        fprintf(stderr, "%s: failed to write output file.\n", build_args.output_file);
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
