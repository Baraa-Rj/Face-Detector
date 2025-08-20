#include "../Headers/CameraManager.h"
#include <iostream>
#include <stdexcept>

CameraManager::CameraManager()
    : cameraIndex(0)
{
}

bool CameraManager::isValidCameraIndex(int cameraIndex) const {
    return cameraIndex >= 0 && cameraIndex < MAX_CAMERA_INDEX;
}

bool CameraManager::isValidProperty(int property) const {
    // Check if property is within valid OpenCV camera property range
    return property >= 0 && property <= 100; // OpenCV property range
}

bool CameraManager::openCamera(int cameraIndex) {
    // Input validation
    if (!isValidCameraIndex(cameraIndex)) {
        std::cerr << "Invalid camera index: " << cameraIndex << " (max: " << MAX_CAMERA_INDEX - 1 << ")" << std::endl;
        return false;
    }
    
    this->cameraIndex = cameraIndex;
    
    if (videoCapture.isOpened()) {
        videoCapture.release();
    }
    
    videoCapture.open(cameraIndex);
    
    if (!videoCapture.isOpened()) {
        std::cerr << "Could not open camera " << cameraIndex << std::endl;
        return false;
    }
    
    // Set camera properties with validation
    if (!videoCapture.set(cv::CAP_PROP_FRAME_WIDTH, 640) ||
        !videoCapture.set(cv::CAP_PROP_FRAME_HEIGHT, 480) ||
        !videoCapture.set(cv::CAP_PROP_FPS, 30)) {
        std::cerr << "Warning: Could not set all camera properties" << std::endl;
    }
    
    return true;
}

void CameraManager::closeCamera() {
    if (videoCapture.isOpened()) {
        videoCapture.release();
    }
}

bool CameraManager::isOpened() const {
    return videoCapture.isOpened();
}

cv::Mat CameraManager::captureFrame() {
    if (!videoCapture.isOpened()) {
        return cv::Mat();
    }
    
    cv::Mat frame;
    videoCapture >> frame;
    
    if (frame.empty()) {
        return cv::Mat();
    }
    
    return frame;
}

cv::Size CameraManager::getFrameSize() const {
    if (!videoCapture.isOpened()) {
        return cv::Size(0, 0);
    }
    
    return cv::Size(
        static_cast<int>(videoCapture.get(cv::CAP_PROP_FRAME_WIDTH)),
        static_cast<int>(videoCapture.get(cv::CAP_PROP_FRAME_HEIGHT))
    );
}

bool CameraManager::setCameraProperty(int property, double value) {
    if (!isValidProperty(property)) {
        std::cerr << "Invalid camera property: " << property << std::endl;
        return false;
    }
    
    if (!videoCapture.isOpened()) {
        std::cerr << "Camera is not opened" << std::endl;
        return false;
    }
    
    return videoCapture.set(property, value);
}

double CameraManager::getCameraProperty(int property) const {
    if (!isValidProperty(property)) {
        std::cerr << "Invalid camera property: " << property << std::endl;
        return -1.0;
    }
    
    if (!videoCapture.isOpened()) {
        std::cerr << "Camera is not opened" << std::endl;
        return -1.0;
    }
    
    return videoCapture.get(property);
}
