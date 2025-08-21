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
    
    if (captureThread.joinable()) {
        captureThread.join();
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
    
    // Start both threads
    captureThread = std::thread(&FrameProcessor::frameCaptureThread, this);
    processingThread = std::thread(&FrameProcessor::frameProcessingThread, this);
}

void FrameProcessor::stopProcessing() {
    if (!processingActive) {
        return;
    }
    
    shouldStop = true;
    processingActive = false;
    
    // Notify both threads to wake up and check shouldStop
    rawFrameCondition.notify_all();
    
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

void FrameProcessor::frameCaptureThread() {
    while (processingActive && !shouldStop) {
        auto startTime = std::chrono::steady_clock::now();
        
        // Check if camera is still open
        if (!cameraManager.isOpened()) {
            std::cerr << "Camera is not open in capture thread!" << std::endl;
            break;
        }
        
        cv::Mat frame = cameraManager.captureFrame();
        
        if (!frame.empty()) {
            // Add frame to the raw frame queue
            {
                std::lock_guard<std::mutex> locker(rawFrameMutex);
                
                // Limit queue size to prevent memory issues
                if (rawFrameQueue.size() >= MAX_RAW_FRAMES) {
                    rawFrameQueue.pop(); // Remove oldest frame
                }
                
                rawFrameQueue.push(frame.clone());
            }
            
            // Notify processing thread that new frame is available
            rawFrameCondition.notify_one();
        }
        
        // Control frame rate
        auto endTime = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        auto waitTime = std::max(0, FRAME_INTERVAL_MS - static_cast<int>(elapsed.count()));
        
        if (waitTime > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(waitTime));
        }
    }
}

void FrameProcessor::frameProcessingThread() {
    while (processingActive && !shouldStop) {
        cv::Mat frameToProcess;
        
        // Wait for a frame to be available
        {
            std::unique_lock<std::mutex> locker(rawFrameMutex);
            
            // Wait for frame or stop signal
            rawFrameCondition.wait(locker, [this]() {
                return !rawFrameQueue.empty() || shouldStop;
            });
            
            if (shouldStop) {
                break;
            }
            
            if (!rawFrameQueue.empty()) {
                frameToProcess = rawFrameQueue.front();
                rawFrameQueue.pop();
            }
        }
        
        if (!frameToProcess.empty()) {
            // Process the frame for face detection
            std::vector<cv::Rect> faces = faceDetector.detectFaces(frameToProcess);
            
            // Draw face rectangles on the processed frame
            cv::Mat processedFrame;
            frameToProcess.copyTo(processedFrame);
            faceDetector.drawFaceRectangles(processedFrame, faces);
            
            // Update the latest processed frame and results
            {
                std::lock_guard<std::mutex> locker(processedFrameMutex);
                latestFrame = std::move(processedFrame);
                latestFaces = faces;
                faceCount = static_cast<int>(faces.size());
            }
            
            // Notify UI thread about the update
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
