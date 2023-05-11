#!/bin/bash

git submodule update --init --recursive

cd deps/flare-mono

./autogen.sh  
./configure --with-static_mono=yes --prefix=$PWD/build/
make -j6
make install