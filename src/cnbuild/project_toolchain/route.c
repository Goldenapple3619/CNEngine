#include "project_toolchain.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <dirent.h>
    #include <sys/stat.h>
#endif


CNProject *new_cnproject(void)
{
    CNProject *proj = malloc(sizeof(CNProject));

    if (!proj)
        return (NULL);

    proj->builds.capacity = 0;
    proj->builds.size = 0;
    proj->builds.content = NULL;

    proj->name = NULL;
    proj->version_name = NULL;

    proj->content.capacity = 0;
    proj->content.size = 0;
    proj->content.content = NULL;

    return (proj);
}

CNAsset *new_cnasset(const char *asset_location, cnasset_type tp)
{
    CNAsset *asset = malloc(sizeof(CNAsset));

    if (!asset)
        return (NULL);

    asset->location = asset_location ? strdup(asset_location) : NULL;
    asset->type = tp;

    return (asset);
}

void delete_cnasset(CNAsset *ptr)
{
    if (!ptr)
        return;
    if (ptr->location)
        (void)free(ptr->location);
    (void)free(ptr);
}

void delete_cnproject(CNProject *ptr)
{
    if (!ptr)
        return;
    if (ptr->builds.content)
        (void)empty_generic_vector(&ptr->builds, &free);
    if (ptr->content.content)
        (void)empty_generic_vector(&ptr->content, &free);
    if (ptr->name)
        (void)free(ptr->name);
    if (ptr->version_name)
        (void)free(ptr->version_name);
    (void)free(ptr);
}

cnasset_type tp_from_string(const char *str)
{
    if (!str)
        return (-1);
    if (!strcmp(str, "objects"))
        return (CNASSET_TP_OBJ);
    if (!strcmp(str, "scenes"))
        return (CNASSET_TP_SCN);
    if (!strcmp(str, "assets"))
        return (CNASSET_TP_RAW);
    if (!strcmp(str, "guis"))
        return (CNASSET_TP_GUI);
    if (!strcmp(str, "srcs"))
        return (CNASSET_TP_SRC);
    if (!strcmp(str, "pcasset"))
        return (CNASSET_TP_PCA);
    return (-1);
}

char *resolve_path(char *base_path, const char *project_root)
{
    const char *placeholder = "${project_root}";
    char *pos;
    char *new_path;
    size_t prefix_len;
    size_t suffix_len;
    size_t root_len;

    if (!base_path || !project_root) {
        return (NULL);
    }

    pos = strstr(base_path, placeholder);

    if (!pos)
        return (base_path);

    prefix_len = (size_t)(pos - base_path);
    suffix_len = strlen(pos + strlen(placeholder));
    root_len = strlen(project_root);

    new_path = malloc(prefix_len + root_len + suffix_len + 1);

    if (!new_path) {
        (void)free(base_path);
        return (NULL);
    }

    (void)memcpy(new_path, base_path, prefix_len);
    (void)memcpy(new_path + prefix_len, project_root, root_len);
    (void)memcpy(new_path + prefix_len + root_len,
           pos + strlen(placeholder),
           suffix_len + 1);

    (void)free(base_path);
    return new_path;
}

