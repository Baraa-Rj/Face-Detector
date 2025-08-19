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

# Source files
set(SOURCES
    main.cpp
    MainWindow.cpp
    FaceDetector.cpp
    CameraManager.cpp
)

# Header files
set(HEADERS
    MainWindow.h
    FaceDetector.h
    CameraManager.h
)

# Create executable
add_executable(face_detection ${SOURCES} ${HEADERS})

# Link libraries
target_link_libraries(face_detection 
    Qt6::Core 
    Qt6::Widgets 
    ${OpenCV_LIBS}
)

# Include directories
target_include_directories(face_detection PRIVATE ${OpenCV_INCLUDE_DIRS})
