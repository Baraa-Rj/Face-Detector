#include "../Headers/CameraManager.h"
#include <iostream>
#include <stdexcept>

CameraManager::CameraManager()
    : m_cameraIndex(0)
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
    
    m_cameraIndex = cameraIndex;
    
    if (m_camera.isOpened()) {
        m_camera.release();
    }
    
    m_camera.open(m_cameraIndex);
    
    if (!m_camera.isOpened()) {
        std::cerr << "Could not open camera " << cameraIndex << std::endl;
        return false;
    }
    
    // Set camera properties with validation
    if (!m_camera.set(cv::CAP_PROP_FRAME_WIDTH, 640) ||
        !m_camera.set(cv::CAP_PROP_FRAME_HEIGHT, 480) ||
        !m_camera.set(cv::CAP_PROP_FPS, 30)) {
        std::cerr << "Warning: Could not set all camera properties" << std::endl;
    }
    
    return true;
}

void CameraManager::closeCamera() {
    if (m_camera.isOpened()) {
        m_camera.release();
    }
}

bool CameraManager::isOpened() const {
    return m_camera.isOpened();
}

cv::Mat CameraManager::captureFrame() {
    if (!m_camera.isOpened()) {
        return cv::Mat();
    }
    
    cv::Mat frame;
    m_camera >> frame;
    
    // Simplified validation - frame.empty() already checks dimensions
    if (frame.empty()) {
        return cv::Mat();
    }
    
    return frame;
}

cv::Size CameraManager::getFrameSize() const {
    if (!m_camera.isOpened()) {
        return cv::Size(0, 0);
    }
    
    return cv::Size(
        static_cast<int>(m_camera.get(cv::CAP_PROP_FRAME_WIDTH)),
        static_cast<int>(m_camera.get(cv::CAP_PROP_FRAME_HEIGHT))
    );
}

bool CameraManager::setCameraProperty(int property, double value) {
    if (!m_camera.isOpened()) {
        return false;
    }
    
    if (!isValidProperty(property)) {
        std::cerr << "Invalid camera property: " << property << std::endl;
        return false;
    }
    
    return m_camera.set(property, value);
}

double CameraManager::getCameraProperty(int property) const {
    if (!m_camera.isOpened()) {
        return -1.0;
    }
    
    if (!isValidProperty(property)) {
        std::cerr << "Invalid camera property: " << property << std::endl;
        return -1.0;
    }
    
    return m_camera.get(property);
}
