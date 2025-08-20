#pragma once

#include <QObject>
#include <QTimer>
#include <QMutex>
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
    void processFrame();

signals:
    void frameProcessed();
    void processingError(const QString& message);

private:
    CameraManager m_cameraManager;
    FaceDetector m_faceDetector;
    
    // Timer-based processing
    QTimer m_processingTimer;
    std::atomic<bool> m_isProcessing;
    
    // Frame data (protected by mutex)
    mutable QMutex m_frameMutex;
    cv::Mat m_latestFrame;
    std::vector<cv::Rect> m_latestFaces;
    int m_faceCount;
    
    static constexpr int FRAME_INTERVAL_MS = 33; // ~30 FPS
};
