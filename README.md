# Real-Time Face Detection with Qt + OpenCV

A high-performance C++ Qt application that performs real-time face detection using your webcam and OpenCV's Haar cascade classifier with a professional GUI interface and multi-threaded architecture.

## Features

- **Modern Qt GUI**: Professional interface with real-time camera feed display
- **Multi-threaded Architecture**: Separate threads for frame capture and processing
- **Parallel Processing**: Multi-threaded frame processing using a thread pool for improved performance
- **Frame Numbering**: Visual frame counter to verify processing order
- **Real-time Detection**: Live webcam face detection using OpenCV Haar cascades
- **Visual Feedback**: Green rectangles around detected faces with frame numbers
- **Status Display**: Real-time face count and processing status
- **Optimized Performance**: Memory-efficient frame handling with move semantics
- **Thread Safety**: Comprehensive mutex protection and safe cross-thread communication
- **Loose Coupling**: Qt GUI separated from pure C++ core logic

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

### Build System

The project uses CMake with Qt6 and OpenCV integration:

#### Using build script (Recommended)
```bash
./build.sh
```

This will create a `build/` directory and compile the Qt GUI application.

#### Manual CMake build
```bash
# Create build directory
mkdir build
cd build

# Configure and build
cmake ..
make

# Executable created: face_detection
```

## Usage

### Qt GUI Application
1. Make sure your webcam is connected and accessible
2. Run the compiled executable: `./build/face_detection`
3. A GUI window will open with camera controls
4. Click "Start Camera" to begin face detection
5. Watch the frame counter in the upper-left corner of the video feed
6. Green rectangles will be drawn around detected faces
7. Real-time face count is displayed in the status bar
8. The application uses parallel processing - multiple frames can be processed simultaneously using a thread pool
9. Click "Stop Camera" or close the window to quit

## Troubleshooting

### Camera not opening
- Ensure your webcam is not being used by another application
- Check camera permissions
- Try different camera indices (change `cv::VideoCapture cap(0)` to `cap(1)`, etc.)

### Classifier not loading
- The program looks for `haarcascade_frontalface_alt.xml` in the project root directory
- This file is already included in the repository
- If you get an error, ensure the file exists in the same directory as the executable

### Compilation errors
- Ensure OpenCV is properly installed: `pkg-config --modversion opencv4`
- Check that you have a C++17 compatible compiler
- Verify all dependencies are installed

## Project Structure

The project uses a multi-threaded, modular design with loose coupling between components:

### Directory Organization
```
faceDetection/
├── Headers/                    # Header files (.h)
│   ├── MainWindow.h           # Main GUI window interface
│   ├── FaceDetector.h         # Face detection interface
│   ├── CameraManager.h        # Camera operations interface
│   └── FrameProcessor.h       # Multi-threaded frame processing
├── src/                        # Source files (.cpp)
│   ├── main.cpp               # Application entry point
│   ├── MainWindow.cpp         # Main GUI window implementation
│   ├── FaceDetector.cpp       # Face detection implementation
│   ├── CameraManager.cpp      # Camera operations implementation
│   └── FrameProcessor.cpp     # Multi-threaded frame processing
├── CMakeLists.txt              # Qt + OpenCV build configuration
├── build.sh                    # Build script
├── haarcascade_frontalface_alt.xml  # Face detection classifier
└── README.md                   # This documentation
```

### Core Classes
- **MainWindow**: Qt GUI interface and user interactions
- **FrameProcessor**: Multi-threaded frame capture and processing coordinator
- **FaceDetector**: Face detection logic and OpenCV integration
- **CameraManager**: Camera operations and video capture

### Architecture Benefits
- **Multi-threaded**: Separate threads for capture and processing
- **Parallel Processing**: Thread pool allows multiple frames to be processed simultaneously
- **Loose Coupling**: Qt GUI separated from C++ core logic
- **Thread Safety**: Comprehensive mutex protection and thread-safe queues
- **Memory Efficient**: Optimized frame handling with move semantics and overflow protection
- **Maintainable**: Clean separation of concerns
- **High Performance**: Utilizes multiple CPU cores for improved throughput

## Class Responsibilities

### FrameProcessor
- Manages capture thread and a pool of processing worker threads
- Implements parallel frame processing with thread pool architecture
- Handles thread-safe frame queuing with overflow protection
- Provides thread-safe callbacks to the GUI layer
- Coordinates between camera and face detection components
- Utilizes multiple CPU cores for improved performance

### FaceDetector
- Loads and manages Haar cascade classifier
- Processes frames for face detection using OpenCV
- Draws detection rectangles on processed frames
- Optimized for real-time performance

### CameraManager
- Manages camera lifecycle (open/close/capture)
- Handles camera initialization and properties
- Provides reliable frame capture with error handling
- Validates camera access and availability

### MainWindow
- Creates and manages Qt GUI interface
- Handles user interactions (start/stop camera)
- Displays real-time video feed with frame numbers
- Shows detection results and status information
- Communicates with FrameProcessor via safe callbacks

## Key Technologies Used

- **Qt6**: Modern GUI framework with signals/slots and cross-thread communication
- **OpenCV**: Computer vision library for face detection and camera operations
- **C++17**: Modern C++ with threading, mutexes, condition variables, and move semantics
- **Multi-threading**: Thread pool pattern with parallel frame processing and thread-safe synchronization

## License

This project is open source and available under the MIT License.
