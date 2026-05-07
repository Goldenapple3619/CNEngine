project(cnassets C)

add_library(cnassets SHARED
    src/engine_object_writer.c
)

target_compile_options(cnassets
    PRIVATE
        -Wall -Wextra -Wshadow -Werror -O2
)

target_include_directories(cnassets
    PUBLIC
        include
)

target_link_libraries(cnassets
    PUBLIC
        cncore
)