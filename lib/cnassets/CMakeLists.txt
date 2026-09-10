project(cnassets C)

add_library(cnassets SHARED
    src/engine_object_writer.c
    src/engine_object_reader.c
    src/io_handler.c
    src/fmt_loader.c
    src/asset_ctx.c
    src/engine_types.c
)

if(CN_BUILD_TESTS)
    target_compile_options(cnassets
        PRIVATE
            --coverage -O0 -g
    )
    target_link_options(cnassets PRIVATE --coverage)
else()
    target_compile_options(cnassets
        PRIVATE
            -Wall -Wextra -Wshadow -Werror -O2
    )
endif()

target_include_directories(cnassets
    PUBLIC
        include
)

target_link_libraries(cnassets
    PUBLIC
        cncore
)