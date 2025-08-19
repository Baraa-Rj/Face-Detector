#!/bin/bash

# Build script for Face Detection program

echo "Building Face Detection program..."

# Check if OpenCV is installed
if ! pkg-config --exists opencv4; then
    echo "Error: OpenCV4 not found. Please install it first:"
    echo "sudo apt-get install libopencv-dev pkg-config"
    exit 1
fi

echo "OpenCV version: $(pkg-config --modversion opencv4)"

# Create build directory
mkdir -p build
cd build

# Build using CMake
echo "Configuring with CMake..."
cmake .. || { echo "CMake configuration failed"; exit 1; }

echo "Building..."
make || { echo "Build failed"; exit 1; }

echo "Build successful! Executable created: build/face_detection"
echo ""
echo "To run the program:"
echo "cd build && ./face_detection"
echo ""
echo "Or from the project root:"
echo "./build/face_detection"
