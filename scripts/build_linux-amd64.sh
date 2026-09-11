#!/bin/bash
set -e

CN_DEV_BUILD=OFF

while [[ $# -gt 0 ]]; do
    case "$1" in
        --dev)
            CN_DEV_BUILD=ON
            shift
            ;;
        --no-dev)
            CN_DEV_BUILD=OFF
            shift
            ;;
        *)
            echo "Unknown argument: $1"
            exit 1
            ;;
    esac
done

mkdir -p build/linux-amd64
cd build/linux-amd64

cmake ../.. \
    -DDIST_DIR=../../dist/linux-amd64 \
    -DSDL2_IMAGE=ON \
    -DSDL2_MIXER=ON \
    -DSDL2_TTF=ON \
    -DCMAKE_C_FLAGS="-m64" \
    -DCMAKE_CXX_FLAGS="-m64" \
    -DTARGET_ARCH="amd64" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
    -DCN_SANITIZE=OFF \
    -DCN_DEV_BUILD=${CN_DEV_BUILD}

cmake --build . -- -j$(nproc)
