cmake_minimum_required(VERSION 3.20)

project(CNEngine
    VERSION 0.1
    LANGUAGES C
)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_REQUIRED_LIBRARIES m CACHE BOOL "" FORCE)

# ========= Options =========
option(CN_BUILD_ENGINE "Build engine executable" ON)

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

set(SDL2_TEST OFF CACHE BOOL "" FORCE)
set(SDL_TEST OFF CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL2IMAGE_SAMPLES OFF CACHE BOOL "" FORCE)
set(SDL2IMAGE_TESTS OFF CACHE BOOL "" FORCE)
set(SDL2TTF_SAMPLES OFF CACHE BOOL "" FORCE)
set(SDL2TTF_TESTS OFF CACHE BOOL "" FORCE)
set(SDL2MIXER_SAMPLES OFF CACHE BOOL "" FORCE)
set(SDL2MIXER_TESTS OFF CACHE BOOL "" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(BUILD_DOCS OFF CACHE BOOL "" FORCE)

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

set(DAV1D_VENDORED ON CACHE BOOL "" FORCE)

set(SDL2IMAGE_AVIF OFF CACHE BOOL "" FORCE)
set(SDL2IMAGE_WEBP OFF CACHE BOOL "" FORCE)

set(SDL2IMAGE_PNG ON CACHE BOOL "" FORCE)
set(SDL2IMAGE_JPG ON CACHE BOOL "" FORCE)

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

FetchContent_Declare(
    glad
    GIT_REPOSITORY https://github.com/Dav1dde/glad.git
    GIT_TAG v2.0.6
    GIT_SHALLOW TRUE
    SOURCE_SUBDIR cmake
)

FetchContent_GetProperties(glad)
if(NOT glad_POPULATED)
    FetchContent_Populate(glad)
    add_subdirectory(${glad_SOURCE_DIR}/cmake ${glad_BINARY_DIR})
endif()

glad_add_library(glad_gl STATIC
    LANGUAGE C
    API gl:core=4.6
)

FetchContent_MakeAvailable(glad)

find_package(OpenGL REQUIRED)

# ========= Modules =========
add_subdirectory(lib/cncore)
add_subdirectory(lib/cngraphic)
add_subdirectory(lib/cnaudio)
add_subdirectory(lib/cninput)
add_subdirectory(lib/rgui)
add_subdirectory(lib/r2d)
add_subdirectory(lib/r3d)

# dist deploy
add_custom_target(cn_dist ALL)

# ========= engine executable =========
if(CN_BUILD_ENGINE)
    add_dependencies(cn_dist cn_engine)

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
            cninput

            rgui
            r2d
            r3d

            m
    )

    target_include_directories(cn_engine
        PRIVATE
            lib/cncore/include
            lib/cngraphic/include
            lib/cnaudio/include
            lib/cninput/include

            lib/rgui/include
            lib/r2d/include
            lib/r3d/include
    )

    # main
    add_custom_command(TARGET cn_dist POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${DIST_DIR}/bin"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:cn_engine>
            "${DIST_DIR}/bin/"
    )
endif()

# SDL2
foreach(lib SDL2 SDL2_image SDL2_mixer SDL2_ttf)
    add_custom_command(TARGET cn_dist POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${DIST_DIR}/lib"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:${lib}>
            "${DIST_DIR}/lib/"
    )
endforeach()

#gl
foreach(lib glad_gl)
    add_custom_command(TARGET cn_dist POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${DIST_DIR}/lib"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:${lib}>
            "${DIST_DIR}/lib/"
    )
endforeach()

# vendored
foreach(lib freetype harfbuzz ogg opus opusfile xmp tiff)
    if(TARGET ${lib})
        get_target_property(lib_type ${lib} TYPE)
        if(lib_type STREQUAL "SHARED_LIBRARY")
            add_custom_command(TARGET cn_dist POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory "${DIST_DIR}/lib"
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    $<TARGET_FILE:${lib}>
                    "${DIST_DIR}/lib/"
            )
        endif()
    endif()
endforeach()

# includes
foreach(lib cncore cngraphic cnaudio cninput rgui r2d r3d)
    add_custom_command(TARGET cn_dist POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/lib/${lib}/include"
            "${DIST_DIR}/include/"
    )
endforeach()

# vendored includes

foreach(pair
    "sdl2_SOURCE_DIR/include"
)
    # Split into varname and subdir
    string(REPLACE "/" ";" pair_list "${pair}")
    list(GET pair_list 0 _var)
    list(GET pair_list 1 _subdir)

    set(_dir "${${_var}}/${_subdir}")
    if(EXISTS "${_dir}")
        add_custom_command(TARGET cn_dist POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${DIST_DIR}/include/SDL2"
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${_dir}"
                "${DIST_DIR}/include/SDL2"
        )
    else()
        message(WARNING "SDL include dir not found: ${_dir}")
    endif()
endforeach()

add_custom_command(TARGET cn_dist POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${sdl2_BINARY_DIR}/include"
        "${DIST_DIR}/include/"
)

foreach(pair
    "sdl2_image_SOURCE_DIR/include"
    "sdl2_mixer_SOURCE_DIR/include"
)
    string(REPLACE "/" ";" pair_list "${pair}")
    list(GET pair_list 0 _var)
    list(GET pair_list 1 _subdir)

    set(_dir "${${_var}}/${_subdir}")
    if(EXISTS "${_dir}")
        add_custom_command(TARGET cn_dist POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "${DIST_DIR}/include"
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${_dir}"
                "${DIST_DIR}/include"
        )
    else()
        message(WARNING "SDL include dir not found: ${_dir}")
    endif()
endforeach()

add_custom_command(TARGET cn_dist POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${DIST_DIR}/include"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${sdl2_ttf_SOURCE_DIR}/SDL_ttf.h"
        "${DIST_DIR}/include"
)

foreach(lib glad_gl)
    get_target_property(_inc_dirs ${lib} INTERFACE_INCLUDE_DIRECTORIES)
    if(_inc_dirs)
        foreach(_dir ${_inc_dirs})
            if(EXISTS "${_dir}")
                add_custom_command(TARGET cn_dist POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_directory
                        "${_dir}"
                        "${DIST_DIR}/include/"
                )
            endif()
        endforeach()
    endif()
endforeach()

# modules libraries
foreach(lib cncore cngraphic cnaudio cninput rgui r2d r3d)
    add_custom_command(TARGET cn_dist POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:${lib}>
            "${DIST_DIR}/lib/"
    )
endforeach()

# machine specific libs
if(WIN32)
    get_filename_component(MINGW_BIN_DIR ${CMAKE_C_COMPILER} DIRECTORY)

    if(TARGET_ARCH STREQUAL "i386")
        set(MINGW_RUNTIME_DLLS libwinpthread-1.dll libgcc_s_dw2-1.dll)
    else()
        set(MINGW_RUNTIME_DLLS libwinpthread-1.dll libgcc_s_seh-1.dll)
    endif()

    foreach(dll ${MINGW_RUNTIME_DLLS})
        if (EXISTS "${MINGW_BIN_DIR}/${dll}")
            add_custom_command(TARGET cn_dist POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory "${DIST_DIR}/lib"
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${MINGW_BIN_DIR}/${dll}"
                    "${DIST_DIR}/lib/"
            )
        else()
            message(STATUS "Skipping ${dll} - not found in ${MINGW_BIN_DIR}")
        endif()
    endforeach()
endif()