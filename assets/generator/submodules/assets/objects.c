#include "libcnassets.h"
#include "libcncore.h"

cn_value _load_object(Object *__this, void **args)
{
    if (!args || !args[0]) {
        RAISE(ERR_INVALID_POINTER, "can't load unspecified object.");
        return (VALUE_NULL);
    }

    return (VALUE_NULL);
}
