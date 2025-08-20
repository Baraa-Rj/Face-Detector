#pragma once

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <opencv2/opencv.hpp>
#include <vector>
#include <atomic>

#include "FaceDetector.h"
#include "CameraManager.h"

class FrameProcessor : public QObject {
    Q_OBJECT

public:
    explicit FrameProcessor(QObject* parent = nullptr);
    ~FrameProcessor();

    void startProcessing();
    void stopProcessing();
    bool isProcessing() const;
    
    // Thread-safe getters for the main thread
    cv::Mat getLatestFrame() const;
    std::vector<cv::Rect> getLatestFaces() const;
    int getFaceCount() const;

public slots:
    void processFrames();

signals:
    void frameProcessed();
    void processingError(const QString& message);

private:
    void processSingleFrame();
    
    CameraManager m_cameraManager;
    FaceDetector m_faceDetector;
    
    // Thread management
    QThread m_processingThread;
    std::atomic<bool> m_isProcessing;
    std::atomic<bool> m_shouldStop;
    
    // Frame data (protected by mutex)
    mutable QMutex m_frameMutex;
    cv::Mat m_latestFrame;
    std::vector<cv::Rect> m_latestFaces;
    int m_faceCount;
    
    // Synchronization
    QWaitCondition m_frameCondition;
    static constexpr int FRAME_INTERVAL_MS = 33; // ~30 FPS
};
