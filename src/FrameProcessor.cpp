#include "../Headers/FrameProcessor.h"
#include <QDebug>
#include <QTimer>

FrameProcessor::FrameProcessor(QObject* parent)
    : QObject(parent)
    , m_isProcessing(false)
    , m_faceCount(0)
{
    if (!m_faceDetector.loadClassifier()) {
        qWarning() << "Could not load face cascade classifier!";
    }
    
    m_processingTimer.setSingleShot(false);
    m_processingTimer.setInterval(FRAME_INTERVAL_MS);
    connect(&m_processingTimer, &QTimer::timeout, this, &FrameProcessor::processFrame);
}

FrameProcessor::~FrameProcessor() {
    stopProcessing();
    
    if (m_cameraManager.isOpened()) {
        m_cameraManager.closeCamera();
    }
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
    m_processingTimer.start();
}

void FrameProcessor::stopProcessing() {
    if (!m_isProcessing) {
        return;
    }
    
    m_isProcessing = false;
    m_processingTimer.stop();
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

void FrameProcessor::processFrame() {
    if (!m_isProcessing) {
        return;
    }
    
    cv::Mat frame = m_cameraManager.captureFrame();
    
    if (frame.empty()) {
        return;
    }
    
    std::vector<cv::Rect> faces = m_faceDetector.detectFaces(frame);
    
    cv::Mat processedFrame = frame.clone();
    m_faceDetector.drawFaceRectangles(processedFrame, faces);
    
    {
        QMutexLocker locker(&m_frameMutex);
        m_latestFrame = processedFrame;
        m_latestFaces = faces;
        m_faceCount = static_cast<int>(faces.size());
    }
    
    emit frameProcessed();
}
