#!/bin/sh

export VERBOSE=1
export CC=clang

cmake -S . -B build
cmake --build build

