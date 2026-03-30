#!/bin/bash
set -e

mkdir -p build/macos-arm64
cd build/macos-arm64

cmake ../.. \
    -DDIST_DIR=../../dist/macos-arm64 \
    -DSDL2_IMAGE=ON \
    -DSDL2_MIXER=ON \
    -DSDL2_TTF=ON \
    -DCMAKE_C_FLAGS="-m64" \
    -DCMAKE_CXX_FLAGS="-m64" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_POLICY_VERSION_MINIMUM=3.5

cmake --build . -- -j8