uint8_t cnressources_walk_path(CNProject *project, char *path, cnasset_type tp)
{
    CNAsset *temp_res;
    size_t len;
    char *the_path;

    if (!project || !path)
        return (1);

    #ifdef _WIN32
        char search_path[MAX_PATH];
        WIN32_FIND_DATAA find_data;
        HANDLE hfind;

        (void)snprintf(search_path, sizeof(search_path), "%s\\*", path);

        hfind = FindFirstFileA(search_path, &find_data);
        if (hfind == INVALID_HANDLE_VALUE)
            return (1);

        do {
            if (!strcmp(find_data.cFileName, ".") || !strcmp(find_data.cFileName, ".."))
                continue;

            if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                continue;

            len = strlen(path) + strlen(find_data.cFileName) + 2;
            the_path = malloc(len);

            if (!the_path) {
                (void)FindClose(hfind);
                return 1;
            }

            (void)snprintf(the_path, len, "%s\\%s", path, find_data.cFileName);

            temp_res = new_cnasset(the_path, tp);

            (void)free(the_path);

            if (!temp_res) {
                (void)FindClose(hfind);
                return (1);
            }

            if (insert_generic_vector(&project->content, temp_res)) {
                (void)delete_cnasset(temp_res);
                (void)FindClose(hfind);
                return 1;
            }

        } while (FindNextFileA(hfind, &find_data));

        (void)FindClose(hfind);
    #else
        DIR *dir = opendir(path);
        struct dirent *entry;
        struct stat st;

        if (!dir)
            return 1;

        while ((entry = readdir(dir)) != NULL) {
            if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
                continue;

            len = strlen(path) + strlen(entry->d_name) + 2;
            the_path = malloc(len);

            if (!the_path) {
                (void)closedir(dir);
                return 1;
            }

            (void)snprintf(the_path, len, "%s/%s", path, entry->d_name);

            if (stat(the_path, &st) != 0 || !S_ISREG(st.st_mode)) {
                (void)free(the_path);
                continue;
            }

            temp_res = new_cnasset(the_path, tp);

            (void)free(the_path);

            if (!temp_res) {
                (void)closedir(dir);
                return (1);
            }

            if (insert_generic_vector(&project->content, temp_res)) {
                (void)delete_cnasset(temp_res);
                (void)closedir(dir);
                return 1;
            }
        }

        (void)closedir(dir);
    #endif

    return (0);
}

uint8_t parse_cnressources(CNProject *project, xmlNode *node, const char *project_root)
{
    char *res_path;
    cnasset_type tp;
    CNAsset *temp_res;
    xmlChar *temp_s;

    for (xmlNode *node_child = node->children; node_child; node_child = node_child->next) {
        if (node_child->type != XML_ELEMENT_NODE)
            continue;

        res_path = string_from_node(node_child);

        if (!res_path)
            return (1);

        res_path = resolve_path(res_path, project_root);

        if (!res_path)
            return (1);

        temp_s = xmlGetProp(node_child, (xmlChar *)"type");

        if (!temp_s) {
            fprintf(stderr, "missing element type for '%s'.\n", node_child->name);
            (void)free(res_path);
            return (1);
        }

        tp = tp_from_string((const char *)temp_s);
        xmlFree(temp_s);

        if (tp < 0) {
            fprintf(stderr, "invalid element type for '%s'.\n", node_child->name);
            (void)free(res_path);
            return (1);
        }

        if (!strcmp((const char *)node_child->name, "dir")) {
            if (cnressources_walk_path(project, res_path, tp)) {
                (void)free(res_path);
                return (1);
            }
        } else if (!strcmp((const char *)node_child->name, "file")) {
            temp_res = new_cnasset(res_path, tp);

            if (!temp_res)
                return (1);

            if (insert_generic_vector(&project->content, temp_res)) {
                (void)delete_cnasset(temp_res);
                return (1);
            }
        } else {
            fprintf(stderr, "invalid element '%s' in '%s'.\n", node_child->name, node->name);
            (void)free(res_path);
            return (1);
        }

        (void)free(res_path);
    }

    return (0);
}

uint8_t parse_cnbuilds(CNProject *project, xmlNode *node)
{
    CNBuild *build;

    (void)project;
    (void)build;
    for (xmlNode *node_child = node->children; node_child; node_child = node_child->next) {
        if (node_child->type != XML_ELEMENT_NODE)
            continue;

        if (!strcmp((const char *)node_child->name, "binary")) {
            for (xmlNode *build_content_node = node_child->children; build_content_node; build_content_node = build_content_node->next) {
                if (build_content_node->type != XML_ELEMENT_NODE)
                    continue;

                if (!strcmp((const char *)build_content_node->name, "dependencies")) {

                } else if (!strcmp((const char *)build_content_node->name, "name")) {

                } else if (!strcmp((const char *)build_content_node->name, "entry")) {

                } else {
                    fprintf(stderr, "invalid element '%s' in '%s'.\n", build_content_node->name, node_child->name);
                    return (1);
                }
            }
        } else {
            fprintf(stderr, "invalid element '%s' in '%s'.\n", node_child->name, node->name);
            return (1);
        }
    }

    return (0);
}

