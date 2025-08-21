#include "../Headers/FrameProcessor.h"
#include <iostream>
#include <chrono>

FrameProcessor::FrameProcessor()
    : processingActive(false)
    , shouldStop(false)
    , frameCounter(0)
    , frameReady(false)
    , frameProcessed(true)
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
    
    if (captureThread.joinable()) {
        captureThread.join();
    }
    
    if (processingThread.joinable()) {
        processingThread.join();
    }
}

void FrameProcessor::startProcessing() {
    if (processingActive) {
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
    frameCounter = 0;
    frameReady = false;
    frameProcessed = true;
    
    captureThread = std::thread(&FrameProcessor::frameCaptureThread, this);
    processingThread = std::thread(&FrameProcessor::frameProcessingThread, this);
}

void FrameProcessor::stopProcessing() {
    if (!processingActive) {
        return;
    }
    
    shouldStop = true;
    processingActive = false;
    
    frameReadyCondition.notify_all();
    frameProcessedCondition.notify_all();
    
    cameraManager.closeCamera();
    
    if (captureThread.joinable()) {
        captureThread.join();
    }
    
    if (processingThread.joinable()) {
        processingThread.join();
    }
}

bool FrameProcessor::isProcessing() const {
    return processingActive;
}

cv::Mat FrameProcessor::getLatestFrame() const {
    std::lock_guard<std::mutex> locker(processedFrameMutex);
    return latestFrame.clone();
}

std::vector<cv::Rect> FrameProcessor::getLatestFaces() const {
    std::lock_guard<std::mutex> locker(processedFrameMutex);
    return latestFaces;
}

int FrameProcessor::getFaceCount() const {
    std::lock_guard<std::mutex> locker(processedFrameMutex);
    return faceCount;
}

void FrameProcessor::setFrameUpdateCallback(std::function<void()> callback) {
    std::lock_guard<std::mutex> locker(callbackMutex);
    frameUpdateCallback = callback;
}

void FrameProcessor::drawFrameNumber(cv::Mat& frame, int frameNumber) {
    std::string frameText = "Frame: " + std::to_string(frameNumber);
    
    int fontFace = cv::FONT_HERSHEY_SIMPLEX;
    double fontScale = 1.0;
    int thickness = 2;
    cv::Scalar color(0, 255, 0);
    
    cv::Point textPosition(20, 40);
    
    cv::putText(frame, frameText, textPosition, fontFace, fontScale, color, thickness);
}

void FrameProcessor::frameCaptureThread() {
    while (processingActive && !shouldStop) {
        {
            std::unique_lock<std::mutex> locker(frameBufferMutex);
            
            frameProcessedCondition.wait(locker, [this]() {
                return frameProcessed || shouldStop;
            });
            
            if (shouldStop) {
                break;
            }
        }
        
        cv::Mat frame = cameraManager.captureFrame();
        
        if (!frame.empty()) {
            {
                std::lock_guard<std::mutex> locker(frameBufferMutex);
                currentFrame = std::move(frame);
                frameReady = true;
                frameProcessed = false;
            }
            
            frameReadyCondition.notify_one();
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(FRAME_INTERVAL_MS));
    }
}

void FrameProcessor::frameProcessingThread() {
    while (processingActive && !shouldStop) {
        {
            std::unique_lock<std::mutex> locker(frameBufferMutex);
            
            frameReadyCondition.wait(locker, [this]() {
                return frameReady || shouldStop;
            });
            
            if (shouldStop) {
                break;
            }
        }
        
        cv::Mat frameToProcess;
        {
            std::lock_guard<std::mutex> locker(frameBufferMutex);
            frameToProcess = std::move(currentFrame);
        }
        
        if (!frameToProcess.empty()) {
            int currentFrameNumber = ++frameCounter;
            
            std::vector<cv::Rect> faces = faceDetector.detectFaces(frameToProcess);
            
            cv::Mat processedFrame;
            frameToProcess.copyTo(processedFrame);
            faceDetector.drawFaceRectangles(processedFrame, faces);
            
            drawFrameNumber(processedFrame, currentFrameNumber);
            
            {
                std::lock_guard<std::mutex> locker(processedFrameMutex);
                latestFrame = std::move(processedFrame);
                latestFaces = faces;
                faceCount = static_cast<int>(faces.size());
            }
            
            {
                std::lock_guard<std::mutex> locker(frameBufferMutex);
                frameReady = false;
                frameProcessed = true;
            }
            
            frameProcessedCondition.notify_one();
            
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
    }
}
