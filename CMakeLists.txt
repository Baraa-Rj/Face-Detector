cmake_minimum_required(VERSION 3.16)
project(FaceDetection)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find OpenCV package
find_package(OpenCV REQUIRED)

# Console version
add_executable(face_detection main.cpp)
target_link_libraries(face_detection ${OpenCV_LIBS})
target_include_directories(face_detection PRIVATE ${OpenCV_INCLUDE_DIRS})

# Qt version (optional)
option(BUILD_QT_GUI "Build Qt GUI version" ON)
if(BUILD_QT_GUI)
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)
    set(CMAKE_AUTOUIC ON)
    
    find_package(Qt6 COMPONENTS Core Widgets QUIET)
    if(Qt6_FOUND)
        add_executable(face_detection_qt main_qt.cpp)
        target_link_libraries(face_detection_qt 
            Qt6::Core 
            Qt6::Widgets 
            ${OpenCV_LIBS}
        )
        target_include_directories(face_detection_qt PRIVATE ${OpenCV_INCLUDE_DIRS})
        message(STATUS "Qt GUI version will be built")
    else()
        message(WARNING "Qt6 not found - Qt GUI version will not be built")
    endif()
endif()
