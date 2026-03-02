mkdir build-windows
cd build-windows

cmake .. -DCMAKE_TOOLCHAIN_FILE=../mingw_toolchain.cmake \
    -DSDL2_IMAGE=ON \
    -DSDL2_MIXER=ON \
    -DSDL2_TTF=ON \
    -DCMAKE_POLICY_VERSION_MINIMUM=3.5

cmake --build . -- -j8
