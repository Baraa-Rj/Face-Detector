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
    
    // Thread-safe getters
    cv::Mat getLatestFrame() const;
    std::vector<cv::Rect> getLatestFaces() const;
    int getFaceCount() const;
    
    // Callback for frame updates (Qt-free interface)
    void setFrameUpdateCallback(std::function<void()> callback);

private:
    void processFrames();
    bool isValidCameraIndex(int index) const;
    
    CameraManager cameraManager;
    FaceDetector faceDetector;
    
    // Thread management (pure C++)
    std::thread processingThread;
    std::atomic<bool> processingActive;
    std::atomic<bool> shouldStop;
    
    // Frame data (protected by mutex)
    mutable std::mutex frameMutex;
    cv::Mat latestFrame;
    std::vector<cv::Rect> latestFaces;
    int faceCount;
    
    // Synchronization
    std::condition_variable frameCondition;
    static constexpr int FRAME_INTERVAL_MS = 33; // ~30 FPS
    static constexpr int MAX_CAMERA_INDEX = 10; // Reasonable limit
    
    // Callback for frame updates
    std::function<void()> frameUpdateCallback;
    mutable std::mutex callbackMutex; // Protect callback access
};
