#!/bin/bash
set -e

mkdir -p build/macos-arm64
cd build/macos-arm64

cmake ../.. \
    -DDIST_DIR=../../dist/macos-arm64 \
    -DSDL2_IMAGE=ON \
    -DSDL2_MIXER=ON \
    -DSDL2_TTF=ON \
    -DTARGET_ARCH="arm64" \
    -DCMAKE_OSX_ARCHITECTURES=arm64 \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
    -DCN_SANITIZE=OFF

cmake --build . -- -j$(sysctl -n hw.ncpu)
