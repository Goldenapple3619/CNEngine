#ifndef _SCENE_TOOLCHAIN_H_
    #define _SCENE_TOOLCHAIN_H_

    #include "../build.h"

    #include "cnsceneobj.h"

    void delete_parsed_scene(struct scene_element_s *parsed_object);
    struct generic_vector_s *parse_scene(const char *file_path, struct engine_object_file_writer_ctx_s *wctx, Object *asset_ctx);

#endif