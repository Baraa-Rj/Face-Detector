#!/bin/bash

# Test script for Face Detection program

echo "=== Face Detection Program Test ==="
echo ""

# Test 1: Check if OpenCV is installed
echo "Test 1: Checking OpenCV installation..."
if pkg-config --exists opencv4; then
    echo "✓ OpenCV $(pkg-config --modversion opencv4) is installed"
else
    echo "✗ OpenCV not found"
    exit 1
fi

# Test 2: Check if classifier file exists
echo ""
echo "Test 2: Checking classifier file..."
if [ -f "haarcascade_frontalface_alt.xml" ]; then
    echo "✓ Haar cascade classifier file found"
else
    echo "✗ Classifier file not found"
    exit 1
fi

# Test 3: Build the program
echo ""
echo "Test 3: Building the program..."
if ./build.sh; then
    echo "✓ Program built successfully"
else
    echo "✗ Build failed"
    exit 1
fi

# Test 4: Check if executable exists
echo ""
echo "Test 4: Checking executable..."
if [ -f "build/face_detection" ]; then
    echo "✓ Executable created successfully"
else
    echo "✗ Executable not found"
    exit 1
fi

# Test 5: Test program startup (will fail on camera access, but should load classifier)
echo ""
echo "Test 5: Testing program startup..."
timeout 3s ./build/face_detection > /tmp/face_detection_test.log 2>&1
if grep -q "Face detection started" /tmp/face_detection_test.log; then
    echo "✓ Program starts successfully and loads classifier"
else
    echo "✗ Program failed to start or load classifier"
    echo "Last few lines of output:"
    tail -5 /tmp/face_detection_test.log
    exit 1
fi

echo ""
echo "=== All tests passed! ==="
echo "The program is ready to use with a webcam."
echo ""
echo "To run: ./build/face_detection"
echo "To quit: Press 'q' while the program is running"
