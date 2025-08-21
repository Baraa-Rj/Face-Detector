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

private:
    bool isValidCameraIndex(int cameraIndex) const;
    
    cv::VideoCapture videoCapture;
    int cameraIndex;
    
    static constexpr int MAX_CAMERA_INDEX = 10;
};
