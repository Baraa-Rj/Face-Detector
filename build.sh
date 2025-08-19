#!/bin/bash

echo "Building Face Detection program..."

# Check if OpenCV is installed
if ! pkg-config --exists opencv4; then
    echo "Error: OpenCV not found. Installing OpenCV..."
    sudo apt-get install -y libopencv-dev pkg-config
fi

# Create build directory
mkdir -p build
cd build

# Build using CMake
echo "Building with CMake..."
cmake .. && make

if [ $? -eq 0 ]; then
    echo ""
    echo "Build successful!"
    echo ""
    echo "Available executables:"
    if [ -f "face_detection" ]; then
        echo "  Console version: ./build/face_detection"
    fi
    if [ -f "face_detection_qt" ]; then
        echo "  Qt GUI version: ./build/face_detection_qt"
    fi
    echo ""
    echo "To run console version: ./build/face_detection"
    echo "To run Qt GUI version: ./build/face_detection_qt"
else
    echo "Build failed!"
    exit 1
fi
