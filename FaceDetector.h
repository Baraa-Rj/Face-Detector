#pragma once

#include <opencv2/opencv.hpp>
#include <vector>
#include <string>

class FaceDetector {
public:
    FaceDetector();
    ~FaceDetector() = default;

    bool loadClassifier();
    std::vector<cv::Rect> detectFaces(const cv::Mat& frame);
    void drawFaceRectangles(cv::Mat& frame, const std::vector<cv::Rect>& faces) const;
    
    void setScaleFactor(double scaleFactor);
    void setMinNeighbors(int minNeighbors);
    void setMinSize(const cv::Size& minSize);
    
    double getScaleFactor() const { return m_scaleFactor; }
    int getMinNeighbors() const { return m_minNeighbors; }
    cv::Size getMinSize() const { return m_minSize; }

private:
    cv::CascadeClassifier m_faceCascade;
    double m_scaleFactor;
    int m_minNeighbors;
    cv::Size m_minSize;
    
    std::vector<std::string> getClassifierPaths() const;
};
