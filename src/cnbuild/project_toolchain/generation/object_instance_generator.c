#include "../project_toolchain.h"

static const char GENERATED_CONTENT_START[] = "#include \"libcncore.h\"\n"
"Object *c_new_${OBJ_NAME}(void **args) {"
"Object *obj = new_object();"
"if (!obj) {"
"PROPAGATE_ERR();"
"return (NULL);"
"}"
"Object *${PARENT_OBJ_CONSTRUCTOR}(void **args);SET_PARENT_CLASS_BUILD_STATIC(obj, ${PARENT_OBJ_CONSTRUCTOR}(NULL));";

static const char GENERATED_CONTENT_METHODS[] = "CREATE_METHOD_CLASS_BUILD(obj, \"${SYMBOL_NAME}\", &${SYMBOL_REF});";
static const char GENERATED_CONTENT_ATTRS[] = "if (!set_attr(obj, \"${ATTR_NAME}\", ${ATTR_TYPE}, ${ATTR_REF})) {PROPAGATE_ERR(); (void)delete_object(obj); return (NULL);};";

static const char GENERATED_CONTENT_END[] = "return (obj);"
"}";

static const char GENERATED_PREFIX_NAME[] = "__generated_";

void delete_object_generation_data(ObjectGenerationData *obj_data)
{
    if (!obj_data) {
        RAISE(ERR_INVALID_POINTER, "can't delete empty object generation data.");
        return;
    }

    if (obj_data->object_name)
        (void)free(obj_data->object_name);
    if (obj_data->object_base)
        (void)free(obj_data->object_base);
    if (obj_data->generate_in)
        (void)free(obj_data->generate_in);
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

uint8_t build_object_methods(xmlNode *node, ObjectGenerationData *obj_data)
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

uint8_t build_object_attributes(xmlNode *node, ObjectGenerationData *obj_data)
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

ObjectGenerationData *get_object_generation_data(const char *object_path)
{
    ObjectGenerationData *obj_data;
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

    obj_data = malloc(sizeof(ObjectGenerationData));

    if (!obj_data) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate object data.");
        (void)xmlFreeDoc(doc);
        return (NULL);
    }

    temp_s = xmlGetProp(root, (xmlChar *)"name");

    if (!temp_s) {
        obj_data->object_name = NULL;
    } else {
        obj_data->object_name = strdup((char *)temp_s);
        xmlFree(temp_s);
    }

    temp_s = xmlGetProp(root, (xmlChar *)"base");

    if (!temp_s) {
        obj_data->object_base = NULL;
    } else {
        obj_data->object_base = strdup((char *)temp_s);
        xmlFree(temp_s);
    }

    obj_data->generate_in = NULL;
    (void)init_generic_vector(&obj_data->attributes);
    (void)init_generic_vector(&obj_data->methods);

    for (xmlNode *node = root->children; node; node = node->next) {
        if (node->type != XML_ELEMENT_NODE)
            continue;
        if (!strcmp((const char *)node->name, "generate_in")) {
            if (obj_data->generate_in)
                (void)free(obj_data->generate_in);

            obj_data->generate_in = string_from_node(node);

            if (!obj_data->generate_in) {
                PROPAGATE_ERR();
                (void)delete_object_generation_data(obj_data);
                (void)xmlFreeDoc(doc);
                return (NULL);
            }
        } else if (!strcmp((const char *)node->name, "methods")) {
            if (build_object_methods(node, obj_data)) {
                PROPAGATE_ERR();
                (void)delete_object_generation_data(obj_data);
                (void)xmlFreeDoc(doc);
                return (NULL);
            }
        } else if (!strcmp((const char *)node->name, "attrs")) {
            if (build_object_attributes(node, obj_data)) {
                PROPAGATE_ERR();
                (void)delete_object_generation_data(obj_data);
                (void)xmlFreeDoc(doc);
                return (NULL);
            }
        } else {
            RAISE_FMT(ERR_INVALID_TYPE, "invalid element '%s' in '%s'.", node->name, root->name);
            (void)delete_object_generation_data(obj_data);
            (void)xmlFreeDoc(doc);
            return (NULL);
        }
    }

    (void)xmlFreeDoc(doc);
    return (obj_data);
}

