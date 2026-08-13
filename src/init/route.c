#include "init.h"

static const char project_desc[] = ""
"<project cnversion=\"${CNVERSION}\">\n"
"    <name>${PROJECT_NAME}</name>\n"
"    <version>1.0.0</version>\n"
"\n"
"    <builds>\n"
"        <binary os=\"linux\" arch=\"amd64\">\n"
"            <dependencies>\n"
"                <module>core</module>\n"
"                <module>assets</module>\n"
"                <module>graphic</module>\n"
"                <module>input</module>\n"
"                <module>audio</module>\n"
"                <module>r3d</module>\n"
"                <module>rgui</module>\n"
"            </dependencies>\n"
"            <name>client-linux-64bits</name>\n"
"            <entry>\n"
"                main\n"
"            </entry>\n"
"            <assets>\n"
"                <endian>little</endian>\n"
"                <align>8</align>\n"
"                <max-size>2100000000</max-size>\n"
"            </assets>\n"
"        </binary>\n"
"    </builds>\n"
"\n"
"    <ressources>\n"
"        <dir type=\"objects\">${project_root}/objects</dir>\n"
"        <dir type=\"scenes\" packing=\"nolink\">${project_root}/scenes</dir>\n"
"        <dir type=\"assets\">${project_root}/assets</dir>\n"
"        <dir type=\"guis\">${project_root}/guis</dir>\n"
"        <dir type=\"srcs\">${project_root}/src</dir>\n"
"    </ressources>\n"
"</project>\n"
"\n";

char *get_input(const char *prompt)
{
    char *buf;
    char *tmp;
    size_t capacity = 64;
    size_t size = 0;
    int c;

    if (prompt) {
        printf("%s", prompt);
        fflush(stdout);
    }

    buf = malloc(sizeof(char) * capacity);

    if (!buf) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to prealloc user input.");
        return (NULL);
    }

    while ((c = getchar()) != EOF && c != '\n') {
        if ((size + 1) >= capacity) {
            capacity *= 2;
            tmp = realloc(buf, capacity);

            if (!tmp) {
                RAISE(ERR_OUT_OF_MEMORY, "failed to realloc user input.");
                (void)free(buf);
                return (NULL);
            }

            buf = tmp;
        }

        buf[size++] = (char)c;
    }

    if (c == EOF && size == 0) {
        RAISE(ERR_OK, "user input cancelled.");
        (void)free(buf);
        return (NULL);
    }

    buf[size] = '\0';

    tmp = realloc(buf, size + 1);
    return (tmp ? tmp : buf);
}

void empty_project_definition(ProjectDefinition *project)
{
    if (project->name)
        (void)free(project->name);
}

uint8_t init_project_file(const ProjectDefinition *project, const char *output_path)
{
    String temp;
    FILE *fp;
    char *project_file_path = join_path(output_path, "project.cnproj");

    if (!project_file_path) {
        PROPAGATE_ERR();
        return (1);
    }

    temp.c_str = NULL;
    temp.size = 0;

    if (str_override_cp(&temp, project_desc)) {
        PROPAGATE_ERR();
        (void)free(project_file_path);
        empty_str(&temp);
        return (1);
    }

    if (str_replace(&temp, "${CNVERSION}", "1", 0)) {
        PROPAGATE_ERR();
        (void)free(project_file_path);
        empty_str(&temp);
        return (1);
    }

    if (str_replace(&temp, "${PROJECT_NAME}", project->name, 0)) {
        PROPAGATE_ERR();
        (void)free(project_file_path);
        empty_str(&temp);
        return (1);
    }

    fp = fopen(project_file_path, "wb");

    if (!fp) {
        RAISE_FMT(ERR_OS, "failed to open %s.", project_file_path);
        (void)free(project_file_path);
        empty_str(&temp);
        return (1);
    }

    (void)free(project_file_path);

    if (fwrite(temp.c_str, sizeof(char), temp.size, fp) != temp.size) {
        RAISE(ERR_OS, "failed to write content to project file.");
        empty_str(&temp);
        fclose(fp);
        return (1);
    }

    empty_str(&temp);
    fclose(fp);
    return (0);
}

uint8_t init_directory_structure(const char *output_path)
{
    const char *DIR_STRUCTURE[] = {
        "assets",
        "guis",
        "objects",
        "scenes",
        "src"
    };
    char *path;

    for (size_t i = 0; i < sizeof(DIR_STRUCTURE) / sizeof(char *); ++i) {
        path = join_path(output_path, DIR_STRUCTURE[i]);

        if (!path) {
            PROPAGATE_ERR();
            return (1);
        }

        if (!is_dir(path)) {
            if (make_dir(path)) {
                (void)free(path);
                PROPAGATE_ERR();
                return (1);
            }
        }

        (void)free(path);
    }

    return (0);
}

int init(size_t argc, char **argv)
{
    (void)argc;
    (void)argv;

    char *output_path = strdup(".");
    char *project_file_path;

    if (!output_path) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate output path.");
        return (1);
    }

    for (size_t i = 0; i < argc - 2; ++i) {
        if (!strcmp(argv[2 + i], "-D")) {
            if (argc - 2 <= i + 1) {
                fprintf(stderr, "missing path after -D.\n");
                return (1);
            } else {
                ++i;
            }

            if (!is_dir(argv[2 + i])) {
                fprintf(stderr, "path after -D is not reachable.\n");
                return (1);
            }

            if (output_path)
                (void)free(output_path);

            output_path = strdup(argv[2 + i]);

            if (!output_path) {
                RAISE(ERR_OUT_OF_MEMORY, "failed to allocate output path.");
                return (1);
            }
        }
    }

    ProjectDefinition project;
    char *temp_resp;

    project_file_path = join_path(output_path, "project.cnproj");

    if (!project_file_path) {
        PROPAGATE_ERR();
        (void)free(output_path);
        return (1);
    }

    (void)memset(&project, 0, sizeof(ProjectDefinition));

    if (is_file(project_file_path)) {
        temp_resp = get_input("project.cnproj already found in this repository, do you still wish to continue and overwrite it ? [Y/n]: ");

        if (!temp_resp) {
            PROPAGATE_ERR();
            (void)free(project_file_path);
            (void)free(output_path);
            return (1);
        }

        for (size_t i = 0; temp_resp[i]; ++i)
            temp_resp[i] = tolower(temp_resp[i]);

        if (!strcmp(temp_resp, "yes") || !strcmp(temp_resp, "y")) {
            (void)free(temp_resp);
        } else {
            (void)free(temp_resp);
            (void)free(output_path);
            (void)free(project_file_path);
            printf("cancelled.\n");
            return (0);
        }
    }

    (void)free(project_file_path);

    project.name = get_input("project name: ");

    if (!project.name) {
        PROPAGATE_ERR();
        (void)free(output_path);
        (void)empty_project_definition(&project);
        return (1);
    }

    if (init_directory_structure(output_path)) {
        PROPAGATE_ERR();
        (void)free(output_path);
        (void)empty_project_definition(&project);
        return (1);
    }

    if (init_project_file(&project, output_path)) {
        PROPAGATE_ERR();
        (void)free(output_path);
        (void)empty_project_definition(&project);
        return (1);
    }

    (void)free(output_path);
    (void)empty_project_definition(&project);
    return (0);
}
