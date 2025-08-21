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
    void frameCaptureThread();
    void frameProcessingThread();
    bool isValidCameraIndex(int index) const;
    void drawFrameNumber(cv::Mat& frame, int frameNumber);
    
    CameraManager cameraManager;
    FaceDetector faceDetector;
    
    // Threads
    std::thread captureThread;
    std::thread processingThread;
    
    // Control flags
    std::atomic<bool> processingActive;
    std::atomic<bool> shouldStop;
    
    // Frame counter
    std::atomic<int> frameCounter;
    
    // Single frame buffer for sequential processing
    mutable std::mutex frameBufferMutex;
    cv::Mat currentFrame;
    bool frameReady;
    bool frameProcessed;
    std::condition_variable frameReadyCondition;
    std::condition_variable frameProcessedCondition;
    
    // Final processed results
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
