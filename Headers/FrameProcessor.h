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
    using FrameSource = std::function<cv::Mat()>;
    using FacesCallback = std::function<void(const cv::Mat& frame, const std::vector<cv::Rect>& faces)>;

    // frameSource replaces the camera when set; an empty Mat means "no frame yet".
    explicit FrameProcessor(FrameSource frameSource = nullptr, int frameIntervalMs = FRAME_INTERVAL_MS);
    ~FrameProcessor();

    void startProcessing();
    void stopProcessing();
    bool isProcessing() const;
    
    cv::Mat getLatestFrame() const;
    std::vector<cv::Rect> getLatestFaces() const;
    int getFaceCount() const;
    
    void setFrameUpdateCallback(std::function<void()> callback);
    // Called by a worker for every processed frame with the input frame and its faces.
    void setFacesDetectedCallback(FacesCallback callback);

private:
    void frameCaptureThread();
    void frameProcessingWorker(int workerIndex);
    bool loadWorkerClassifiers();
    void drawFrameNumber(cv::Mat& frame, int frameNumber);
    
    CameraManager cameraManager;
    // One detector per worker: cv::CascadeClassifier is not thread-safe.
    std::vector<FaceDetector> workerDetectors;
    
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
    FacesCallback facesDetectedCallback;
    mutable std::mutex callbackMutex;
    
    FrameSource frameSource;
    int frameIntervalMs;
};
