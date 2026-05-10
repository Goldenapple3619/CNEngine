#include "init.h"

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

    if (!buf)
        return (NULL);

    while ((c = getchar()) != EOF && c != '\n') {
        if ((size + 1) >= capacity) {
            capacity *= 2;
            tmp = realloc(buf, capacity);
    
            if (!tmp) {
                (void)free(buf);
                return (NULL);
            }
    
            buf = tmp;
        }

        buf[size++] = (char)c;
    }

    if (c == EOF && size == 0) {
        (void)free(buf);
        return (NULL);
    }

    buf[size] = '\0';

    tmp = realloc(buf, size + 1);
    return (tmp ? tmp : buf);
}

int init(size_t argc, char **argv)
{
    (void)argc;
    (void)argv;

    char *name = get_input("project name: ");

    if (!name) {
        fprintf(stderr, "cancelled.\n");
        return (1);
    }

    printf("<data: %s, >", name);
    (void)free(name);
    return (0);
}
