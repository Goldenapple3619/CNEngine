project(cnsceneobj C)

add_library(cnsceneobj SHARED
    src/interface.c
    src/tree_section.c
)

target_compile_options(cnsceneobj
    PRIVATE
        -Wall -Wextra -Wshadow -Werror -O2
)

target_include_directories(cnsceneobj
    PUBLIC
        include
)

target_link_libraries(cnsceneobj
    PUBLIC
        cnassets
)