uint8_t add_base_content_to_generation(String *generated, const char *base_content_path)
{
    FILE *file;
    char *buffer;
    long file_size;
    size_t read_size;

    file = fopen(base_content_path, "rb");

    if (!file) {
        PROPAGATE_ERR();
        return (1);
    }
    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        PROPAGATE_ERR();
        return (1);
    }

    file_size = ftell(file);

    if (file_size < 0) {
        fclose(file);
        PROPAGATE_ERR();
        return (1);
    }
    if (fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        PROPAGATE_ERR();
        return (1);
    }

    buffer = malloc((size_t)file_size + 1);

    if (!buffer) {
        fclose(file);
        PROPAGATE_ERR();
        return (1);
    }

    read_size = fread(buffer, 1, (size_t)file_size, file);

    fclose(file);

    if (read_size != (size_t)file_size) {
        free(buffer);
        PROPAGATE_ERR();
        return (1);
    }

    buffer[file_size] = '\0';

    if (str_rcadd_cp(generated, buffer)) {
        free(buffer);
        PROPAGATE_ERR();
        return (1);
    }

    free(buffer);

    return (0);
}

uint8_t add_entry_to_generation(String *generated, ObjectGenerationData *obj_data)
{
    String temp;
    String temp_constructor;

    temp.c_str = NULL;
    temp.size = 0;

    temp_constructor.c_str = NULL;
    temp_constructor.size = 0;

    if (strstr(obj_data->object_base, "__builtin_") == obj_data->object_base) {
        if (str_override_cp(&temp_constructor, "new_")) {
            PROPAGATE_ERR();
            return (1);
        }

        if (str_rcadd_cp(&temp_constructor, obj_data->object_base + 10)) {
            PROPAGATE_ERR();
            empty_str(&temp_constructor);
            return (1);
        }
    } else {
        if (str_override_cp(&temp_constructor, "c_new_")) {
            PROPAGATE_ERR();
            return (1);
        }

        if (str_rcadd_cp(&temp_constructor, obj_data->object_base)) {
            PROPAGATE_ERR();
            empty_str(&temp_constructor);
            return (1);
        }
    }

    if (str_override_cp(&temp, GENERATED_CONTENT_START)) {
        PROPAGATE_ERR();
        empty_str(&temp_constructor);
        return (1);
    }

    if (str_replace(&temp, "${OBJ_NAME}", obj_data->object_name, 0) || str_replace(&temp, "${PARENT_OBJ_CONSTRUCTOR}", temp_constructor.c_str, 0)) {
        PROPAGATE_ERR();
        empty_str(&temp_constructor);
        empty_str(&temp);
        return (1);
    }

    empty_str(&temp_constructor);

    if (str_rcadd_cp(generated, temp.c_str)) {
        PROPAGATE_ERR();
        empty_str(&temp);
        return (1);
    }
    empty_str(&temp);
    return (0);
}

uint8_t add_methods_to_generation(String *generated, ObjectGenerationData *obj_data)
{
    String temp;
    OBJAttrib *temp_attrib;

    temp.c_str = NULL;
    temp.size = 0;

    for (size_t i = 0; i < obj_data->methods.size; ++i) {
        if (str_override_cp(&temp, GENERATED_CONTENT_METHODS)) {
            PROPAGATE_ERR();
            return (1);
        }

        temp_attrib = obj_data->methods.content[i];

        if (str_replace(&temp, "${SYMBOL_NAME}", temp_attrib->value.as.str, 0) || str_replace(&temp, "${SYMBOL_REF}", temp_attrib->name, 0)) {
            PROPAGATE_ERR();
            empty_str(&temp);
            return (1);
        }

        if (str_rcadd_cp(generated, temp.c_str)) {
            PROPAGATE_ERR();
            empty_str(&temp);
            return (1);
        }
    }
    empty_str(&temp);
    return (0);
}

