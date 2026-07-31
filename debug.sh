#!/bin/bash

mkdir -p build_debug

cmake -S . -B build_debug -DCMAKE_BUILD_TYPE=Debug -g -O0

cmake --build build_debug -j$(nproc)
