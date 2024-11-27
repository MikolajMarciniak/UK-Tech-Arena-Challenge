#!/bin/bash

clear
if [ -d "build" ]; then
    rm -rf build
fi

mkdir build

cd build

cmake ..
make

if [ -f "main" ]; then
    ./main
else
    echo "Error: 'main' file not found. Compilation may have failed."
    exit 1
fi