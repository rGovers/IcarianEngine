#!/bin/bash

git submodule update --init --recursive

# ./setup.sh

cd deps/flare-mono

./autogen.sh 
./configure --prefix=$PWD/crossbuild --host=x86_64-w64-mingw32 --disable-boehm --with-static_mono=yes 
make clean
make -j6
make install

# ./configure --prefix=$PWD/crossbuild32 --host=i686-w64-mingw32 --disable-boehm --disable-mcs-build --with-static_mono=yes 
# make -j6
# make install