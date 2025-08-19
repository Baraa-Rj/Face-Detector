#!/bin/bash

# Simple build script for Face Detection program

echo "Building Face Detection program..."

# Create build directory
mkdir -p build
cd build

# Build using CMake
echo "Building with CMake..."
cmake .. && make

if [ $? -eq 0 ]; then
    echo "Build successful! Run with: ./build/face_detection"
else
    echo "Build failed!"
    exit 1
fi
