#pragma once

#include <opencv2/opencv.hpp>
#if CV_VERSION_MAJOR >= 5
#include <opencv2/xobjdetect.hpp>
#endif
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
