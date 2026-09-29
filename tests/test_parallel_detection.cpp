#include "FaceDetector.h"
#include "FrameProcessor.h"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <set>
#include <thread>
#include <tuple>
#include <vector>

// FrameProcessor runs several worker threads. cv::CascadeClassifier is not
// thread-safe, so each worker needs its own detector. This test feeds four
// different same-size images through FrameProcessor and checks every result
// against a single-threaded baseline for that image.

using RectSet = std::set<std::tuple<int, int, int, int>>;

static RectSet toSet(const std::vector<cv::Rect>& rects) {
    RectSet s;
    for (const auto& r : rects) {
        s.emplace(r.x, r.y, r.width, r.height);
    }
    return s;
}

static std::vector<cv::Mat> makeImages(const cv::Mat& face) {
    const cv::Size frameSize(640, 480);
    const int sizes[] = {200, 240, 280, 320};
    const cv::Point offsets[] = {{40, 30}, {360, 200}, {180, 100}, {300, 20}};

    std::vector<cv::Mat> images;
    for (int i = 0; i < 4; ++i) {
        cv::Mat image(frameSize, CV_8UC3, cv::Scalar(90 + 20 * i, 110, 130));
        cv::Mat scaled;
        cv::resize(face, scaled, cv::Size(sizes[i], sizes[i]));
        scaled.copyTo(image(cv::Rect(offsets[i], scaled.size())));
        images.push_back(image);
    }
    return images;
}

int main() {
    cv::Mat face = cv::imread(TEST_DATA_DIR "/astronaut.jpg");
    if (face.empty()) {
        std::cerr << "FAIL: could not read " TEST_DATA_DIR "/astronaut.jpg" << std::endl;
        return 1;
    }
    const std::vector<cv::Mat> images = makeImages(face);

    FaceDetector reference;
    if (!reference.loadClassifier()) {
        std::cerr << "FAIL: classifier did not load" << std::endl;
        return 1;
    }
    std::vector<RectSet> baseline;
    for (const auto& image : images) {
        baseline.push_back(toSet(reference.detectFaces(image)));
        if (baseline.back().empty()) {
            std::cerr << "FAIL: baseline found no face; test image is unusable" << std::endl;
            return 1;
        }
    }

    const int totalFrames = 600;
    const int maxInFlight = 8;  // stays below the queue bound, so no frame is dropped
    std::atomic<int> produced{0};
    std::atomic<int> processed{0};
    std::atomic<int> wrong{0};
    std::atomic<int> unknown{0};
    std::atomic<bool> done{false};

    auto source = [&]() -> cv::Mat {
        while (!done && produced - processed >= maxInFlight) {
            std::this_thread::sleep_for(std::chrono::microseconds(200));
        }
        if (done || produced >= totalFrames) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            return cv::Mat();
        }
        return images[produced++ % images.size()].clone();
    };

    std::mutex doneMutex;
    std::condition_variable doneCondition;

    FrameProcessor processor(source, 0);
    processor.setFacesDetectedCallback([&](const cv::Mat& frame, const std::vector<cv::Rect>& faces) {
        int index = -1;
        for (size_t i = 0; i < images.size(); ++i) {
            if (cv::norm(frame, images[i], cv::NORM_INF) == 0) {
                index = static_cast<int>(i);
                break;
            }
        }
        if (index < 0) {
            ++unknown;
        } else if (toSet(faces) != baseline[index]) {
            ++wrong;
        }
        if (++processed >= totalFrames) {
            std::lock_guard<std::mutex> locker(doneMutex);
            doneCondition.notify_all();
        }
    });

    processor.startProcessing();
    if (!processor.isProcessing()) {
        std::cerr << "FAIL: processing did not start" << std::endl;
        return 1;
    }

    {
        std::unique_lock<std::mutex> locker(doneMutex);
        doneCondition.wait_for(locker, std::chrono::seconds(120), [&]() { return processed >= totalFrames; });
    }
    done = true;
    processor.stopProcessing();

    std::cout << "processed=" << processed << " wrong=" << wrong << " unknown=" << unknown << std::endl;
    if (processed < totalFrames || wrong != 0 || unknown != 0) {
        std::cerr << "FAIL: parallel results differ from the single-thread baseline" << std::endl;
        return 1;
    }
    std::cout << "PASS" << std::endl;
    return 0;
}
