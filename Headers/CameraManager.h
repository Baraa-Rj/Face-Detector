#pragma once

#include <opencv2/opencv.hpp>
#include <QObject>

class CameraManager : public QObject {
    Q_OBJECT

public:
    explicit CameraManager(QObject* parent = nullptr);
    ~CameraManager() = default;

    bool openCamera(int cameraIndex = 0);
    void closeCamera();
    bool isOpened() const;
    
    cv::Mat captureFrame();
    cv::Size getFrameSize() const;
    
    bool setCameraProperty(int property, double value);
    double getCameraProperty(int property) const;

signals:
    void cameraError(const QString& message);
    void frameReady(const cv::Mat& frame);

private:
    cv::VideoCapture m_camera;
    int m_cameraIndex;
};
