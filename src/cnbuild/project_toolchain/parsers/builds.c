#include "../project_toolchain.h"

cnbuild_architectures arch_from_string(const char *str)
{
    if (!str)
        return (CNBUILD_ARCH_HOST);
    if (!strcmp(str, "amd64"))
        return (CNBUILD_ARCH_AMD64);
    if (!strcmp(str, "arm64"))
        return (CNBUILD_ARCH_ARM64);
    if (!strcmp(str, "i386"))
        return (CNBUILD_ARCH_I386);
    return (CNBUILD_ARCH_HOST);
}

cnbuild_system os_from_string(const char *str)
{
    if (!str)
        return (CNBUILD_SYS_HOST);
    if (!strcmp(str, "win"))
        return (CNBUILD_SYS_WIN);
    if (!strcmp(str, "linux"))
        return (CNBUILD_SYS_GEN_LINUX);
    if (!strcmp(str, "macos"))
        return (CNBUILD_SYS_DARWIN);
    return (CNBUILD_SYS_HOST);
}

uint8_t parse_cnbuilds_xml(const EngineConfig *config, CNProject *project, xmlNode *node)
{
    CNBuild *build;
    char *temp;
    xmlChar *temp_s;
    char *temp_module;
    char *temp_module_full;
    SubModule *submodule;

    for (xmlNode *node_child = node->children; node_child; node_child = node_child->next) {
        if (node_child->type != XML_ELEMENT_NODE)
            continue;

        if (!strcmp((const char *)node_child->name, "binary")) {
            build = new_build();

            if (!build) {
                PROPAGATE_ERR();
                return (1);
            }

            temp_s = xmlGetProp(node_child, (xmlChar *)"os");

            if (!temp_s) {
                build->machine = CNBUILD_SYS_HOST;
            } else {
                build->machine = os_from_string((const char *)temp_s);
                xmlFree(temp_s);
            }

            temp_s = xmlGetProp(node_child, (xmlChar *)"arch");

            if (!temp_s) {
                build->arch = CNBUILD_ARCH_HOST;
            } else {
                build->arch = arch_from_string((const char *)temp_s);
                xmlFree(temp_s);
            }

            for (xmlNode *build_content_node = node_child->children; build_content_node; build_content_node = build_content_node->next) {
                if (build_content_node->type != XML_ELEMENT_NODE)
                    continue;

                if (!strcmp((const char *)build_content_node->name, "dependencies")) {
                    for (xmlNode *deps_node = build_content_node->children; deps_node; deps_node = deps_node->next) {
                        if (deps_node->type != XML_ELEMENT_NODE)
                            continue;

                        if (!strcmp((const char *)deps_node->name, "module")) {
                            temp = string_from_node(deps_node);

                            if (!temp) {
                                PROPAGATE_ERR();
                                (void)delete_build(build);
                                return (1);
                            }

                            temp_module = join_path(config->submodules_location, temp);

                            if (!temp_module) {
                                PROPAGATE_ERR();
                                (void)delete_build(build);
                                (void)free(temp);
                                return (1);
                            }

                            temp_module_full = replace_extension(temp_module, "xml");

                            (void)free(temp_module);

                            if (!temp_module_full) {
                                PROPAGATE_ERR();
                                (void)delete_build(build);
                                (void)free(temp);
                                return (1);
                            }

                            submodule = new_submodule();

                            if (!submodule) {
                                PROPAGATE_ERR();
                                (void)free(temp_module_full);
                                (void)delete_build(build);
                                (void)free(temp);
                                return (1);
                            }

                            if (parse_submodules_xml(submodule, temp_module_full)) {
                                PROPAGATE_ERR();
                                (void)delete_submodule(submodule);
                                (void)free(temp_module_full);
                                (void)delete_build(build);
                                (void)free(temp);
                                return (1);
                            }

                            (void)free(temp_module_full);

                            if (insert_generic_vector(&build->dependencies, submodule)) {
                                PROPAGATE_ERR();
                                (void)delete_submodule(submodule);
                                (void)delete_build(build);
                                (void)free(temp);
                                return (1);
                            }

                            (void)free(temp);
                        } else {
                            RAISE_FMT(ERR_INVALID_TYPE, "invalid element '%s' in '%s'.", deps_node->name, build_content_node->name);
                            (void)delete_build(build);
                            return (1);
                        }
                    }
                } else if (!strcmp((const char *)build_content_node->name, "name")) {
                    temp = string_from_node(build_content_node);

                    if (!temp) {
                        PROPAGATE_ERR();
                        (void)delete_build(build);
                        return (1);
                    }

                    if (build_set_name(build, temp)) {
                        PROPAGATE_ERR();
                        (void)delete_build(build);
                        (void)free(temp);
                        return (1);
                    }
                    (void)free(temp);
                } else if (!strcmp((const char *)build_content_node->name, "entry")) {
                    temp = string_from_node(build_content_node);

                    if (!temp) {
                        PROPAGATE_ERR();
                        (void)delete_build(build);
                        return (1);
                    }

                    if (build_set_entry_point(build, temp)) {
                        PROPAGATE_ERR();
                        (void)delete_build(build);
                        (void)free(temp);
                        return (1);
                    }
                    (void)free(temp);
                } else {
                    RAISE_FMT(ERR_INVALID_TYPE, "invalid element '%s' in '%s'.", build_content_node->name, node_child->name);
                    (void)delete_build(build);
                    return (1);
                }
            }

            if (insert_generic_vector(&project->builds, build)) {
                PROPAGATE_ERR();
                (void)delete_build(build);
                return (1);
            }
        } else {
            RAISE_FMT(ERR_INVALID_TYPE, "invalid element '%s' in '%s'.", node_child->name, node->name);
            return (1);
        }
    }

    return (0);
}
