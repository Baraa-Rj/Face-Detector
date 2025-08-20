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
    bool isValidCameraIndex(int cameraIndex) const;
    bool isValidProperty(int property) const;
    
    cv::VideoCapture videoCapture;
    int cameraIndex;
    
    static constexpr int MAX_CAMERA_INDEX = 10;
    static constexpr int MIN_FRAME_WIDTH = 320;
    static constexpr int MAX_FRAME_WIDTH = 1920;
    static constexpr int MIN_FRAME_HEIGHT = 240;
    static constexpr int MAX_FRAME_HEIGHT = 1080;
    static constexpr int MIN_FPS = 1;
    static constexpr int MAX_FPS = 60;
};
