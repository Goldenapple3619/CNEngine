#include "test.h"

cn_value show_fps(Object *__this, void **args)
{
    if (!args || !args[0])
        return (null_value);

    double delta_time = *(double *)args[0];
    char fps_text[23];

    if (delta_time <= 0)
        return (null_value);

    snprintf(fps_text, sizeof(fps_text), "%.2f fps", 1000.0 / (double)delta_time);
    call_method(__this, "set_text", PACK_ARG(fps_text));
    return (null_value);
}
