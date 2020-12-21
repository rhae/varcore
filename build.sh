#!/bin/sh

# export VERBOSE=1
export CC=clang

cmake -S . -B build -DCUNIT_DISABLE_TESTS=TRUE -DCUNIT_DISABLE_EXAMPLES=TRUE
cmake --build build 

