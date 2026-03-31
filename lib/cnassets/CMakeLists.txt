project(cnassets C)

add_library(cnassets SHARED

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

)