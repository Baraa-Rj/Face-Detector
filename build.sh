#!/bin/bash

echo "Building Face Detection Qt Application..."

if [ ! -d "build" ]; then
    mkdir build
fi

cd build

echo "Building with CMake..."
cmake ..

if [ $? -eq 0 ]; then
    echo "Building with Make..."
    make -j$(nproc)
    
    if [ $? -eq 0 ]; then
        echo ""
        echo "Build successful!"
        echo "Executable: ./build/face_detection"
        echo ""
        echo "To run: ./build/face_detection"
    else
        echo "Build failed!"
        exit 1
    fi
else
    echo "CMake configuration failed!"
    exit 1
fi
