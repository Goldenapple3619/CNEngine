mkdir -p build\windows-amd64
pushd build\windows-amd64

cmake -G "MinGW Makefiles" ..\.. -DCMAKE_TOOLCHAIN_FILE=..\..\toolchain\mingw_toolchain-amd64.cmake -DDIST_DIR=..\..\dist\windows-amd64 -DSDL2_IMAGE=ON -DSDL2_MIXER=ON -DSDL2_TTF=ON -DCMAKE_C_FLAGS="-m64" -DCMAKE_CXX_FLAGS="-m64" -DTARGET_ARCH="amd64" -DCMAKE_BUILD_TYPE=Release -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DCN_SANITIZE=OFF -DCMAKE_FIND_ROOT_PATH="C:/msys64/mingw64"

cmake --build . -- -j8
