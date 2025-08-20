#include "../Headers/FaceDetector.h"
#include <iostream>

FaceDetector::FaceDetector() {
}

bool FaceDetector::loadClassifier() {
    std::string path = "../haarcascade_frontalface_alt.xml";
    
    if (tryLoadClassifierFromPath(path)) {
        std::cout << "Loaded classifier from: " << path << std::endl;
        return true;
    }
    
    std::cerr << "Error: Could not load face cascade classifier!" << std::endl;
    return false;
}

bool FaceDetector::tryLoadClassifierFromPath(const std::string& path) {
    return faceCascadeClassifier.load(path);
}

std::vector<cv::Rect> FaceDetector::detectFaces(const cv::Mat& frame) {
    if (frame.empty()) {
        return {};
    }
    
    cv::Mat gray;
    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
    
    std::vector<cv::Rect> faces;
    faceCascadeClassifier.detectMultiScale(
        gray, 
        faces, 
        1.1,
        3,
        0,
        cv::Size(30, 30)
    );
    
    return faces;
}

void FaceDetector::drawFaceRectangles(cv::Mat& frame, const std::vector<cv::Rect>& faces) const {
    for (const auto& face : faces) {
        cv::rectangle(frame, face, cv::Scalar(0, 255, 0), 2);
    }
}


