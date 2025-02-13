#!/bin/bash

cc -o buildC -Ideps/CUBE/include/ build.c -O2
if [ $? -ne 0 ]; then
    echo "Build bootstrap failed"
    exit 1
fi

./buildC "$@"