#!/bin/bash

cc -std=gnu11 -O2 -o buildC -Ideps/CUBE/include/ build.c
if [ $? -ne 0 ]; then
    echo "Build bootstrap failed"
    exit 1
fi

./buildC "$@"