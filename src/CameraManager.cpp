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
    // SECURITY FIX: Validate camera index
    if (!isValidCameraIndex(cameraIndex)) {
        std::cerr << "Invalid camera index: " << cameraIndex << " (max: " << MAX_CAMERA_INDEX - 1 << ")" << std::endl;
        return false;
    }
    
    this->cameraIndex = cameraIndex;
    
    // SECURITY FIX: Properly close existing camera before opening new one
    if (videoCapture.isOpened()) {
        videoCapture.release();
    }
    
    try {
        videoCapture.open(cameraIndex);
    } catch (const cv::Exception& e) {
        std::cerr << "Exception opening camera " << cameraIndex << ": " << e.what() << std::endl;
        return false;
    }
    
    if (!videoCapture.isOpened()) {
        std::cerr << "Could not open camera " << cameraIndex << std::endl;
        return false;
    }
    
    // SECURITY FIX: Set camera properties with error checking
    bool propertiesSet = true;
    if (!videoCapture.set(cv::CAP_PROP_FRAME_WIDTH, 640)) {
        std::cerr << "Warning: Could not set frame width" << std::endl;
        propertiesSet = false;
    }
    if (!videoCapture.set(cv::CAP_PROP_FRAME_HEIGHT, 480)) {
        std::cerr << "Warning: Could not set frame height" << std::endl;
        propertiesSet = false;
    }
    if (!videoCapture.set(cv::CAP_PROP_FPS, 30)) {
        std::cerr << "Warning: Could not set frame rate" << std::endl;
        propertiesSet = false;
    }
    
    if (!propertiesSet) {
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
    // SECURITY FIX: Check if camera is opened
    if (!videoCapture.isOpened()) {
        return cv::Mat();
    }
    
    cv::Mat frame;
    try {
        videoCapture >> frame;
    } catch (const cv::Exception& e) {
        std::cerr << "Exception capturing frame: " << e.what() << std::endl;
        return cv::Mat();
    }
    
    // SECURITY FIX: Validate captured frame
    if (frame.empty() || frame.rows <= 0 || frame.cols <= 0) {
        return cv::Mat();
    }
    
    return frame;
}
