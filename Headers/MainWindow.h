#pragma once

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QGroupBox>
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
    void updateScaleFactor(int value);
    void updateMinNeighbors(int value);
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
    QLabel* m_scaleLabel;
    QLabel* m_neighborsLabel;
    QTimer* m_frameTimer;
    
    // Core Components
    FaceDetector m_faceDetector;
    CameraManager m_cameraManager;
    
    // State
    bool m_cameraRunning;
    int m_faceCount;
};
