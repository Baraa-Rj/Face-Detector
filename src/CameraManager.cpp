#include "../Headers/CameraManager.h"
#include <iostream>

CameraManager::CameraManager(QObject* parent)
    : QObject(parent)
    , m_cameraIndex(0)
{
}

bool CameraManager::openCamera(int cameraIndex) {
    m_cameraIndex = cameraIndex;
    
    if (m_camera.isOpened()) {
        m_camera.release();
    }
    
    m_camera.open(m_cameraIndex);
    
    if (!m_camera.isOpened()) {
        emit cameraError(QString("Could not open camera %1").arg(cameraIndex));
        return false;
    }
    
    m_camera.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    m_camera.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
    m_camera.set(cv::CAP_PROP_FPS, 30);
    m_camera.set(cv::CAP_PROP_BUFFERSIZE, 1);
    
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
    cv::Mat frame;
    
    if (!m_camera.isOpened()) {
        return frame;
    }
    
    m_camera >> frame;
    
    if (frame.empty()) {
        return frame;
    }
    
    if (frame.cols <= 0 || frame.rows <= 0) {
        return cv::Mat();
    }
    
    emit frameReady(frame);
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
    
    return m_camera.set(property, value);
}

double CameraManager::getCameraProperty(int property) const {
    if (!m_camera.isOpened()) {
        return -1.0;
    }
    
    return m_camera.get(property);
}
