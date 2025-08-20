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

private:
    bool tryLoadClassifierFromPath(const std::string& path);
    
    cv::CascadeClassifier faceCascadeClassifier;
};
