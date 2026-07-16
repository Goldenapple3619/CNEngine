#ifndef _LINKER_TOOLCHAIN_H_
    #define _LINKER_TOOLCHAIN_H_

    #include "../build.h"

    typedef struct {
        struct section_blk _s;
        struct asset_registry *asset_reg;
        CNAssetReader *reader;
    } BLKAssetStorage;

#endif