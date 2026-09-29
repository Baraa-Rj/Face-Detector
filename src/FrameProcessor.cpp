#include "../Headers/FrameProcessor.h"
#include <iostream>
#include <chrono>

FrameProcessor::FrameProcessor(FrameSource frameSource, int frameIntervalMs)
    : processingActive(false)
    , shouldStop(false)
    , frameCounter(0)
    , faceCount(0)
    , frameUpdateCallback(nullptr)
    , frameSource(std::move(frameSource))
    , frameIntervalMs(frameIntervalMs)
    , workerDetectors(NUM_PROCESSING_THREADS)
{
    if (!loadWorkerClassifiers()) {
        std::cerr << "Failed to load face detector classifier!" << std::endl;
    }
}

bool FrameProcessor::loadWorkerClassifiers() {
    for (auto& detector : workerDetectors) {
        if (!detector.loadClassifier()) {
            return false;
        }
    }
    return true;
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
    
    if (!frameSource && !cameraManager.isOpened()) {
        if (!cameraManager.openCamera(0)) {
            std::cerr << "Failed to open camera!" << std::endl;
            return;
        }
    }
    
    if (!loadWorkerClassifiers()) {
        std::cerr << "Failed to load face detector classifier!" << std::endl;
        return;
    }
    
    processingActive = true;
    shouldStop = false;
    frameCounter = 0;
    
    captureThread = std::thread(&FrameProcessor::frameCaptureThread, this);
    
    processingWorkers.clear();
    for (int i = 0; i < NUM_PROCESSING_THREADS; ++i) {
        processingWorkers.emplace_back(&FrameProcessor::frameProcessingWorker, this, i);
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

void FrameProcessor::setFacesDetectedCallback(FacesCallback callback) {
    std::lock_guard<std::mutex> locker(callbackMutex);
    facesDetectedCallback = callback;
}

void FrameProcessor::drawFrameNumber(cv::Mat& frame, int frameNumber) {
    if (frame.empty() || frame.rows <= 0 || frame.cols <= 0) {
        return;
    }
    
    std::string frameText = "Frame: " + std::to_string(frameNumber);
    
    int fontFace = cv::FONT_HERSHEY_SIMPLEX;
    double fontScale = 1.0;
    int thickness = 2;
    cv::Scalar color(0, 255, 0);
    
    cv::Point textPosition(20, 40);
    
    if (textPosition.x >= 0 && textPosition.y >= 0 && 
        textPosition.x < frame.cols && textPosition.y < frame.rows) {
        
        try {
            cv::putText(frame, frameText, textPosition, fontFace, fontScale, color, thickness);
        } catch (const cv::Exception& e) {
            std::cerr << "Error drawing frame number: " << e.what() << std::endl;
        }
    }
}

void FrameProcessor::frameCaptureThread() {
    while (processingActive && !shouldStop) {
        cv::Mat frame = frameSource ? frameSource() : cameraManager.captureFrame();
        
        if (!frame.empty()) {
            {
                std::lock_guard<std::mutex> locker(frameQueueMutex);
                
              
                frameQueue.push(std::move(frame));
            }
            
            frameQueueCondition.notify_one();
        }
        
        if (frameIntervalMs > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(frameIntervalMs));
        }
    }
}

void FrameProcessor::frameProcessingWorker(int workerIndex) {
    FaceDetector& faceDetector = workerDetectors[workerIndex];
    
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
            
            std::vector<cv::Rect> faces = faceDetector.detectFaces(frameToProcess);
            
            cv::Mat processedFrame;
            frameToProcess.copyTo(processedFrame);
            faceDetector.drawFaceRectangles(processedFrame, faces);
            
            drawFrameNumber(processedFrame, currentFrameNumber);
            
            {
                std::lock_guard<std::mutex> locker(resultsMutex);
                latestFrame = std::move(processedFrame);
                latestFaces = faces;
                faceCount = static_cast<int>(faces.size());
            }
            
            std::function<void()> callback;
            FacesCallback facesCallback;
            {
                std::lock_guard<std::mutex> locker(callbackMutex);
                callback = frameUpdateCallback;
                facesCallback = facesDetectedCallback;
            }
            
            if (facesCallback) {
                facesCallback(frameToProcess, faces);
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
