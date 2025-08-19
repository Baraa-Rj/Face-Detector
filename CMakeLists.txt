cmake_minimum_required(VERSION 3.10)
project(FaceDetection)

# Find OpenCV package
find_package(OpenCV REQUIRED)

# Include OpenCV headers
include_directories(${OpenCV_INCLUDE_DIRS})

# Create executable
add_executable(face_detection main.cpp)

# Link OpenCV libraries
target_link_libraries(face_detection ${OpenCV_LIBS})

# Set C++ standard
set_target_properties(face_detection PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)
