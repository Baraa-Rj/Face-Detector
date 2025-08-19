cmake_minimum_required(VERSION 3.16)
project(FaceDetectionQt)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Core Widgets REQUIRED)
find_package(OpenCV REQUIRED)

add_executable(face_detection_qt ../main_qt.cpp)

target_link_libraries(face_detection_qt 
    Qt6::Core 
    Qt6::Widgets 
    ${OpenCV_LIBS}
)

target_include_directories(face_detection_qt PRIVATE ${OpenCV_INCLUDE_DIRS})
