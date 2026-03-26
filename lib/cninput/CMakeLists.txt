project(cninput C)

add_library(cninput SHARED
    src/common.c
)

target_compile_options(cninput
    PRIVATE
        -Wall -Wextra -Wshadow -Werror -O2
)

target_include_directories(cninput
    PUBLIC
        include
)

target_link_libraries(cninput
    PUBLIC
        cncore
        SDL2::SDL2
)