#define SDL_MAIN_HANDLED

#include "engine.h"

int main(int argc, char *argv[])
{
    const ErrorContext *temp;
    int ret = argument_route((size_t)argc, (char **)argv);

    while (has_error()) {
        temp = get_error();

        (void)print_error(temp, NULL);
    }

    return (ret);
}