char *get_dirname(const char *path)
{
    const char *last_slash = NULL;
    char *out;
    size_t len;

    for (const char *p = path; *p; p++) {
        if (*p == '/' || *p == '\\')
            last_slash = p;
    }

    if (!last_slash) {
        out = malloc(2);

        if (!out)
            return (NULL);

        out[0] = '.';
        out[1] = '\0';

        return (out);
    }

    len = (size_t)(last_slash - path);

    out = malloc(len + 1);

    if (!out)
        return (NULL);

    (void)memcpy(out, path, len);
    out[len] = '\0';

    return (out);
}

CNProject *parse_project_xml(const char *file_path, const char *project_root)
{
    CNProject *parsed_data; 
    xmlDoc *doc;
    xmlNode *root;

    doc = xmlReadFile(file_path, NULL, 0);

    if (!doc) {
        fprintf(stderr, "%s: failed to open and parse file.\n", file_path);
        return (NULL);
    }

    root = xmlDocGetRootElement(doc);

    if (strcmp((const char *)root->name, "project")) {
        fprintf(stderr, "%s: invalid root element '%s', expecting 'gui'.\n", file_path, root->name);
        (void)xmlFreeDoc(doc);
        (void)xmlCleanupParser();
        return (NULL);
    }

    parsed_data = new_cnproject();

    if (!parsed_data) {
        (void)xmlFreeDoc(doc);
        (void)xmlCleanupParser();
        return (NULL);
    }

    for (xmlNode *node = root->children; node; node = node->next) {
        if (node->type != XML_ELEMENT_NODE)
            continue;
        if (!strcmp((const char *)node->name, "name")) {
            if (parsed_data->name)
                (void)free(parsed_data->name);

            parsed_data->name = string_from_node(node);

            if (!parsed_data->name) {
                (void)delete_cnproject(parsed_data);
                (void)xmlFreeDoc(doc);
                (void)xmlCleanupParser();
                return (NULL);
            }
        } else if (!strcmp((const char *)node->name, "version")) {
            if (parsed_data->version_name)
                (void)free(parsed_data->version_name);

            parsed_data->version_name = string_from_node(node);

            if (!parsed_data->version_name) {
                (void)delete_cnproject(parsed_data);
                (void)xmlFreeDoc(doc);
                (void)xmlCleanupParser();
                return (NULL);
            }
        } else if (!strcmp((const char *)node->name, "builds")) {
            if (parse_cnbuilds(parsed_data, node)) {
                (void)delete_cnproject(parsed_data);
                (void)xmlFreeDoc(doc);
                (void)xmlCleanupParser();
                return (NULL);
            }
        } else if (!strcmp((const char *)node->name, "ressources")) {
            if (parse_cnressources(parsed_data, node, project_root)) {
                (void)delete_cnproject(parsed_data);
                (void)xmlFreeDoc(doc);
                (void)xmlCleanupParser();
                return (NULL);
            }
        } else {
            fprintf(stderr, "%s: invalid element '%s'.\n", file_path, root->name);
            (void)delete_cnproject(parsed_data);
            (void)xmlFreeDoc(doc);
            (void)xmlCleanupParser();
            return (NULL);
        }
    }

    (void)xmlFreeDoc(doc);
    (void)xmlCleanupParser();

    return (parsed_data);
}

