#!/bin/bash

echo "Building Face Detection Qt Application..."

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
    echo "Executable: ./build/face_detection"
    echo ""
    echo "To run: ./build/face_detection"
else
    echo "Build failed!"
    exit 1
fi
