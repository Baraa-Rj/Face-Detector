#include "FrameProcessor.h"

#include <atomic>
#include <chrono>
#include <iostream>
#include <mutex>
#include <set>
#include <thread>

// When workers fall behind, the capture thread must drop the oldest queued
// frame instead of letting the queue grow without limit.

static const int kWorkers = 4;
static const int kQueueBound = 10;  // FrameProcessor::MAX_QUEUE_SIZE
static const int kBacklog = 50;

static cv::Mat taggedFrame(int id) {
    return cv::Mat(64, 64, CV_8UC3, cv::Scalar(id % 256, id / 256, 0));
}

static int frameId(const cv::Mat& frame) {
    cv::Vec3b px = frame.at<cv::Vec3b>(0, 0);
    return px[0] + 256 * px[1];
}

template <typename Pred>
static bool waitFor(Pred pred, int timeoutMs) {
    auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeoutMs);
    while (!pred()) {
        if (std::chrono::steady_clock::now() > deadline) {
            return false;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    return true;
}

int main() {
    std::atomic<int> produced{0};
    std::atomic<int> limit{kWorkers};
    std::atomic<int> blockedWorkers{0};
    std::atomic<bool> releaseWorkers{false};
    std::mutex idsMutex;
    std::set<int> processedIds;

    auto source = [&]() -> cv::Mat {
        if (produced >= limit) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            return cv::Mat();
        }
        return taggedFrame(++produced);
    };

    FrameProcessor processor(source, 0);
    processor.setFacesDetectedCallback([&](const cv::Mat& frame, const std::vector<cv::Rect>&) {
        {
            std::lock_guard<std::mutex> locker(idsMutex);
            processedIds.insert(frameId(frame));
        }
        ++blockedWorkers;
        while (!releaseWorkers) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    });

    processor.startProcessing();

    // Occupy every worker, then queue a backlog while they are busy.
    if (!waitFor([&]() { return blockedWorkers == kWorkers; }, 10000)) {
        std::cerr << "FAIL: workers did not pick up the first frames" << std::endl;
        return 1;
    }
    limit = kWorkers + kBacklog;
    if (!waitFor([&]() { return produced == limit; }, 10000)) {
        std::cerr << "FAIL: backlog was not produced" << std::endl;
        return 1;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    releaseWorkers = true;

    waitFor([&]() { return blockedWorkers >= kWorkers + kQueueBound; }, 10000);
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    processor.stopProcessing();

    std::set<int> expected;
    for (int id = 1; id <= kWorkers; ++id) {
        expected.insert(id);
    }
    for (int id = kWorkers + kBacklog - kQueueBound + 1; id <= kWorkers + kBacklog; ++id) {
        expected.insert(id);
    }

    std::cout << "processed " << processedIds.size() << " frames" << std::endl;
    if (processedIds != expected) {
        std::cerr << "FAIL: expected the first " << kWorkers << " frames plus the newest "
                  << kQueueBound << " of the backlog" << std::endl;
        return 1;
    }
    std::cout << "PASS" << std::endl;
    return 0;
}
