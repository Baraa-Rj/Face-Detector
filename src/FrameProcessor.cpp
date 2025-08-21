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
    frameCounter = 0;
    frameReady = false;
    frameProcessed = true;
    
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
    // Convert frame number to string
    std::string frameText = "Frame: " + std::to_string(frameNumber);
    
    // Set text properties
    int fontFace = cv::FONT_HERSHEY_SIMPLEX;
    double fontScale = 1.0;
    int thickness = 2;
    cv::Scalar color(0, 255, 0); // Green color
    
    // Position text in upper left corner with some padding
    cv::Point textPosition(20, 40);
    
    // Draw the frame number text
    cv::putText(frame, frameText, textPosition, fontFace, fontScale, color, thickness);
}

void FrameProcessor::frameCaptureThread() {
    while (processingActive && !shouldStop) {
        // Wait for the previous frame to be processed
        {
            std::unique_lock<std::mutex> locker(frameBufferMutex);
            frameProcessedCondition.wait(locker, [this]() {
                return frameProcessed || shouldStop;
            });
            
            if (shouldStop) {
                break;
            }
        }
        
        // Capture a new frame
        cv::Mat frame = cameraManager.captureFrame();
        
        if (!frame.empty()) {
            // Store the frame and mark it as ready for processing
            {
                std::lock_guard<std::mutex> locker(frameBufferMutex);
                currentFrame = frame.clone();
                frameReady = true;
                frameProcessed = false;
            }
            
            // Notify processing thread that new frame is ready
            frameReadyCondition.notify_one();
        }
        
        // Control frame rate
        std::this_thread::sleep_for(std::chrono::milliseconds(FRAME_INTERVAL_MS));
    }
}

void FrameProcessor::frameProcessingThread() {
    while (processingActive && !shouldStop) {
        // Wait for a frame to be available
        {
            std::unique_lock<std::mutex> locker(frameBufferMutex);
            
            frameReadyCondition.wait(locker, [this]() {
                return frameReady || shouldStop;
            });
            
            if (shouldStop) {
                break;
            }
        }
        
        // Process the current frame
        cv::Mat frameToProcess;
        {
            std::lock_guard<std::mutex> locker(frameBufferMutex);
            frameToProcess = currentFrame.clone();
        }
        
        if (!frameToProcess.empty()) {
            // Increment frame counter
            int currentFrameNumber = ++frameCounter;
            
            // Process the frame for face detection
            std::vector<cv::Rect> faces = faceDetector.detectFaces(frameToProcess);
            
            // Draw face rectangles on the processed frame
            cv::Mat processedFrame;
            frameToProcess.copyTo(processedFrame);
            faceDetector.drawFaceRectangles(processedFrame, faces);
            
            // Draw frame number on the processed frame
            drawFrameNumber(processedFrame, currentFrameNumber);
            
            // Update the latest processed frame and results
            {
                std::lock_guard<std::mutex> locker(processedFrameMutex);
                latestFrame = std::move(processedFrame);
                latestFaces = faces;
                faceCount = static_cast<int>(faces.size());
            }
            
            // Mark frame as processed
            {
                std::lock_guard<std::mutex> locker(frameBufferMutex);
                frameReady = false;
                frameProcessed = true;
            }
            
            // Notify capture thread that processing is complete
            frameProcessedCondition.notify_one();
            
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
