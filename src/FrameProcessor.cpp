#include "../Headers/FrameProcessor.h"
#include <QDebug>
#include <QThread>
#include <chrono>

FrameProcessor::FrameProcessor(QObject* parent)
    : QObject(parent)
    , m_isProcessing(false)
    , m_shouldStop(false)
    , m_faceCount(0)
{
    // Move this object to the processing thread
    this->moveToThread(&m_processingThread);
    
    // Connect the thread's started signal to our processing slot
    connect(&m_processingThread, &QThread::started, this, &FrameProcessor::processFrames);
    
    // Load the face detector classifier
    if (!m_faceDetector.loadClassifier()) {
        qWarning() << "Could not load face cascade classifier!";
    }
}

FrameProcessor::~FrameProcessor() {
    stopProcessing();
    m_processingThread.quit();
    m_processingThread.wait();
}

void FrameProcessor::startProcessing() {
    if (m_isProcessing) {
        return;
    }
    
    if (!m_cameraManager.openCamera(0)) {
        emit processingError("Could not open camera!");
        return;
    }
    
    m_isProcessing = true;
    m_shouldStop = false;
    
    // Start the processing thread
    m_processingThread.start();
}

void FrameProcessor::stopProcessing() {
    if (!m_isProcessing) {
        return;
    }
    
    m_shouldStop = true;
    m_isProcessing = false;
    
    // Wake up the processing thread if it's waiting
    m_frameCondition.wakeAll();
    
    // Close the camera
    m_cameraManager.closeCamera();
}

bool FrameProcessor::isProcessing() const {
    return m_isProcessing;
}

cv::Mat FrameProcessor::getLatestFrame() const {
    QMutexLocker locker(&m_frameMutex);
    return m_latestFrame.clone();
}

std::vector<cv::Rect> FrameProcessor::getLatestFaces() const {
    QMutexLocker locker(&m_frameMutex);
    return m_latestFaces;
}

int FrameProcessor::getFaceCount() const {
    QMutexLocker locker(&m_frameMutex);
    return m_faceCount;
}

void FrameProcessor::processFrames() {
    while (m_isProcessing && !m_shouldStop) {
        auto startTime = std::chrono::steady_clock::now();
        
        processSingleFrame();
        
        // Calculate time to wait for next frame
        auto endTime = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        auto waitTime = std::max(0, FRAME_INTERVAL_MS - static_cast<int>(elapsed.count()));
        
        // Wait for the next frame interval
        if (waitTime > 0) {
            QMutexLocker locker(&m_frameMutex);
            m_frameCondition.wait(&m_frameMutex, waitTime);
        }
    }
}

void FrameProcessor::processSingleFrame() {
    // Capture frame from camera
    cv::Mat frame = m_cameraManager.captureFrame();
    
    if (frame.empty()) {
        return;
    }
    
    // Detect faces in the frame
    std::vector<cv::Rect> faces = m_faceDetector.detectFaces(frame);
    
    // Draw face rectangles on the frame
    cv::Mat processedFrame = frame.clone();
    m_faceDetector.drawFaceRectangles(processedFrame, faces);
    
    // Update the shared data (protected by mutex)
    {
        QMutexLocker locker(&m_frameMutex);
        m_latestFrame = processedFrame;
        m_latestFaces = faces;
        m_faceCount = static_cast<int>(faces.size());
    }
    
    // Emit signal to notify main thread that new frame is ready
    emit frameProcessed();
}
