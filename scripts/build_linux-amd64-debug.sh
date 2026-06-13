#!/bin/bash
set -e

mkdir -p build/linux-amd64-debug
cd build/linux-amd64-debug

cmake ../.. \
    -DDIST_DIR=../../dist/linux-amd64-debug \
    -DSDL2_IMAGE=ON \
    -DSDL2_MIXER=ON \
    -DSDL2_TTF=ON \
    -DCMAKE_C_FLAGS="-m64" \
    -DCMAKE_CXX_FLAGS="-m64" \
    -DTARGET_ARCH="amd64" \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
    -DCN_SANITIZE=ON

cmake --build . -- -j8