const char *type_enum_name_from_value(cn_type type)
{
    switch (type) {
        case CN_TYPE_BOOL:
            return ("CN_TYPE_BOOL");
        case CN_TYPE_STRING:
            return ("CN_TYPE_STRING");
        case CN_TYPE_FLOAT:
            return ("CN_TYPE_FLOAT");
        case CN_TYPE_FUNCTION:
            return ("CN_TYPE_FUNCTION");
        case CN_TYPE_GENERIC_UNIQ_PTR:
            return ("CN_TYPE_GENERIC_UNIQ_PTR");
        case CN_TYPE_INT:
            return ("CN_TYPE_INT");
        case CN_TYPE_NUMBER:
            return ("CN_TYPE_NUMBER");
        case CN_TYPE_OBJECT:
            return ("CN_TYPE_OBJECT");
        case CN_TYPE_RECT:
            return ("CN_TYPE_RECT");
        case CN_TYPE_VEC2:
            return ("CN_TYPE_VEC2");
        case CN_TYPE_VEC3:
            return ("CN_TYPE_VEC3");
        case CN_TYPE_WEAK_OBJECT:
            return ("CN_TYPE_WEAK_OBJECT");
        case CN_TYPE_NULL:
            return ("CN_TYPE_NULL");
        default:
            return ("CN_TYPE_NULL");
    }
}

uint8_t add_attrs_to_generation(String *generated, ObjectGenerationData *obj_data)
{
    String temp;
    char *temp_arg;
    size_t len;
    OBJAttrib *temp_attrib;

    temp.c_str = NULL;
    temp.size = 0;

    for (size_t i = 0; i < obj_data->attributes.size; ++i) {
        if (str_override_cp(&temp, GENERATED_CONTENT_ATTRS)) {
            PROPAGATE_ERR();
            return (1);
        }

        temp_attrib = obj_data->attributes.content[i];

        len = (size_t)snprintf(NULL, 0, "args[%zu]", i) + 1;
        temp_arg = malloc(len);

        if (!temp_arg) {
            RAISE(ERR_OUT_OF_MEMORY, "failed to allocate alignement flag.");
            return (1);
        }

        (void)snprintf(temp_arg, len, "args[%zu]", i);

        if (str_replace(&temp, "${ATTR_REF}", temp_arg, 0)) {
            PROPAGATE_ERR();
            (void)free(temp_arg);
            empty_str(&temp);
            return (1);
        }

        (void)free(temp_arg);

        if (str_replace(&temp, "${ATTR_NAME}", temp_attrib->name, 0)) {
            PROPAGATE_ERR();
            empty_str(&temp);
            return (1);
        }

        if (str_replace(&temp, "${ATTR_TYPE}", type_enum_name_from_value(temp_attrib->value.type), 0)) {
            PROPAGATE_ERR();
            empty_str(&temp);
            return (1);
        }

        if (str_rcadd_cp(generated, temp.c_str)) {
            PROPAGATE_ERR();
            empty_str(&temp);
            return (1);
        }
    }
    empty_str(&temp);
    return (0);
}

