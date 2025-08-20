cmake_minimum_required(VERSION 3.16)
project(FaceDetectionQt)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable Qt automation tools
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find required packages
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
find_package(OpenCV REQUIRED)

# Subdirectories contain placeholder CMakeLists.txt files for organization
# They don't affect the build process

# Create executable
add_executable(face_detection 
    src/main.cpp
    src/MainWindow.cpp
    src/FaceDetector.cpp
    src/CameraManager.cpp
    src/FrameProcessor.cpp
    Headers/MainWindow.h
    Headers/FaceDetector.h
    Headers/CameraManager.h
    Headers/FrameProcessor.h
)

# Link libraries
target_link_libraries(face_detection 
    Qt6::Core 
    Qt6::Widgets 
    ${OpenCV_LIBS}
)

# Include directories
target_include_directories(face_detection PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/Headers
    ${OpenCV_INCLUDE_DIRS}
)
