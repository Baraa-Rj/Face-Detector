# Real-Time Face Detection with OpenCV

A simple C++ program that performs real-time face detection using your webcam and OpenCV's Haar cascade classifier.

## Features

- Real-time face detection from webcam feed
- Uses Haar cascade classifier for accurate detection
- Draws green rectangles around detected faces
- Simple keyboard control (press 'q' to quit)
- Basic error handling for camera and classifier loading

## Requirements

- OpenCV 4.x
- C++17 compatible compiler (GCC 7+, Clang 5+)
- Webcam
- Linux/Ubuntu (tested on Ubuntu 20.04+)

## Installation

### Install OpenCV and dependencies

```bash
# Update package list
sudo apt-get update

# Install OpenCV development libraries
sudo apt-get install -y libopencv-dev pkg-config

# Verify installation
pkg-config --modversion opencv4
```

## Compilation

### Console Version

#### Using CMake (Recommended)
```bash
# Create build directory
mkdir build
cd build

# Configure and build
cmake ..
make

# Run the program
./face_detection
```

#### Using build script
```bash
./build.sh
```

#### Manual compilation
```bash
g++ -std=c++17 -Wall -Wextra \
    $(pkg-config --cflags opencv4) \
    -o face_detection main.cpp \
    $(pkg-config --libs opencv4)
```

### Qt GUI Version

#### Using build script (Recommended)
```bash
./build_qt.sh
```

#### Manual CMake build
```bash
mkdir build_qt
cd build_qt
cmake -f ../CMakeLists_qt.txt ..
make
```

## Usage

### Console Version
1. Make sure your webcam is connected and accessible
2. Run the compiled executable: `./face_detection`
3. The program will open your webcam and start detecting faces
4. Green rectangles will be drawn around detected faces
5. Press 'q' to quit the program

### Qt GUI Version
1. Make sure your webcam is connected and accessible
2. Run the compiled executable: `./face_detection_qt`
3. A GUI window will open with camera controls
4. Click "Start Camera" to begin face detection
5. Adjust detection sensitivity using the sliders
6. Real-time face count is displayed in the status bar
7. Close the window to quit the program

## Troubleshooting

### Camera not opening
- Ensure your webcam is not being used by another application
- Check camera permissions
- Try different camera indices (change `cv::VideoCapture cap(0)` to `cap(1)`, etc.)

### Classifier not loading
- The program looks for `haarcascade_frontalface_alt.xml` in the OpenCV samples directory
- If you get an error, download the classifier file manually:
  ```bash
  wget https://raw.githubusercontent.com/opencv/opencv/master/data/haarcascades/haarcascade_frontalface_alt.xml
  ```

### Compilation errors
- Ensure OpenCV is properly installed: `pkg-config --modversion opencv4`
- Check that you have a C++17 compatible compiler
- Verify all dependencies are installed

## Project Structure

The project contains both console and GUI versions:

### Console Version
- `main.cpp` - The complete face detection program
- `CMakeLists.txt` - CMake build configuration
- `build.sh` - Build script for console version

### Qt GUI Version
- `main_qt.cpp` - Qt-based GUI face detection program
- `CMakeLists_qt.txt` - CMake configuration for Qt version
- `build_qt.sh` - Build script for Qt version

### Common Files
- `haarcascade_frontalface_alt.xml` - Face detection classifier
- `README.md` - This documentation

## Code Structure

The program consists of a single `main.cpp` file with the following key components:

1. **Camera initialization**: Opens the default webcam
2. **Classifier loading**: Loads the Haar cascade face detector
3. **Main loop**: Captures frames, detects faces, and displays results
4. **Face detection**: Uses `detectMultiScale()` with optimized parameters
5. **Visualization**: Draws rectangles around detected faces
6. **Cleanup**: Properly releases resources

## Key OpenCV Functions Used

- `cv::VideoCapture`: Camera interface
- `cv::CascadeClassifier`: Face detection classifier
- `cv::cvtColor`: Color space conversion
- `cv::detectMultiScale`: Multi-scale object detection
- `cv::rectangle`: Drawing rectangles
- `cv::imshow`: Display images

## License

This project is open source and available under the MIT License.