uint8_t generate_object_src(CNProject *project, const char *output_path, const char *project_root)
{
    CNAsset *temp;
    char *generation_output;
    char *generation_name;
    ObjectGenerationData *obj_data;
    String *generated;
    FILE *fp;
    cnbool src_exist_in_project = false;

    for (size_t i = 0; i < project->content.size; ++i) {
        temp = project->content.content[i];

        if (temp->type != CNASSET_TP_OBJ)
            continue;

        printf("generating source from %s.\n", path_basename(temp->location));

        obj_data = get_object_generation_data(temp->location);

        if (!obj_data) {
            PROPAGATE_ERR();
            return (1);
        }

        if (!obj_data->object_name) {
            RAISE(ERR_INVALID_POINTER, "objects are required to have a name.");
            (void)delete_object_generation_data(obj_data);
            return (1);
        }

        if (!obj_data->object_base) {
            RAISE(ERR_INVALID_POINTER, "objects are required to have a base.");
            (void)delete_object_generation_data(obj_data);
            return (1);
        }

        if (obj_data->generate_in) {
            obj_data->generate_in = resolve_path(obj_data->generate_in, project_root, "${project_root}");

            if (!obj_data->generate_in) {
                PROPAGATE_ERR();
                (void)delete_object_generation_data(obj_data);
                return (1);
            }
        }

        generation_name = malloc(sizeof(char) * (((sizeof(GENERATED_PREFIX_NAME) / sizeof(char)) - 1 + strlen(obj_data->generate_in ? path_basename(obj_data->generate_in) : obj_data->object_name) + (obj_data->generate_in ? 0 : 2) + 1)));

        if (!generation_name) {
            RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new generated file name.");
            (void)delete_object_generation_data(obj_data);
            return (1);
        }

        (void)strcpy(generation_name, GENERATED_PREFIX_NAME);
        (void)strcpy(generation_name + (sizeof(GENERATED_PREFIX_NAME) / sizeof(char)) - 1, obj_data->generate_in ? path_basename(obj_data->generate_in) : obj_data->object_name);
        if (!obj_data->generate_in)
            (void)strcpy(generation_name + (sizeof(GENERATED_PREFIX_NAME) / sizeof(char)) - 1 + strlen(obj_data->object_name), ".c");

        generation_output = join_path(output_path, generation_name);

        (void)free(generation_name);

        if (!generation_output) {
            PROPAGATE_ERR();
            (void)delete_object_generation_data(obj_data);
            return (1);
        }

        generated =  new_str_from_const("");

        if (!generated) {
            PROPAGATE_ERR();
            (void)free(generation_output);
            (void)delete_object_generation_data(obj_data);
            return (1);
        }

        if (obj_data->generate_in) {
            if (add_base_content_to_generation(generated, obj_data->generate_in)) {
                PROPAGATE_ERR();
                (void)delete_str(generated);
                (void)delete_object_generation_data(obj_data);
                (void)free(generation_output);
                return (1);
            }
        }

        if (add_entry_to_generation(generated, obj_data)) {
            PROPAGATE_ERR();
            (void)delete_str(generated);
            (void)delete_object_generation_data(obj_data);
            (void)free(generation_output);
            return (1);
        }

        if (add_attrs_to_generation(generated, obj_data)) {
            PROPAGATE_ERR();
            (void)delete_str(generated);
            (void)delete_object_generation_data(obj_data);
            (void)free(generation_output);
            return (1);
        }

        if (add_methods_to_generation(generated, obj_data)) {
            PROPAGATE_ERR();
            (void)delete_str(generated);
            (void)delete_object_generation_data(obj_data);
            (void)free(generation_output);
            return (1);
        }

        if (str_rcadd_cp(generated, GENERATED_CONTENT_END)) {
            PROPAGATE_ERR();
            (void)delete_str(generated);
            (void)delete_object_generation_data(obj_data);
            (void)free(generation_output);
            return (1);
        }

        if (obj_data->generate_in) {
            for (size_t j = 0; j < project->content.size; ++j) {
                temp = project->content.content[j];
                if (temp->type != CNASSET_TP_SRC)
                    continue;

                if (!strcmp(temp->location, obj_data->generate_in)) {
                    src_exist_in_project = true;
                    (void)free(temp->location);
                    temp->location = strdup(generation_output);

                    if (!temp->location) {
                        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new string for src location");
                        (void)delete_str(generated);
                        (void)delete_object_generation_data(obj_data);
                        (void)free(generation_output);
                        return (1);
                    }
                }
            }
        }
        if (!src_exist_in_project) {
            temp = new_cnasset(generation_output, CNASSET_TP_SRC);

            if (insert_generic_vector(&project->content, temp)) {
                PROPAGATE_ERR();
                (void)delete_cnasset(temp);
                (void)delete_str(generated);
                (void)delete_object_generation_data(obj_data);
                (void)free(generation_output);
                return (1);
            }
        }

        (void)delete_object_generation_data(obj_data);

        fp = fopen(generation_output, "w");

        if (!fp) {
            RAISE_FMT(ERR_OS, "failed to open output file '%s'.", generation_output);
            (void)delete_str(generated);
            (void)free(generation_output);
            return (1);
        }

        if (fwrite(generated->c_str, sizeof(char), generated->size, fp) != generated->size) {
            RAISE_FMT(ERR_OS, "failed to write %zu bytes to output file '%s'.", generated->size, generation_output);
            (void)fclose(fp);
            (void)delete_str(generated);
            (void)free(generation_output);
            return (1);
        }

        (void)fclose(fp);
        (void)free(generation_output);
        (void)delete_str(generated);
    }

    return (0);
}