char *join_path(const char *a, const char *b) {
    if (!a || !b)
        return (NULL);

    size_t len;
    size_t len_a = strlen(a);
    size_t len_b = strlen(b);
    size_t total = len_a + len_b + 2;
    char *result = (char *)malloc(total);

    if (!result)
        return (NULL);

    result[0] = '\0';
    strcpy(result, a);

    if (!(len_a > 0 && (a[len_a - 1] == '/' || a[len_a - 1] == '\\'))) {
        len = strlen(result);
        result[len] = PATH_SEP;
        result[len + 1] = '\0';
    }

    strcat(result, b);
    return (result);
}


cnbool is_dir(const char *path) {
    #ifdef _WIN32
        DWORD attr = GetFileAttributesA(path);

        if (attr == INVALID_FILE_ATTRIBUTES)
            return (false);

        return ((attr & FILE_ATTRIBUTE_DIRECTORY) ? true : false);
    #else
        struct stat s;

        return ((stat(path, &s) == 0 && S_ISDIR(s.st_mode)) ? true : false);
    #endif
}


char *init_build_path(const char *project_root)
{
    char *build_path = join_path(project_root, "build");

    if (!build_path)
        return (NULL);

    if (!is_dir(build_path)) {
        if (MKDIR(build_path)) {
            (void)free(build_path);
            return (NULL);
        }
    }

    return (build_path);
}

int run_gcc_compile(const char *input, const char *output)
{
#ifdef _WIN32
    char cmd[2048];

    snprintf(cmd, sizeof(cmd),
             "gcc -c \"%s\" -o \"%s\"",
             input, output);

    STARTUPINFOA si = {0};
    PROCESS_INFORMATION pi = {0};
    si.cb = sizeof(si);

    if (!CreateProcessA(
            NULL,
            cmd,
            NULL, NULL,
            FALSE,
            0,
            NULL,
            NULL,
            &si,
            &pi))
    {
        return -1;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return 0;

#else
    pid_t pid = fork();
    if (pid == 0)
    {
        char *argv[] = {
            "gcc",
            "-c",
            (char *)input,
            "-o",
            (char *)output,
            NULL
        };

        execvp("gcc", argv);
        _exit(127);
    }
    else if (pid > 0)
    {
        int status;
        waitpid(pid, &status, 0);
        return status;
    }

    return -1;
#endif
}

CNProject *parse_project(const char *file_path, Object *asset_ctx)
{
    CNProject *project;
    CNAsset *temp_asset;
    char *project_root = get_dirname(file_path);
    char *build_path;
    
    if (!project_root)
        return (NULL);

    build_path = init_build_path(project_root);

    if (!build_path) {
        (void)free(project_root);
        return (NULL);
    }

    project = parse_project_xml(file_path, project_root);

    if (!project) {
        (void)free(build_path);
        (void)free(project_root);
        return (NULL);
    }

    for (size_t i = 0; i < project->content.size; ++i) {
        temp_asset = project->content.content[i];

        if (temp_asset->type == 4) {
            const char *name = strrchr(temp_asset->location, '/');
            char base[512];
            #ifdef _WIN32
                if (!name) name = strrchr(temp_asset->location, '\\');
            #endif
            name = name ? name + 1 : temp_asset->location;

            snprintf(base, sizeof(base), "%s/%s.o", build_path, name);

            run_gcc_compile(temp_asset->location, base);
        }

        printf("%d: %s\n", temp_asset->type, temp_asset->location);
    }
    
    (void)asset_ctx;
    (void)free(build_path);
    (void)free(project_root);

    return (project);
}

int build_project(size_t argc, char **argv, Object *asset_ctx)
{
    struct build_args_s build_args = {0};
    CNProject *left_overs;
    
    (void)asset_ctx;

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

    left_overs = parse_project(build_args.input_files.content[0], asset_ctx);

    if (!left_overs) {
        (void)reset_args(&build_args);
        return (1);
    }

    (void)delete_cnproject(left_overs);
    (void)reset_args(&build_args);

    return (0);
}