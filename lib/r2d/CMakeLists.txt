project(r2d C)

add_library(r2d SHARED
    src/2dboard.c
    src/object2d.c
    src/camera2d.c
)

target_compile_options(r2d
    PRIVATE
        -Wall -Wextra -Wshadow -Werror -O2
)

target_include_directories(r2d
    PUBLIC
        include
)

target_link_libraries(r2d
    PUBLIC
        cncore
        cngraphic
)