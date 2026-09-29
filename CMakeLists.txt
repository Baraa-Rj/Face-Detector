cmake_minimum_required(VERSION 3.16)
project(FaceDetection)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
find_package(OpenCV REQUIRED)

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

target_link_libraries(face_detection 
    Qt6::Core 
    Qt6::Widgets 
    ${OpenCV_LIBS}
    pthread
)

target_include_directories(face_detection PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/Headers
    ${OpenCV_INCLUDE_DIRS}
)

target_compile_options(face_detection PRIVATE -pthread)

set(FACE_CASCADE_FILE ${CMAKE_CURRENT_SOURCE_DIR}/haarcascade_frontalface_alt.xml)
configure_file(${FACE_CASCADE_FILE} ${CMAKE_CURRENT_BINARY_DIR}/haarcascade_frontalface_alt.xml COPYONLY)
target_compile_definitions(face_detection PRIVATE FACE_CASCADE_PATH="${FACE_CASCADE_FILE}")

enable_testing()

add_executable(test_classifier_path
    tests/test_classifier_path.cpp
    src/FaceDetector.cpp
)
target_link_libraries(test_classifier_path ${OpenCV_LIBS})
target_compile_definitions(test_classifier_path PRIVATE FACE_CASCADE_PATH="${FACE_CASCADE_FILE}")
target_include_directories(test_classifier_path PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Headers
    ${OpenCV_INCLUDE_DIRS}
)
add_test(NAME classifier_path_from_repo_root COMMAND test_classifier_path
         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
add_test(NAME classifier_path_from_build_dir COMMAND test_classifier_path
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
