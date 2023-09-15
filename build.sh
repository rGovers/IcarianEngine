#!/bin/bash

gcc -o buildC -Ideps/CUBE/include/ build.c
if [ $? -ne 0 ]; then
    echo "Build bootstrap failed"
    exit 1
fi

./buildC "$@"