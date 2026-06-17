#include "project_toolchain.h"

CNAsset *new_cnasset(const char *asset_location, cnasset_type tp)
{
    CNAsset *asset = malloc(sizeof(CNAsset));

    if (!asset)
        return (NULL);

    asset->location = asset_location ? strdup(asset_location) : NULL;
    asset->type = tp;

    return (asset);
}

void delete_cnasset(CNAsset *ptr)
{
    if (!ptr)
        return;
    if (ptr->location)
        (void)free(ptr->location);
    (void)free(ptr);
}
