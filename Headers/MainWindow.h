#pragma once

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTimer>
#include <opencv2/opencv.hpp>

#include "FrameProcessor.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() = default;

private slots:
    void toggleCamera();
    void onFrameProcessed();
    void onProcessingError(const QString& message);
    void updateDisplay();

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
    
    FrameProcessor m_frameProcessor;
    QTimer m_displayTimer;
    
    bool m_cameraRunning;
    int m_faceCount;
};
