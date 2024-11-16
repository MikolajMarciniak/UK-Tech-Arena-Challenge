#!/bin/bash

# Remove the 'build' directory if it exists
if [ -d "build" ]; then
    rm -rf build
fi

# Create the 'build' directory
mkdir build

# Change into the 'build' directory
cd build

# Run cmake and make
cmake ..
make

# Check if 'main' exists and execute it
if [ -f "main" ]; then
    ./main
else
    echo "Error: 'main' file not found. Compilation may have failed."
    exit 1
fi
