cmake_minimum_required(VERSION 3.20)

project(CNEngine
    VERSION 0.1
    LANGUAGES C
)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_REQUIRED_LIBRARIES m CACHE BOOL "" FORCE)

# ========= Options =========
option(CN_BUILD_GAME "Build game executable" ON)

if(NOT DEFINED DIST_DIR)
    get_filename_component(CWD_NAME ${CMAKE_SOURCE_DIR} NAME)
    set(DIST_DIR "${CMAKE_SOURCE_DIR}/dist/${CWD_NAME}")
endif()

if(NOT DEFINED TARGET_ARCH)
    if(CMAKE_SIZEOF_VOID_P EQUAL 4)
        set(TARGET_ARCH "i386")
    elseif(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(TARGET_ARCH "amd64")
    endif()
endif()

# ========= Fetch SDL2 =========
include(FetchContent)

set(SDL2_DISABLE_SDL2MAIN ON CACHE BOOL "" FORCE)
set(SDL_SHARED ON CACHE BOOL "" FORCE)
set(SDL_STATIC OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    SDL2
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-2.32.10
)
FetchContent_MakeAvailable(SDL2)

set(SDL2IMAGE_VENDORED ON CACHE BOOL "" FORCE)
set(SDL2IMAGE_EXTERNAL_PROJECT_BUILD_ALWAYS ON)

set(LIBAVIF_VENDORED ON CACHE BOOL "" FORCE)
set(DAV1D_VENDORED ON CACHE BOOL "" FORCE)

set(SDL2IMAGE_AVIF OFF CACHE BOOL "" FORCE)
set(SDL2IMAGE_WEBP OFF CACHE BOOL "" FORCE)

if(WIN32)
    if(TARGET_ARCH STREQUAL "i386")
        set(SDL2IMAGE_EXTERNAL_PROJECT_CMAKE_ARGS
            -DCMAKE_TOOLCHAIN_FILE=${CMAKE_CURRENT_LIST_DIR}/toolchain/mingw_toolchain-i386.cmake
            -DCMAKE_BUILD_TYPE=Release
        )
    elseif(TARGET_ARCH STREQUAL "amd64")
        set(SDL2IMAGE_EXTERNAL_PROJECT_CMAKE_ARGS
            -DCMAKE_TOOLCHAIN_FILE=${CMAKE_CURRENT_LIST_DIR}/toolchain/mingw_toolchain-amd64.cmake
            -DCMAKE_BUILD_TYPE=Release
        )
    endif()
else()
    set(SDL2IMAGE_EXTERNAL_PROJECT_CMAKE_ARGS
        -DCMAKE_BUILD_TYPE=Release
    )
endif()

FetchContent_Declare(
    SDL2_image
    GIT_REPOSITORY https://github.com/libsdl-org/SDL_image.git
    GIT_TAG release-2.8.8
)
FetchContent_MakeAvailable(SDL2_image)

set(SDL2TTF_VENDORED ON CACHE BOOL "" FORCE)
set(SDL2TTF_HARFBUZZ ON CACHE BOOL "" FORCE)

FetchContent_Declare(
    SDL2_ttf
    GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf.git
    GIT_TAG release-2.24.0
)
FetchContent_MakeAvailable(SDL2_ttf)

set(SDL2MIXER_WAVPACK OFF CACHE BOOL "" FORCE)
set(SDL2MIXER_VENDORED ON CACHE BOOL "" FORCE)

if(TARGET_ARCH STREQUAL "i386")
    set(SDL2MIXER_EXTERNAL_PROJECT_CMAKE_ARGS
        -DCMAKE_C_FLAGS="-m32"
        -DCMAKE_CXX_FLAGS="-m32"
        -DCMAKE_EXE_LINKER_FLAGS="-m32 -lm"
        -DCMAKE_SHARED_LINKER_FLAGS="-m32 -lm"
        -DCMAKE_BUILD_TYPE=Release
    )
else()
    set(SDL2MIXER_EXTERNAL_PROJECT_CMAKE_ARGS
        -DCMAKE_BUILD_TYPE=Release
    )
endif()

FetchContent_Declare(
    SDL2_mixer
    GIT_REPOSITORY https://github.com/libsdl-org/SDL_mixer.git
    GIT_TAG release-2.8.1
)

FetchContent_MakeAvailable(SDL2_mixer)

# ========= Modules =========
add_subdirectory(lib/cncore)
add_subdirectory(lib/cngraphic)
add_subdirectory(lib/cnaudio)
# add_subdirectory(lib/cninput)

# ========= Game executable =========
if(CN_BUILD_GAME)
    add_executable(cn_engine
        src/main.c
    )

    target_compile_options(cn_engine
        PRIVATE
            -Wall -Wextra -Wshadow -Werror -O2
    )

    target_link_libraries(cn_engine
        PRIVATE
            cncore
            cngraphic
            cnaudio
            m
            # cninput
    )

    target_include_directories(cn_engine
        PRIVATE
            lib/cncore/include
            lib/cngraphic/include
            lib/cnaudio/include
            # lib/cninput/include
    )

    # main
    add_custom_command(TARGET cn_engine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${DIST_DIR}/bin"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:cn_engine>
            "${DIST_DIR}/bin/"
    )

    # SDL2
    foreach(lib SDL2 SDL2_image SDL2_mixer SDL2_ttf)
        add_custom_command(TARGET cn_engine POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "${DIST_DIR}/lib"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:${lib}>
                "${DIST_DIR}/lib/"
        )
    endforeach()

    # includes
    foreach(lib cncore cngraphic cnaudio)
        add_custom_command(TARGET cn_engine POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${CMAKE_SOURCE_DIR}/lib/${lib}/include"
                "${DIST_DIR}/include/"
        )
    endforeach()

    # modules libraries
    foreach(lib cncore cngraphic cnaudio)
        add_custom_command(TARGET cn_engine POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:${lib}>
                "${DIST_DIR}/lib/"
        )
    endforeach()
endif()