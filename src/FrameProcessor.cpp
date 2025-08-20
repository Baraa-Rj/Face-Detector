#include "../Headers/FrameProcessor.h"
#include <iostream>
#include <chrono>
#include <stdexcept>

FrameProcessor::FrameProcessor()
    : processingActive(false)
    , shouldStop(false)
    , faceCount(0)
    , frameUpdateCallback(nullptr)
{
    if (!faceDetector.loadClassifier()) {
        std::cerr << "Failed to load face detector classifier!" << std::endl;
    }
}

FrameProcessor::~FrameProcessor() {
    if (processingActive) {
        stopProcessing();
    }
    
    if (processingThread.joinable()) {
        processingThread.join();
    }
}

bool FrameProcessor::isValidCameraIndex(int index) const {
    return index >= 0 && index < MAX_CAMERA_INDEX;
}

void FrameProcessor::startProcessing() {
    if (processingActive) {
        return;
    }
    
    // Validate camera index before opening
    if (!isValidCameraIndex(0)) {
        std::cerr << "Invalid camera index: 0" << std::endl;
        return;
    }
    
    if (!cameraManager.isOpened()) {
        if (!cameraManager.openCamera(0)) {
            std::cerr << "Failed to open camera!" << std::endl;
            return;
        }
    }
    
    if (!faceDetector.loadClassifier()) {
        std::cerr << "Failed to load face detector classifier!" << std::endl;
        return;
    }
    
    processingActive = true;
    shouldStop = false;
    
    // Start the processing thread
    processingThread = std::thread(&FrameProcessor::processFrames, this);
}

void FrameProcessor::stopProcessing() {
    if (!processingActive) {
        return;
    }
    
    shouldStop = true;
    processingActive = false;
    
    // Wake up the processing thread if it's waiting
    frameCondition.notify_all();
    
    // Close the camera
    cameraManager.closeCamera();
    
    // Wait for thread to finish
    if (processingThread.joinable()) {
        processingThread.join();
    }
}

bool FrameProcessor::isProcessing() const {
    return processingActive;
}

cv::Mat FrameProcessor::getLatestFrame() const {
    std::lock_guard<std::mutex> locker(frameMutex);
    return latestFrame.clone();
}

std::vector<cv::Rect> FrameProcessor::getLatestFaces() const {
    std::lock_guard<std::mutex> locker(frameMutex);
    return latestFaces;
}

int FrameProcessor::getFaceCount() const {
    std::lock_guard<std::mutex> locker(frameMutex);
    return faceCount;
}

void FrameProcessor::setFrameUpdateCallback(std::function<void()> callback) {
    std::lock_guard<std::mutex> locker(callbackMutex);
    frameUpdateCallback = callback;
}

void FrameProcessor::processFrames() {
    while (processingActive && !shouldStop) {
        auto startTime = std::chrono::steady_clock::now();
        
        // Capture and process frame
        cv::Mat frame = cameraManager.captureFrame();
        
        if (!frame.empty()) {
            std::vector<cv::Rect> faces = faceDetector.detectFaces(frame);
            
            // Process frame data first, then lock mutex for minimal time
            cv::Mat processedFrame;
            frame.copyTo(processedFrame); // More efficient than clone()
            faceDetector.drawFaceRectangles(processedFrame, faces);
            
            // Lock mutex only for data update
            {
                std::lock_guard<std::mutex> locker(frameMutex);
                latestFrame = std::move(processedFrame); // Use move for efficiency
                latestFaces = faces;
                faceCount = static_cast<int>(faces.size());
            }
            
            // Safely call callback with protection
            std::function<void()> callback;
            {
                std::lock_guard<std::mutex> locker(callbackMutex);
                callback = frameUpdateCallback;
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
            std::unique_lock<std::mutex> locker(frameMutex);
            frameCondition.wait_for(locker, std::chrono::milliseconds(waitTime));
        }
    }
}
