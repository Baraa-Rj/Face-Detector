#include "../Headers/FrameProcessor.h"
#include <iostream>
#include <chrono>

FrameProcessor::FrameProcessor()
    : processingActive(false)
    , shouldStop(false)
    , frameCounter(0)
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
    
    for (auto& worker : processingWorkers) {
        if (worker.joinable()) {
            worker.join();
        }
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
    
    // Start capture thread
    captureThread = std::thread(&FrameProcessor::frameCaptureThread, this);
    
    // Start multiple processing worker threads
    processingWorkers.clear();
    for (int i = 0; i < NUM_PROCESSING_THREADS; ++i) {
        processingWorkers.emplace_back(&FrameProcessor::frameProcessingWorker, this);
    }
    
    std::cout << "Started " << NUM_PROCESSING_THREADS << " processing threads for parallel execution" << std::endl;
}

void FrameProcessor::stopProcessing() {
    if (!processingActive) {
        return;
    }
    
    shouldStop = true;
    processingActive = false;
    
    frameQueueCondition.notify_all();
    
    cameraManager.closeCamera();
    
    if (captureThread.joinable()) {
        captureThread.join();
    }
    
    for (auto& worker : processingWorkers) {
        if (worker.joinable()) {
            worker.join();
        }
    }
    
    processingWorkers.clear();
}

bool FrameProcessor::isProcessing() const {
    return processingActive;
}

cv::Mat FrameProcessor::getLatestFrame() const {
    std::lock_guard<std::mutex> locker(resultsMutex);
    return latestFrame.clone();
}

std::vector<cv::Rect> FrameProcessor::getLatestFaces() const {
    std::lock_guard<std::mutex> locker(resultsMutex);
    return latestFaces;
}

int FrameProcessor::getFaceCount() const {
    std::lock_guard<std::mutex> locker(resultsMutex);
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
        cv::Mat frame = cameraManager.captureFrame();
        
        if (!frame.empty()) {
            {
                std::lock_guard<std::mutex> locker(frameQueueMutex);
                
                // Remove old frames if queue is full to prevent memory overflow
                while (frameQueue.size() >= MAX_QUEUE_SIZE) {
                    frameQueue.pop();
                }
                
                frameQueue.push(std::move(frame));
            }
            
            frameQueueCondition.notify_one();
        }
        
        // Small delay to control frame rate
        std::this_thread::sleep_for(std::chrono::milliseconds(FRAME_INTERVAL_MS));
    }
}

void FrameProcessor::frameProcessingWorker() {
    while (processingActive && !shouldStop) {
        cv::Mat frameToProcess;
        
        {
            std::unique_lock<std::mutex> locker(frameQueueMutex);
            
            frameQueueCondition.wait(locker, [this]() {
                return !frameQueue.empty() || shouldStop;
            });
            
            if (shouldStop) {
                break;
            }
            
            if (!frameQueue.empty()) {
                frameToProcess = std::move(frameQueue.front());
                frameQueue.pop();
            }
        }
        
        if (!frameToProcess.empty()) {
            int currentFrameNumber = ++frameCounter;
            
            // Process frame for face detection (this is the computationally intensive part)
            std::vector<cv::Rect> faces = faceDetector.detectFaces(frameToProcess);
            
            cv::Mat processedFrame;
            frameToProcess.copyTo(processedFrame);
            faceDetector.drawFaceRectangles(processedFrame, faces);
            
            drawFrameNumber(processedFrame, currentFrameNumber);
            
            // Update results atomically
            {
                std::lock_guard<std::mutex> locker(resultsMutex);
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
