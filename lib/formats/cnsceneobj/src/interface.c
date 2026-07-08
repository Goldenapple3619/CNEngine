#include "cnsceneobj.h"

CN_API struct asset_registry *register_asset(void)
{
    struct asset_registry *scene_asset = (struct asset_registry *)malloc(sizeof(struct asset_registry));
    struct section_registry *temp_section_registry;

    if (!scene_asset)
        return (NULL);

    scene_asset->name = "scene";
    scene_asset->asset_type = ENGINE_OBJ_SCN;
    scene_asset->registered_sections.capacity = 0;
    scene_asset->registered_sections.size = 0;
    scene_asset->registered_sections.content = NULL;

    temp_section_registry = (struct section_registry *)malloc(sizeof(struct section_registry));

    if (!temp_section_registry) {
        (void)free(scene_asset);
        return (NULL);
    }

    (void)init_tree_section_registry(temp_section_registry);
    
    if (insert_generic_vector(&scene_asset->registered_sections, temp_section_registry)) {
        (void)free(temp_section_registry);
        (void)free(scene_asset);
        return (NULL);
    }

    return (scene_asset);
}

CN_API void unregister_asset(struct asset_registry *registered_asset)
{
    if (!registered_asset)
        return;
    if (registered_asset->registered_sections.content)
        (void)empty_generic_vector(&registered_asset->registered_sections, &free);
    (void)free(registered_asset);
}
