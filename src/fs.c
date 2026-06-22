#include "engine.h"

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

char *join_path(const char *a, const char *b)
{
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


cnbool is_dir(const char *path)
{
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

#ifdef _WIN32
    static char *quote_arg(const char *arg)
    {
        if (!arg)
            return (NULL);

        size_t pos = 0;
        size_t len = strlen(arg);
        size_t max_size = (len * 2) + 3;
        int need_quotes = strchr(arg, ' ') || strchr(arg, '\t') || len == 0;
        char *out = malloc(max_size);
        char c;

        if (!out)
            return (NULL);

        if (need_quotes)
            out[pos++] = '"';

        for (size_t i = 0; i < len; i++) {
            c = arg[i];
            if (c == '"') {
                out[pos++] = '\\';
                out[pos++] = '"';
            } else {
                out[pos++] = c;
            }
        }

        if (need_quotes)
            out[pos++] = '"';

        out[pos] = '\0';

        return (out);
    }
#endif

int run_program(const char *program, const char *const argv[])
{
    if (!program || !argv)
        return (-1);
    #ifdef _WIN32
        char cmd[WIN_MAX_COMMAND_SIZE];
        char *quoted;
        DWORD exit_code = 0;
        size_t used = 0;
        size_t qlen;
        STARTUPINFOA si;
        PROCESS_INFORMATION pi;

        for (int i = 0; argv[i]; i++) {
            quoted = quote_arg(argv[i]);
            
            if (!quoted)
                return (-1);

            qlen = strlen(quoted);

            if (used + qlen + 2 >= sizeof(cmd)) {
                (void)free(quoted);
                return (-1);
            }

            if (i > 0)
                cmd[used++] = ' ';

            memcpy(cmd + used, quoted, qlen);
            used += qlen;

            (void)free(quoted);
        }

        cmd[used] = '\0';

        memset(&si, 0, sizeof(si));
        memset(&pi, 0, sizeof(pi));

        si.cb = sizeof(si);

        if (!CreateProcessA(program, cmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
            return (-1);

        WaitForSingleObject(pi.hProcess, INFINITE);
        GetExitCodeProcess(pi.hProcess, &exit_code);

        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

        return ((int)exit_code);
    #else
        int status;
        pid_t pid = fork();

        if (pid < 0)
            return (-1);


        if (pid == 0) {
            execvp(program, (char *const *)argv);

            _exit(127);
        }

        if (waitpid(pid, &status, 0) < 0)
            return (-1);

        if (WIFEXITED(status))
            return (WEXITSTATUS(status));

        return (-1);
    #endif
}

const char *path_basename(const char *path)
{
    const char *last_slash = path;
    const char *p = path;

    if (path == NULL || *path == '\0') {
        return (path);
    }

    while (*p) {
        if (*p == '/' || *p == '\\')
            last_slash = p;
        p++;
    }

    return ((*last_slash == '/' || *last_slash == '\\') ? last_slash + 1 : path);
}

const char *get_extension(const char *path)
{
    const char *last_slash = path;
    const char *last_dot = NULL;

    for (const char *p = path; *p; p++) {
        if (*p == '/' || *p == '\\') {
            last_slash = p;
            last_dot = NULL;
        } else if (*p == '.') {
            last_dot = p;
        }
    }

    if (last_dot && last_dot > last_slash)
        return (last_dot);

    return (NULL);
}

char *replace_extension(const char *path, const char *ext)
{
    const char *dot = get_extension(path);
    size_t base_len = dot ? (size_t)(dot - path) : strlen(path);
    int add_dot = (ext[0] != '.');
    size_t ext_len;
    char *out;
    char *p;

    if (!add_dot)
        ext++;

    ext_len = strlen(ext);
    out = malloc(base_len + ext_len + add_dot + 1);

    if (!out)
        return (NULL);

    memcpy(out, path, base_len);

    p = out + base_len;

    if (add_dot)
        *p++ = '.';

    memcpy(p, ext, ext_len);
    p[ext_len] = '\0';
    return (out);
}

uint8_t copy_file(const char *src, const char *dst)
{
    if (!src || !dst)
        return (1);

    unsigned char buffer[COPY_BUFFER_SIZE];
    FILE *in = fopen(src, "rb");
    FILE *out;
    size_t bytes;
    char *new_dst;

    if (!in)
        return (1);

    if (is_dir(dst)) {
        new_dst = join_path(dst, path_basename(src));
    } else {
        new_dst = strdup(dst);
    }

    if (!new_dst) {
        (void)fclose(in);
    }

    out = fopen(new_dst, "wb");

    if (!out) {
        (void)free(new_dst);
        (void)fclose(in);
        return (1);
    }

    while ((bytes = fread(buffer, 1, sizeof(buffer), in)) > 0) {
        if (fwrite(buffer, 1, bytes, out) != bytes) {
            (void)free(new_dst);
            (void)fclose(in);
            (void)fclose(out);
            return (1);
        }
    }

    (void)free(new_dst);
    (void)fclose(out);

    if (ferror(in)) {
        (void)fclose(in);
        return (1);
    }

    (void)fclose(in);
    return (0);
}

uint8_t copytree(const char *src, const char *dst)
{
    #ifdef _WIN32
        WIN32_FIND_DATAA fd;
        HANDLE h;

        char pattern[MAX_PATH];
        char *src_path;
        char *dst_path;

        if (!is_dir(dst)) {
            if (MKDIR(dst) != 0)
                return (1);
        }

        snprintf(pattern, sizeof(pattern), "%s\\*", src);

        h = FindFirstFileA(pattern, &fd);

        if (h == INVALID_HANDLE_VALUE)
            return (1);

        do {
            if (!strcmp(fd.cFileName, ".") || !strcmp(fd.cFileName, ".."))
                continue;

            src_path = join_path(src, fd.cFileName);
            dst_path = join_path(dst, fd.cFileName);

            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                if (copytree(src_path, dst_path) != 0) {
                    FindClose(h);
                    free(src_path);
                    free(dst_path);
                    return (1);
                }
            } else {
                if (copy_file(src_path, dst_path) != 0) {
                    FindClose(h);
                    free(src_path);
                    free(dst_path);
                    return (1);
                }
            }

            free(src_path);
            free(dst_path);
        } while (FindNextFileA(h, &fd));

        FindClose(h);
        return (0);
    #else
        DIR *dir;
        struct dirent *entry;

        char *src_path;
        char *dst_path;

        if (!is_dir(dst)) {
            if (MKDIR(dst) != 0)
                return (1);
        }

        dir = opendir(src);
    
        if (!dir)
            return (1);

        while ((entry = readdir(dir)) != NULL) {
            if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
                continue;

            src_path = join_path(src, entry->d_name);
            dst_path = join_path(dst, entry->d_name);

            if (is_dir(src_path)) {
                if (copytree(src_path, dst_path) != 0) {
                    closedir(dir);
                    free(src_path);
                    free(dst_path);
                    return (1);
                }
            } else {
                if (copy_file(src_path, dst_path) != 0) {
                    closedir(dir);
                    free(src_path);
                    free(dst_path);
                    return (1);
                }
            }
            free(src_path);
            free(dst_path);
        }

        closedir(dir);
        return (0);
    #endif
}