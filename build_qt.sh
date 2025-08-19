#!/bin/bash

echo "Building Qt Face Detection program..."

# Check if OpenCV is installed
if ! pkg-config --exists opencv4; then
    echo "Error: OpenCV not found. Installing OpenCV..."
    sudo apt-get install -y libopencv-dev pkg-config
fi

# Create build directory
mkdir -p build_qt
cd build_qt

# Copy CMakeLists file
cp ../CMakeLists_qt.txt CMakeLists.txt

# Build using CMake
echo "Building with CMake..."
cmake . && make

if [ $? -eq 0 ]; then
    echo "Build successful! Run with: ./build_qt/face_detection_qt"
else
    echo "Build failed!"
    exit 1
fi
