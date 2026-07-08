#include "project_toolchain.h"

cnasset_type tp_from_string(const char *str)
{
    if (!str) {
        RAISE(ERR_INVALID_POINTER, "can't get asset type from empty string.");
        return (-1);
    }
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
    RAISE_FMT(ERR_INVALID_TYPE, "invalid type string '%s'.", str);
    return (-1);
}

uint8_t cnressources_walk_path(CNProject *project, char *path, cnasset_type tp)
{
    CNAsset *temp_res;
    size_t len;
    char *the_path;

    if (!project) {
        RAISE(ERR_INVALID_POINTER, "can't walk path on empty project.");
        return (1);
    }

    if (!path) {
        RAISE(ERR_INVALID_POINTER, "can't walk empty path.");
        return (1);
    }

    #ifdef _WIN32
        char search_path[MAX_PATH];
        WIN32_FIND_DATAA find_data;
        HANDLE hfind;

        (void)snprintf(search_path, sizeof(search_path), "%s\\*", path);

        hfind = FindFirstFileA(search_path, &find_data);
        if (hfind == INVALID_HANDLE_VALUE) {
            RAISE_FMT(ERR_OS, "failed to FindFirstFileA in '%s' for '%s'.", search_path, path);
            return (1);
        }

        do {
            if (!strcmp(find_data.cFileName, ".") || !strcmp(find_data.cFileName, ".."))
                continue;

            if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                continue;

            len = strlen(path) + strlen(find_data.cFileName) + 2;
            the_path = malloc(len);

            if (!the_path) {
                RAISE_FMT(ERR_OUT_OF_MEMORY, "failed to allocate new path string of size %zu.", len);
                (void)FindClose(hfind);
                return 1;
            }

            (void)snprintf(the_path, len, "%s\\%s", path, find_data.cFileName);

            temp_res = new_cnasset(the_path, tp);

            (void)free(the_path);

            if (!temp_res) {
                PROPAGATE_ERR();
                (void)FindClose(hfind);
                return (1);
            }

            if (insert_generic_vector(&project->content, temp_res)) {
                PROPAGATE_ERR();
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
                RAISE_FMT(ERR_OUT_OF_MEMORY, "failed to allocate new path string of size %zu.", len);
                (void)closedir(dir);
                return 1;
            }

            (void)snprintf(the_path, len, "%s/%s", path, entry->d_name);

            if (stat(the_path, &st) != 0 || !S_ISREG(st.st_mode)) {
                RAISE_FMT(ERR_OS, "failed to stat path '%s'.", the_path);
                (void)free(the_path);
                continue;
            }

            temp_res = new_cnasset(the_path, tp);

            (void)free(the_path);

            if (!temp_res) {
                PROPAGATE_ERR();
                (void)closedir(dir);
                return (1);
            }

            if (insert_generic_vector(&project->content, temp_res)) {
                PROPAGATE_ERR();
                (void)delete_cnasset(temp_res);
                (void)closedir(dir);
                return 1;
            }
        }

        (void)closedir(dir);
    #endif

    return (0);
}
