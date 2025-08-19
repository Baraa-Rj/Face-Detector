# Real-Time Face Detection with Qt + OpenCV

A modern C++ Qt application that performs real-time face detection using your webcam and OpenCV's Haar cascade classifier with a professional GUI interface.

## Features

- **Modern Qt GUI**: Professional interface with controls and real-time display
- **Real-time Detection**: Live webcam face detection using OpenCV
- **Adjustable Settings**: Sliders for detection sensitivity tuning
- **Visual Feedback**: Green rectangles around detected faces
- **Status Display**: Real-time face count and system status
- **Modular Design**: Clean separation of concerns with low coupling

## Requirements

- **Qt6**: Core and Widgets modules
- **OpenCV 4.x**: Computer vision library
- **C++17**: Compatible compiler (GCC 7+, Clang 5+)
- **Webcam**: Default camera device
- **Linux/Ubuntu**: Tested on Ubuntu 20.04+

## Installation

### Install Qt6, OpenCV and dependencies

```bash
# Update package list
sudo apt-get update

# Install Qt6 and OpenCV development libraries
sudo apt-get install -y qt6-base-dev qt6-tools-dev libopencv-dev pkg-config

# Verify installations
pkg-config --modversion opencv4
qmake6 --version
```

## Compilation

### Unified Build System

The project now uses a single build system for both versions:

#### Using build script (Recommended)
```bash
./build.sh
```

This will build both the console and Qt GUI versions (if Qt6 is available).

#### Manual CMake build
```bash
# Create build directory
mkdir build
cd build

# Configure and build
cmake ..
make

# Available executables:
# - face_detection (console version)
# - face_detection_qt (Qt GUI version, if Qt6 found)
```

#### Manual compilation (Console only)
```bash
g++ -std=c++17 -Wall -Wextra \
    $(pkg-config --cflags opencv4) \
    -o face_detection main.cpp \
    $(pkg-config --libs opencv4)
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

The project uses a modular design with low coupling between components, organized in a clean directory structure:

### Directory Organization
```
faceDetection/
├── Headers/                    # Header files (.h)
│   ├── MainWindow.h           # Main GUI window interface
│   ├── FaceDetector.h         # Face detection interface
│   └── CameraManager.h        # Camera operations interface
├── src/                        # Source files (.cpp)
│   ├── main.cpp               # Application entry point
│   ├── MainWindow.cpp         # Main GUI window implementation
│   ├── FaceDetector.cpp       # Face detection implementation
│   └── CameraManager.cpp      # Camera operations implementation
├── CMakeLists.txt              # Qt + OpenCV build configuration
├── build.sh                    # Build script
├── haarcascade_frontalface_alt.xml  # Face detection classifier
└── README.md                   # This documentation
```

### Core Classes
- **MainWindow**: Main GUI window and user interface
- **FaceDetector**: Face detection logic and OpenCV integration
- **CameraManager**: Camera operations and video capture

### Architecture Benefits
- **Low Coupling**: Each class has a single responsibility
- **High Cohesion**: Related functionality grouped together
- **Easy Testing**: Components can be tested independently
- **Maintainable**: Changes to one component don't affect others

## Class Responsibilities

### FaceDetector
- Loads and manages Haar cascade classifier
- Processes frames for face detection
- Draws detection rectangles
- Configurable detection parameters

### CameraManager
- Manages camera lifecycle (open/close)
- Captures video frames
- Handles camera errors and properties
- Emits Qt signals for camera events

### MainWindow
- Creates and manages Qt GUI interface
- Handles user interactions (buttons, sliders)
- Displays video feed and detection results
- Coordinates between camera and detector components

## Key Technologies Used

- **Qt6**: Modern GUI framework with signals/slots
- **OpenCV**: Computer vision and camera operations
- **C++17**: Modern C++ features and RAII principles

## License

This project is open source and available under the MIT License.
