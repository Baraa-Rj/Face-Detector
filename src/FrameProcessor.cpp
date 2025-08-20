#include "../Headers/FrameProcessor.h"
#include <iostream>
#include <chrono>
#include <stdexcept>

FrameProcessor::FrameProcessor()
    : m_isProcessing(false)
    , m_shouldStop(false)
    , m_faceCount(0)
    , m_frameUpdateCallback(nullptr)
{
    if (!m_faceDetector.loadClassifier()) {
        std::cerr << "Could not load face cascade classifier!" << std::endl;
    }
}

FrameProcessor::~FrameProcessor() {
    stopProcessing();
    
    if (m_processingThread.joinable()) {
        m_processingThread.join();
    }
    
    if (m_cameraManager.isOpened()) {
        m_cameraManager.closeCamera();
    }
}

bool FrameProcessor::isValidCameraIndex(int index) const {
    return index >= 0 && index < MAX_CAMERA_INDEX;
}

void FrameProcessor::startProcessing() {
    if (m_isProcessing) {
        return;
    }
    
    // Validate camera index before opening
    if (!isValidCameraIndex(0)) {
        std::cerr << "Invalid camera index: 0" << std::endl;
        return;
    }
    
    if (!m_cameraManager.openCamera(0)) {
        std::cerr << "Could not open camera!" << std::endl;
        return;
    }
    
    m_isProcessing = true;
    m_shouldStop = false;
    
    // Start the processing thread
    m_processingThread = std::thread(&FrameProcessor::processFrames, this);
}

void FrameProcessor::stopProcessing() {
    if (!m_isProcessing) {
        return;
    }
    
    m_shouldStop = true;
    m_isProcessing = false;
    
    // Wake up the processing thread if it's waiting
    m_frameCondition.notify_all();
    
    // Close the camera
    m_cameraManager.closeCamera();
    
    // Wait for thread to finish
    if (m_processingThread.joinable()) {
        m_processingThread.join();
    }
}

bool FrameProcessor::isProcessing() const {
    return m_isProcessing;
}

cv::Mat FrameProcessor::getLatestFrame() const {
    std::lock_guard<std::mutex> locker(m_frameMutex);
    return m_latestFrame.clone();
}

std::vector<cv::Rect> FrameProcessor::getLatestFaces() const {
    std::lock_guard<std::mutex> locker(m_frameMutex);
    return m_latestFaces;
}

int FrameProcessor::getFaceCount() const {
    std::lock_guard<std::mutex> locker(m_frameMutex);
    return m_faceCount;
}

void FrameProcessor::setFrameUpdateCallback(std::function<void()> callback) {
    std::lock_guard<std::mutex> locker(m_callbackMutex);
    m_frameUpdateCallback = callback;
}

void FrameProcessor::processFrames() {
    while (m_isProcessing && !m_shouldStop) {
        auto startTime = std::chrono::steady_clock::now();
        
        // Capture and process frame
        cv::Mat frame = m_cameraManager.captureFrame();
        
        if (!frame.empty()) {
            std::vector<cv::Rect> faces = m_faceDetector.detectFaces(frame);
            
            // Process frame data first, then lock mutex for minimal time
            cv::Mat processedFrame;
            frame.copyTo(processedFrame); // More efficient than clone()
            m_faceDetector.drawFaceRectangles(processedFrame, faces);
            
            // Lock mutex only for data update
            {
                std::lock_guard<std::mutex> locker(m_frameMutex);
                m_latestFrame = std::move(processedFrame); // Use move for efficiency
                m_latestFaces = faces;
                m_faceCount = static_cast<int>(faces.size());
            }
            
            // Safely call callback with protection
            std::function<void()> callback;
            {
                std::lock_guard<std::mutex> locker(m_callbackMutex);
                callback = m_frameUpdateCallback;
            }
            
            if (callback) {
                try {
                    callback();
                } catch (const std::exception& e) {
                    std::cerr << "Callback error: " << e.what() << std::endl;
                }
            }
        }
        
        // Calculate time to wait for next frame
        auto endTime = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        auto waitTime = std::max(0, FRAME_INTERVAL_MS - static_cast<int>(elapsed.count()));
        
        // Wait for the next frame interval
        if (waitTime > 0) {
            std::unique_lock<std::mutex> locker(m_frameMutex);
            m_frameCondition.wait_for(locker, std::chrono::milliseconds(waitTime));
        }
    }
}
