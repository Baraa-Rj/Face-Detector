#pragma once

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>
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

private:
    void setupUI();
    void setupConnections();
    void startCamera();
    void stopCamera();
    void updateVideoDisplay(const cv::Mat& frame);
    QImage matToQImage(const cv::Mat& mat) const;

private:
    // UI Components
    QWidget* m_centralWidget;
    QLabel* m_videoLabel;
    QLabel* m_statusLabel;
    QPushButton* m_startButton;
    QTimer* m_frameTimer;
    
    // Core Components
    FaceDetector m_faceDetector;
    CameraManager m_cameraManager;
    
    // State
    bool m_cameraRunning;
    int m_faceCount;
};
