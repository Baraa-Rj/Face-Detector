#pragma once

#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
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
    
    // Thread-safe getters
    cv::Mat getLatestFrame() const;
    std::vector<cv::Rect> getLatestFaces() const;
    int getFaceCount() const;
    
    // Callback for frame updates (Qt-free interface)
    void setFrameUpdateCallback(std::function<void()> callback);

private:
    void processFrames();
    
    CameraManager m_cameraManager;
    FaceDetector m_faceDetector;
    
    // Thread management (pure C++)
    std::thread m_processingThread;
    std::atomic<bool> m_isProcessing;
    std::atomic<bool> m_shouldStop;
    
    // Frame data (protected by mutex)
    mutable std::mutex m_frameMutex;
    cv::Mat m_latestFrame;
    std::vector<cv::Rect> m_latestFaces;
    int m_faceCount;
    
    // Synchronization
    std::condition_variable m_frameCondition;
    static constexpr int FRAME_INTERVAL_MS = 33; // ~30 FPS
    
    // Callback for frame updates
    std::function<void()> m_frameUpdateCallback;
};
