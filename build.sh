#!/bin/bash

gcc -o buildC -Ideps/CUBE/include/ -IFlareBase/ build.c
./buildC