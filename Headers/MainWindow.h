#pragma once

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <chrono>
#include <opencv2/opencv.hpp>

#include "FaceDetector.h"
#include "CameraManager.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() = default;

private slots:
    void processFrame();
    void toggleCamera();
    void onCameraError(const QString& message);
    void startFrameProcessing();

private:
    void setupUI();
    void setupConnections();
    void startCamera();
    void stopCamera();
    void updateVideoDisplay(const cv::Mat& frame);
    QImage matToQImage(const cv::Mat& mat) const;

private:
    QWidget* m_centralWidget;
    QLabel* m_videoLabel;
    QLabel* m_statusLabel;
    QPushButton* m_startButton;
    
    FaceDetector m_faceDetector;
    CameraManager m_cameraManager;
    
    bool m_cameraRunning;
    int m_faceCount;
    
    std::chrono::steady_clock::time_point m_lastFrameTime;
    static constexpr int FRAME_INTERVAL_MS = 33;
};
