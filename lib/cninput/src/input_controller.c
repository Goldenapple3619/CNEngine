#include "libcninput.h"

InputController *new_input_controller(input_type target_type, int64_t target_value)
{
    InputController *controller = (InputController *)malloc(sizeof(InputController));

    if (!controller)
        return (NULL);
    controller->target_type = target_type;
    controller->target_value = target_value;
    return (controller);
}

void delete_input_controller(InputController *ic)
{
    if (!ic)
        return;
    (void)free(ic);
}
