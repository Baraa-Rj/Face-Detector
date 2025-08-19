#include "../Headers/FaceDetector.h"
#include <iostream>

FaceDetector::FaceDetector() {
}

bool FaceDetector::loadClassifier() {
    auto paths = getClassifierPaths();
    
    for (const auto& path : paths) {
        if (m_faceCascade.load(path)) {
            std::cout << "Loaded classifier from: " << path << std::endl;
            return true;
        }
    }
    
    std::cerr << "Error: Could not load face cascade classifier!" << std::endl;
    return false;
}

std::vector<cv::Rect> FaceDetector::detectFaces(const cv::Mat& frame) {
    if (frame.empty()) {
        return {};
    }
    
    cv::Mat gray;
    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
    
    std::vector<cv::Rect> faces;
    m_faceCascade.detectMultiScale(
        gray, 
        faces, 
        1.1,        // Scale factor - optimal for most cases
        3,           // Min neighbors - good balance of accuracy/sensitivity
        0,           // Flags
        cv::Size(30, 30)  // Min size - reasonable minimum face size
    );
    
    return faces;
}

void FaceDetector::drawFaceRectangles(cv::Mat& frame, const std::vector<cv::Rect>& faces) const {
    for (const auto& face : faces) {
        cv::rectangle(frame, face, cv::Scalar(0, 255, 0), 2);
    }
}

std::vector<std::string> FaceDetector::getClassifierPaths() const {
    return {
        "haarcascade_frontalface_alt.xml",
        "../haarcascade_frontalface_alt.xml",
        "../../haarcascade_frontalface_alt.xml"
    };
}
