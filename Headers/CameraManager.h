#pragma once

#include <opencv2/opencv.hpp>

class CameraManager {
public:
    CameraManager();
    ~CameraManager() = default;

    bool openCamera(int cameraIndex = 0);
    void closeCamera();
    bool isOpened() const;
    
    cv::Mat captureFrame();
    cv::Size getFrameSize() const;
    
    bool setCameraProperty(int property, double value);
    double getCameraProperty(int property) const;

private:
    cv::VideoCapture m_camera;
    int m_cameraIndex;
};
