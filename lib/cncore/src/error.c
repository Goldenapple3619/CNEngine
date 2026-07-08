#include "libcncore.h"

_Thread_local ErrorContext g_error_history[ERR_HISTORY] = {0};
_Thread_local uint32_t g_error_history_index = 0;

CN_API void raise_error(ErrorCode c, const char *message, const char *file, const char *function, uint32_t line)
{
    if (g_error_history_index >= ERR_HISTORY) {
        (void)fprintf(stderr, "out of error history, direct printing to stderr: %d: %s: %s %s %" PRIu32 "\n", c, message ? message : "null", file, function, line);
        return;
    }

    g_error_history[g_error_history_index].code = c;
    g_error_history[g_error_history_index].frame_count = 0;

    if (g_error_history[g_error_history_index].frame_count >= ERR_MAX_FRAMES) {
        (void)fprintf(stderr, "out of error frames, direct printing to stderr: %s %s %" PRIu32 "\n", file, function, line);
        return;
    }

    if (message) {
        if (strlen(message) + 1 >= ERR_MSG_SIZE) {
            (void)fprintf(stderr, "error message too large, going to be truncated, printing to stderr: %s\n", message);
        }

        (void)snprintf(g_error_history[g_error_history_index].message, ERR_MSG_SIZE, "%s", message);
    } else {
        (void)memset((void *)g_error_history[g_error_history_index].message, 0, sizeof(char) * ERR_MSG_SIZE);
    }

    g_error_history[g_error_history_index].frames[g_error_history[g_error_history_index].frame_count].line = line;
    g_error_history[g_error_history_index].frames[g_error_history[g_error_history_index].frame_count].file = file;
    g_error_history[g_error_history_index].frames[g_error_history[g_error_history_index].frame_count].function = function;
    g_error_history[g_error_history_index].frame_count += 1;

    ++g_error_history_index;
}

CN_API void raise_error_fmt(ErrorCode c, const char *file, const char *function, uint32_t line, const char *fmt, ...)
{
    if (g_error_history_index >= ERR_HISTORY) {
        (void)fprintf(stderr, "out of error history, direct printing to stderr: %d: %s: %s %s %" PRIu32 "\n", c, fmt ? fmt : "null", file, function, line);
        return;
    }

    g_error_history[g_error_history_index].code = c;
    g_error_history[g_error_history_index].frame_count = 0;

    if (g_error_history[g_error_history_index].frame_count >= ERR_MAX_FRAMES) {
        (void)fprintf(stderr, "out of error frames, direct printing to stderr: %s %s %" PRIu32 "\n", file, function, line);
        return;
    }

    if (fmt) {
        va_list args;
        va_start(args, fmt);

        vsnprintf(g_error_history[g_error_history_index].message, ERR_MSG_SIZE, fmt, args);

        va_end(args);
    } else {
        (void)memset((void *)g_error_history[g_error_history_index].message, 0, sizeof(char) * ERR_MSG_SIZE);
    }

    g_error_history[g_error_history_index].frames[g_error_history[g_error_history_index].frame_count].line = line;
    g_error_history[g_error_history_index].frames[g_error_history[g_error_history_index].frame_count].file = file;
    g_error_history[g_error_history_index].frames[g_error_history[g_error_history_index].frame_count].function = function;
    g_error_history[g_error_history_index].frame_count += 1;

    ++g_error_history_index;
}

CN_API void push_error(const char *file, const char *function, uint32_t line)
{
    if (g_error_history_index == 0) {
        (void)fprintf(stderr, "frame written with no history, direct printing to stderr: %s %s %" PRIu32 "\n", file, function, line);
        return;
    }

    if (g_error_history[g_error_history_index - 1].frame_count >= ERR_MAX_FRAMES) {
        (void)fprintf(stderr, "out of error frames, direct printing to stderr: %s %s %" PRIu32 "\n", file, function, line);
        return;
    }

    g_error_history[g_error_history_index - 1].frames[g_error_history[g_error_history_index - 1].frame_count].line = line;
    g_error_history[g_error_history_index - 1].frames[g_error_history[g_error_history_index - 1].frame_count].file = file;
    g_error_history[g_error_history_index - 1].frames[g_error_history[g_error_history_index - 1].frame_count].function = function;
    g_error_history[g_error_history_index - 1].frame_count += 1;
}

CN_API const ErrorContext *get_error(void)
{
    if (g_error_history_index != 0) {
        g_error_history_index -= 1;

        return (&(g_error_history[g_error_history_index]));
    }

    return (NULL);
}

CN_API cnbool has_error(void)
{
    return (g_error_history_index != 0 ? true : false);
}

CN_API const char *error_type_to_text(ErrorCode c)
{
    const char *errornames[] = {
        "OK",
        "OUT_OF_MEMORY",
        "INVALID_POINTER",
        "OS_ERROR",
        "OUT_OF_BOUND",
        "INVALID_TYPE",
        "INCOMPATIBLE",
        "RUNTIME_ERROR"
    };

    if (c >= (sizeof(errornames) / sizeof(char *)))
        return ("UKNOWN_ERROR");

    return (errornames[(size_t)c]);
}

CN_API void print_error(const ErrorContext *err, FILE *output)
{
    if (!err)
        return;

    if (!output) {
        printf("[%s] %s\n", error_type_to_text(err->code), err->message);
        for (size_t i = 0; i < err->frame_count; ++i) {
            if (i != (err->frame_count - 1))
                printf("  |-[%zu] %s@%s:%" PRIu32 "\n", err->frame_count - i, err->frames[i].function ? err->frames[i].function : "???", err->frames[i].file ? err->frames[i].file : "???", err->frames[i].line);
            else
                printf("  \\-[%zu] %s@%s:%" PRIu32 "\n", err->frame_count - i, err->frames[i].function ? err->frames[i].function : "???", err->frames[i].file ? err->frames[i].file : "???", err->frames[i].line);
        }
    } else {
        fprintf(output, "[%s] %s\n", error_type_to_text(err->code), err->message);
        for (size_t i = 0; i < err->frame_count; ++i) {
            if (i != (err->frame_count - 1))
                fprintf(output, "  |-[%zu] %s@%s:%" PRIu32 "\n", err->frame_count - i, err->frames[i].function ? err->frames[i].function : "???", err->frames[i].file ? err->frames[i].file : "???", err->frames[i].line);
            else
                fprintf(output, "  \\-[%zu] %s@%s:%" PRIu32 "\n", err->frame_count - i, err->frames[i].function ? err->frames[i].function : "???", err->frames[i].file ? err->frames[i].file : "???", err->frames[i].line);
        }
    }
}
