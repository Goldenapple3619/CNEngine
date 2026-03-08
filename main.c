#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <stdio.h>
#include <signal.h>
#include "libcncore.h"

static Object *global_ctx = NULL;

void sigint_handler(int signum)
{
    (void)signum;

    if (global_ctx && has_method(global_ctx, "_stop"))
        call_method(global_ctx, "_stop", NULL);
}

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    Object *ctx = new_ctx(NULL, NULL, NULL);

    if (!ctx)
        return (1);

    global_ctx = ctx;

    cn_value val = call_method(ctx, "_init", NULL);

    if (!val.type || val.as.i)
        return (1);
    signal(SIGINT, &sigint_handler);

    call_method(ctx, "_run", NULL);
    call_method(ctx, "_del", NULL);

    delete_object(ctx);

    // (void)SDL_SetMainReady();

    // if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
    //     printf("SDL_Init Error: %s\n", SDL_GetError());
    //     return 1;
    // }

    // SDL_Window *win = SDL_CreateWindow(
    //     "CN Engine",
    //     SDL_WINDOWPOS_CENTERED,
    //     SDL_WINDOWPOS_CENTERED,
    //     1280, 720,
    //     SDL_WINDOW_SHOWN
    // );

    // if (!win) {
    //     printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
    //     SDL_Quit();
    //     return 1;
    // }

    // SDL_Delay(2000);

    // SDL_DestroyWindow(win);
    // SDL_Quit();

    return (0);
}