#include "libcninput.h"

InputController *new_input_controller(input_type target_type, int64_t target_value, cnbool value_ignored)
{
    InputController *controller = (InputController *)malloc(sizeof(InputController));

    if (!controller)
        return (NULL);
    controller->target_type = target_type;
    controller->target_value = target_value;
    controller->ignore_value = value_ignored;
    return (controller);
}

input_type input_type_from_event(cn_event ev)
{
    switch (ev) {
        case EV_KEYDOWN:
        case EV_KEYUP:
            return (INPUT_KEY_PRESS);

        default:
            return (INPUT_UKN);
    }
}

cnbool input_controller_cmp(const InputController *ic, const Event *ev)
{
    if (input_type_from_event(ev->type) != ic->target_type)
        return (false);

    if (ic->ignore_value && ev->v != ic->target_value)
        return (false);

    return (true);
}

void delete_input_controller(InputController *ic)
{
    if (!ic)
        return;
    (void)free(ic);
}
