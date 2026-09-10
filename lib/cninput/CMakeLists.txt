project(cninput C)

add_library(cninput SHARED
    src/input_submodule_ctx.c
    src/input_entry.c
    src/input_controller.c
    src/common.c
)

if(CN_BUILD_TESTS)
    target_compile_options(cninput
        PRIVATE
            --coverage -O0 -g
    )
    target_link_options(cninput PRIVATE --coverage)
else()
    target_compile_options(cninput
        PRIVATE
            -Wall -Wextra -Wshadow -Werror -O2
    )
endif()

target_include_directories(cninput
    PUBLIC
        include
)

target_link_libraries(cninput
    PUBLIC

    cncore
    cngraphic
)