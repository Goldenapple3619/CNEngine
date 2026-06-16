#include "engine_main.h"

static Object *global_ctx = NULL;

static void sigint_handler(int signum)
{
    (void)signum;

    if (global_ctx && has_method(global_ctx, "_stop"))
        call_method(global_ctx, "_stop", NULL);
}

static uint8_t new_message_box(FEEDBACK_STATUS_TYPE box_type, const char *title, const char *content, const Texture *icon)
{
    const Videomode v = {
        .flags = VDM_CPU & VDM_CLOSABLE,
        .size.x = strlen(content) * 12 + 120, .size.y = 120,
        .position.x = (SDL_WINDOWPOS_CENTERED), .position.y = (SDL_WINDOWPOS_CENTERED),
        .native_flags = VDM_N_SHWN
    };
    Window w = new_window(title, icon, &v);
    SDL_Color color = (SDL_Color){.r = 0x00, .g = 0x00, .b = 0x00, .a = 0xff};
    TTF_Font *font = TTF_OpenFont("assets/fonts/ConsolaMono-Book.ttf", 12);
    Texture *text;
    Vector2 pos_text = {.x = 120, .y = 50};
    Vector2 pos_icon = {.x = 10, .y = 10};

    if (!font)
        return (1);
    text = new_texture_from_surface(TTF_RenderUTF8_Blended(font, content, color));

    blit(w.texture, text, NULL, &pos_text);

    if (icon)
        blit(w.texture, icon, NULL, &pos_icon);
    return (0);
}

static void handle_error(Object *ctx, uint32_t error_code, const char *what)
{
    const char *error_messages[] = {
        "allocation failed, out of memory: %s.",
        "uknown error occured."
    };

    if (error_code >= sizeof(error_messages) / sizeof(char *))
        error_code = (sizeof(error_messages) / sizeof(char *)) - 1;

    (void)printf(error_messages[error_code], what);

    if (!ctx) {
        return;
    }

    call_method(ctx, "_stop", NULL);
    DELOC(ctx);

    #if defined(_ENGINE_HAS_GRAPHICS) && (_ENGINE_HAS_GRAPHICS == 1) && defined(_ENGINE_HAS_GUI) && (_ENGINE_HAS_GUI == 1)
        SDL_Init(SDL_INIT_VIDEO);
        TTF_Init();
        IMG_Init(IMG_INIT_PNG);

        if (new_message_box(FDS_ERROR, "Fatal error occured", error_messages[error_code], NULL)) {
            #if defined(_WIN32)
                MessageBox(
                    NULL,
                    error_messages[error_code],
                    "Fatal error occured",
                    MB_OK | MB_ICONERROR
                );
            #endif
        }
    #endif

    exit(1);
}

int main(size_t argc, char **argv)
{
    (void)argc;
    (void)argv;

    Object *ctx = new_ctx();
    cn_value temp_val;

    if (!ctx)
        return (1);

    global_ctx = ctx;

    signal(SIGINT, &sigint_handler);

    if (load_submodules(ctx)) {
        delete_object(ctx);
        return (1);
    }

    temp_val = call_method(ctx, "_init", NULL);

    if (temp_val.type == CN_TYPE_NULL || temp_val.as.i == VALUE_ERR.as.i) {
        handle_error(ctx, 0x00, "Initialisation failed.");
    }

    call_method(ctx, "_run", NULL);
    call_method(ctx, "_stop", NULL);
    DELOC(ctx);

    return (0);
}
