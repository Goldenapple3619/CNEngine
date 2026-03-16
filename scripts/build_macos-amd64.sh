#!/bin/bash
set -e

mkdir -p build/macos-amd64
cd build/macos-amd64

cmake ../.. \
    -DDIST_DIR=../../dist/macos-amd64 \
    -DSDL2_IMAGE=ON \
    -DSDL2_MIXER=ON \
    -DSDL2_TTF=ON \
    -DCMAKE_C_FLAGS="-m64" \
    -DCMAKE_CXX_FLAGS="-m64" \
    -DTARGET_ARCH="amd64" \
    -DCMAKE_POLICY_VERSION_MINIMUM=3.5

cmake --build . -- -j8
