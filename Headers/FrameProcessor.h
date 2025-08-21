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
#include <queue>

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
    void frameCaptureThread();
    void frameProcessingThread();
    bool isValidCameraIndex(int index) const;
    
    CameraManager cameraManager;
    FaceDetector faceDetector;
    
    // Threads
    std::thread captureThread;
    std::thread processingThread;
    
    // Control flags
    std::atomic<bool> processingActive;
    std::atomic<bool> shouldStop;
    
    // Thread-safe queues
    mutable std::mutex rawFrameMutex;
    std::queue<cv::Mat> rawFrameQueue;
    std::condition_variable rawFrameCondition;
    static constexpr size_t MAX_RAW_FRAMES = 5; // Limit queue size
    
    mutable std::mutex processedFrameMutex;
    cv::Mat latestFrame;
    std::vector<cv::Rect> latestFaces;
    int faceCount;
    
    // Timing control
    static constexpr int FRAME_INTERVAL_MS = 33;
    static constexpr int MAX_CAMERA_INDEX = 10;
    
    // Callback mechanism
    std::function<void()> frameUpdateCallback;
    mutable std::mutex callbackMutex;
};
