project(rgui C)

add_library(rgui SHARED
    src/guiboard.c
    src/gui_object.c
    src/text.c
    src/common.c
)

target_compile_options(rgui
    PRIVATE
        -Wall -Wextra -Wshadow -Werror -O2
)

target_include_directories(rgui
    PUBLIC
        include
)

target_link_libraries(rgui
    PUBLIC
        cncore
        cngraphic
        SDL2::SDL2
        SDL2_ttf::SDL2_ttf
)