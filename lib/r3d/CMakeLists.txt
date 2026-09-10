project(r3d C)

find_package(OpenGL REQUIRED)

add_library(r3d SHARED
    src/3dboard.c
    src/camera3d.c
    src/object3d.c
    src/projection3d.c
)

if(CN_BUILD_TESTS)
    target_compile_options(r3d
        PRIVATE
            --coverage -O0 -g
    )
    target_link_options(r3d PRIVATE --coverage)
else()
    target_compile_options(r3d
        PRIVATE
            -Wall -Wextra -Wshadow -Werror -O2
    )
endif()

target_include_directories(r3d
    PUBLIC
        include
)

target_link_libraries(r3d
    PUBLIC
        cncore
        cngraphic
)