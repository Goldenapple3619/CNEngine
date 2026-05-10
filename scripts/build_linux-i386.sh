#!/bin/bash
set -e

mkdir -p build/linux-i386
cd build/linux-i386

cmake ../.. \
    -DDIST_DIR=../../dist/linux-i386 \
    -DCMAKE_C_STANDARD_LIBRARIES="-lm" \
    -DCMAKE_CXX_STANDARD_LIBRARIES="-lm" \
    -DSDL2_IMAGE=ON \
    -DSDL2_MIXER=ON \
    -DSDL2_TTF=ON \
    -DCMAKE_C_FLAGS="-m32" \
    -DCMAKE_CXX_FLAGS="-m32" \
    -DCMAKE_FIND_ROOT_PATH="/usr/lib/i386-linux-gnu /usr/i686-linux-gnu" \
    -DCMAKE_LIBRARY_PATH="/usr/lib/i386-linux-gnu" \
    -DTARGET_ARCH="i386" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_POLICY_VERSION_MINIMUM=3.5

cmake --build . -- -j$(nproc)