#pragma once

#include <opencv2/opencv.hpp>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>

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
    void drawFrameNumber(cv::Mat& frame, int frameNumber);
    
    CameraManager cameraManager;
    FaceDetector faceDetector;
    
    std::thread captureThread;
    std::thread processingThread;
    
    std::atomic<bool> processingActive;
    std::atomic<bool> shouldStop;
    
    std::atomic<int> frameCounter;
    
    mutable std::mutex frameBufferMutex;
    cv::Mat currentFrame;
    bool frameReady;
    bool frameProcessed;
    std::condition_variable frameReadyCondition;
    std::condition_variable frameProcessedCondition;
    
    mutable std::mutex processedFrameMutex;
    cv::Mat latestFrame;
    std::vector<cv::Rect> latestFaces;
    int faceCount;
    
    static constexpr int FRAME_INTERVAL_MS = 33;
    
    std::function<void()> frameUpdateCallback;
    mutable std::mutex callbackMutex;
};
