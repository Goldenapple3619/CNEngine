project(r3d C)

find_package(OpenGL REQUIRED)

add_library(r3d SHARED
    src/3dboard.c
    src/camera3d.c
)

target_compile_options(r3d
    PRIVATE
        -Wall -Wextra -Wshadow -Werror -O2
)

target_include_directories(r3d
    PUBLIC
        include
)

target_link_libraries(r3d
    PUBLIC
        cncore
        cngraphic
        SDL2::SDL2
        OpenGL::GL
)