#pragma once

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCloseEvent>
#include <opencv2/opencv.hpp>

// Forward declaration - Qt-free interface
class FrameProcessor;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void toggleCamera();
    void onFrameUpdate();

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
    
    // Pure C++ processor (no Qt dependencies)
    FrameProcessor* m_frameProcessor;
    
    bool m_cameraRunning;
    int m_faceCount;
};
