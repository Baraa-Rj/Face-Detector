#include "../Headers/CameraManager.h"
#include <iostream>

CameraManager::CameraManager()
    : cameraIndex(0)
{
}

bool CameraManager::isValidCameraIndex(int cameraIndex) const {
    return cameraIndex >= 0 && cameraIndex < MAX_CAMERA_INDEX;
}

bool CameraManager::openCamera(int cameraIndex) {
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
