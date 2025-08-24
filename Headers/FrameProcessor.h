#pragma once

#include <opencv2/opencv.hpp>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <queue>
#include <vector>
#include <future>

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
    void frameProcessingWorker();
    void drawFrameNumber(cv::Mat& frame, int frameNumber);
    
    CameraManager cameraManager;
    FaceDetector faceDetector;
    
    std::thread captureThread;
    std::vector<std::thread> processingWorkers;
    
    std::atomic<bool> processingActive;
    std::atomic<bool> shouldStop;
    
    std::atomic<int> frameCounter;
    
    mutable std::mutex frameQueueMutex;
    std::queue<cv::Mat> frameQueue;
    std::condition_variable frameQueueCondition;
    static constexpr size_t MAX_QUEUE_SIZE = 10;
    
    mutable std::mutex resultsMutex;
    cv::Mat latestFrame;
    std::vector<cv::Rect> latestFaces;
    int faceCount;
    
    static constexpr int NUM_PROCESSING_THREADS = 4;
    static constexpr int FRAME_INTERVAL_MS = 100;
    
    std::function<void()> frameUpdateCallback;
    mutable std::mutex callbackMutex;
};
