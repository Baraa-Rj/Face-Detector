#include "../Headers/FaceDetector.h"
#include <iostream>

FaceDetector::FaceDetector() {
}

bool FaceDetector::loadClassifier() {
    std::string path = "haarcascade_frontalface_alt.xml";
    
    // SECURITY FIX: Check if file exists before trying to load
    if (tryLoadClassifierFromPath(path)) {
        std::cout << "Loaded classifier from: " << path << std::endl;
        return true;
    }
    
    std::cerr << "Error: Could not load face cascade classifier from: " << path << std::endl;
    std::cerr << "Please ensure the classifier file exists in the project directory." << std::endl;
    return false;
}

bool FaceDetector::tryLoadClassifierFromPath(const std::string& path) {
    // SECURITY FIX: Validate path
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
    // SECURITY FIX: Validate input frame
    if (frame.empty() || frame.rows <= 0 || frame.cols <= 0) {
        return {};
    }
    
    // SECURITY FIX: Check if classifier is loaded
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
    // SECURITY FIX: Validate input parameters
    if (frame.empty() || frame.rows <= 0 || frame.cols <= 0) {
        return;
    }
    
    for (const auto& face : faces) {
        // SECURITY FIX: Validate rectangle bounds
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


