#include "cnobjectobj.h"

CN_API struct asset_registry *register_asset(void)
{
    struct asset_registry *object_asset = (struct asset_registry *)malloc(sizeof(struct asset_registry));

    if (!object_asset)
        return (NULL);

    object_asset->name = "object";
    object_asset->asset_type = ENGINE_OBJ_OBJ;
    object_asset->registered_sections.capacity = 0;
    object_asset->registered_sections.size = 0;
    object_asset->registered_sections.content = NULL;

    return (object_asset);
}

CN_API void unregister_asset(struct asset_registry *registered_asset)
{
    if (!registered_asset)
        return;
    if (registered_asset->registered_sections.content)
        (void)empty_generic_vector(&registered_asset->registered_sections, &free);
    (void)free(registered_asset);
}
