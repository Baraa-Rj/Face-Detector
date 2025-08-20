#pragma once

#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <limits>

#include "FaceDetector.h"
#include "CameraManager.h"

class FrameProcessor {
public:
    FrameProcessor();
    ~FrameProcessor();

    void startProcessing();
    void stopProcessing();
    bool isProcessing() const;
    
    cv::Mat getLatestFrame() const;
    std::vector<cv::Rect> getLatestFaces() const;
    int getFaceCount() const;
    
    void setFrameUpdateCallback(std::function<void()> callback);

private:
    void processFrames();
    bool isValidCameraIndex(int index) const;
    
    CameraManager cameraManager;
    FaceDetector faceDetector;
    
    std::thread processingThread;
    std::atomic<bool> processingActive;
    std::atomic<bool> shouldStop;
    
    mutable std::mutex frameMutex;
    cv::Mat latestFrame;
    std::vector<cv::Rect> latestFaces;
    int faceCount;
    
    std::condition_variable frameCondition;
    static constexpr int FRAME_INTERVAL_MS = 33;
    static constexpr int MAX_CAMERA_INDEX = 10;
    
    std::function<void()> frameUpdateCallback;
    mutable std::mutex callbackMutex;
};
