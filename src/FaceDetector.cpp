#include "../Headers/FaceDetector.h"
#include <iostream>

FaceDetector::FaceDetector() {
}

#ifndef FACE_CASCADE_PATH
#define FACE_CASCADE_PATH "haarcascade_frontalface_alt.xml"
#endif

bool FaceDetector::loadClassifier() {
    const std::vector<std::string> paths = {
        FACE_CASCADE_PATH,
        "haarcascade_frontalface_alt.xml"
    };
    
    for (const auto& path : paths) {
        if (tryLoadClassifierFromPath(path)) {
            std::cout << "Loaded classifier from: " << path << std::endl;
            return true;
        }
    }
    
    std::cerr << "Error: Could not load face cascade classifier from: " << paths.front() << std::endl;
    std::cerr << "Please ensure the classifier file exists in the project directory." << std::endl;
    return false;
}

bool FaceDetector::tryLoadClassifierFromPath(const std::string& path) {
    if (path.empty()) {
        std::cerr << "Error: Empty classifier path" << std::endl;
        return false;
    }
    
    try {
        return faceCascadeClassifier.load(path);
    } catch (const cv::Exception& e) {
        std::cerr << "Exception loading classifier from " << path << ": " << e.what() << std::endl;
        return false;
    }
}

std::vector<cv::Rect> FaceDetector::detectFaces(const cv::Mat& frame) {
    if (frame.empty() || frame.rows <= 0 || frame.cols <= 0) {
        return {};
    }
    
    if (faceCascadeClassifier.empty()) {
        std::cerr << "Warning: Face classifier not loaded, skipping detection" << std::endl;
        return {};
    }
    
    cv::Mat gray;
    try {
        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
    } catch (const cv::Exception& e) {
        std::cerr << "Error converting frame to grayscale: " << e.what() << std::endl;
        return {};
    }
    
    std::vector<cv::Rect> faces;
    try {
        faceCascadeClassifier.detectMultiScale(
            gray, 
            faces, 
            1.1,
            3,
            0,
            cv::Size(30, 30)
        );
    } catch (const cv::Exception& e) {
        std::cerr << "Error during face detection: " << e.what() << std::endl;
        return {};
    }
    
    return faces;
}

void FaceDetector::drawFaceRectangles(cv::Mat& frame, const std::vector<cv::Rect>& faces) const {
    if (frame.empty() || frame.rows <= 0 || frame.cols <= 0) {
        return;
    }
    
    for (const auto& face : faces) {
        if (face.x >= 0 && face.y >= 0 && 
            face.x + face.width <= frame.cols && 
            face.y + face.height <= frame.rows &&
            face.width > 0 && face.height > 0) {
            
            try {
                cv::rectangle(frame, face, cv::Scalar(0, 255, 0), 2);
            } catch (const cv::Exception& e) {
                std::cerr << "Error drawing face rectangle: " << e.what() << std::endl;
            }
        }
    }
}


