#define SDL_MAIN_HANDLED

#include "engine.h"

int main(int argc, char *argv[])
{
    return (argument_route((size_t)argc, (char **)argv));
}