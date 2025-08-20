#include "../Headers/FrameProcessor.h"
#include <iostream>
#include <chrono>

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
    
    processingThread = std::thread(&FrameProcessor::processFrames, this);
}

void FrameProcessor::stopProcessing() {
    if (!processingActive) {
        return;
    }
    
    shouldStop = true;
    processingActive = false;
    
    frameCondition.notify_all();
    
    cameraManager.closeCamera();
    
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
        
        cv::Mat frame = cameraManager.captureFrame();
        
        if (!frame.empty()) {
            std::vector<cv::Rect> faces = faceDetector.detectFaces(frame);
            
            cv::Mat processedFrame;
            frame.copyTo(processedFrame);
            faceDetector.drawFaceRectangles(processedFrame, faces);
            
            {
                std::lock_guard<std::mutex> locker(frameMutex);
                latestFrame = std::move(processedFrame);
                latestFaces = faces;
                faceCount = static_cast<int>(faces.size());
            }
            
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
        
        auto endTime = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        auto waitTime = std::max(0, FRAME_INTERVAL_MS - static_cast<int>(elapsed.count()));
        
        if (waitTime > 0) {
            std::unique_lock<std::mutex> locker(frameMutex);
            frameCondition.wait_for(locker, std::chrono::milliseconds(waitTime));
        }
    }
}
