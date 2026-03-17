#!/bin/bash
set -e

mkdir -p build/windows-amd64
cd build/windows-amd64

cmake ../.. -DCMAKE_TOOLCHAIN_FILE=../../toolchain/mingw_toolchain-amd64.cmake \
    -DDIST_DIR=../../dist/windows-amd64 \
    -DSDL2_IMAGE=ON \
    -DSDL2_MIXER=ON \
    -DSDL2_TTF=ON \
    -DCMAKE_C_FLAGS="-m64" \
    -DCMAKE_CXX_FLAGS="-m64" \
    -DTARGET_ARCH="amd64" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
    -DCMAKE_FIND_ROOT_PATH="/usr/x86_64-w64-mingw32"

cmake --build . -- -j8
