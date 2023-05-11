#!/bin/bash

if [ ! -d "build" ]; then
    mkdir build
fi
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS=-O3 -DGENERATE_CONFIG=ON ../IcarianNative/
make -j6
../deps/flare-mono/build/bin/xbuild ../IcarianCS/IcarianCS.csproj /p:Configuration=Release

cd ../bin/
cp -r ../deps/flare-mono/build/lib .
cp -r ../deps/flare-mono/build/etc .
mv IcarianCS.exe IcarianCS.dll