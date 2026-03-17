#!/bin/bash
set -e

mkdir -p build/windows-i386
cd build/windows-i386

cmake ../.. \
    -DCMAKE_TOOLCHAIN_FILE=../../toolchain/mingw_toolchain-i386.cmake \
    -DDIST_DIR=../../dist/windows-i386 \
    -DSDL2_IMAGE=ON \
    -DSDL2_MIXER=ON \
    -DSDL2_TTF=ON \
    -DCMAKE_C_FLAGS="-m32" \
    -DCMAKE_CXX_FLAGS="-m32" \
    -DTARGET_ARCH="i386" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_POLICY_VERSION_MINIMUM=3.20 \
    -DCMAKE_FIND_ROOT_PATH="/usr/i686-w64-mingw32"

cmake --build . -- -j8