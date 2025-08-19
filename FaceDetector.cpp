#include "FaceDetector.h"
#include <iostream>

FaceDetector::FaceDetector()
    : m_scaleFactor(1.1)
    , m_minNeighbors(3)
    , m_minSize(30, 30)
{
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
        m_scaleFactor, 
        m_minNeighbors, 
        0, 
        m_minSize
    );
    
    return faces;
}

void FaceDetector::drawFaceRectangles(cv::Mat& frame, const std::vector<cv::Rect>& faces) const {
    for (const auto& face : faces) {
        cv::rectangle(frame, face, cv::Scalar(0, 255, 0), 2);
    }
}

void FaceDetector::setScaleFactor(double scaleFactor) {
    m_scaleFactor = scaleFactor;
}

void FaceDetector::setMinNeighbors(int minNeighbors) {
    m_minNeighbors = minNeighbors;
}

void FaceDetector::setMinSize(const cv::Size& minSize) {
    m_minSize = minSize;
}

std::vector<std::string> FaceDetector::getClassifierPaths() const {
    return {
        "haarcascade_frontalface_alt.xml",
        "../haarcascade_frontalface_alt.xml",
        "../../haarcascade_frontalface_alt.xml"
    };
}
