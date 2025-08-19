#include <opencv2/opencv.hpp>
#include <iostream>

int main() {
    // Open the default camera (index 0)
    cv::VideoCapture cap(0);
    
    // Check if camera opened successfully
    if (!cap.isOpened()) {
        std::cerr << "Error: Could not open camera!" << std::endl;
        return -1;
    }
    
    // Load the Haar cascade classifier for face detection
    cv::CascadeClassifier faceCascade;
    
    // Try multiple possible paths for the classifier file
    std::vector<std::string> classifierPaths = {
        "haarcascade_frontalface_alt.xml",           // Current directory
        "../haarcascade_frontalface_alt.xml",        // Parent directory (for build folder)
        "../../haarcascade_frontalface_alt.xml"      // Two levels up
    };
    
    bool classifierLoaded = false;
    for (const auto& path : classifierPaths) {
        if (faceCascade.load(path)) {
            classifierLoaded = true;
            std::cout << "Loaded classifier from: " << path << std::endl;
            break;
        }
    }
    
    if (!classifierLoaded) {
        std::cerr << "Error: Could not load face cascade classifier!" << std::endl;
        std::cerr << "Make sure haarcascade_frontalface_alt.xml is accessible." << std::endl;
        return -1;
    }
    
    std::cout << "Face detection started. Press 'q' to quit." << std::endl;
    
    cv::Mat frame;
    std::vector<cv::Rect> faces;
    
    while (true) {
        // Capture frame from camera
        cap >> frame;
        
        // Check if frame was captured successfully
        if (frame.empty()) {
            std::cerr << "Error: Could not capture frame!" << std::endl;
            break;
        }
        
        // Convert to grayscale for face detection
        cv::Mat gray;
        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
        
        // Detect faces using Haar cascade
        faceCascade.detectMultiScale(
            gray,           // Input image
            faces,          // Output vector of detected faces
            1.1,            // Scale factor (how much image size is reduced at each scale)
            3,              // Min neighbors (how many neighbors each candidate rectangle should have)
            0,              // Flags (not used)
            cv::Size(30, 30) // Min size of face to detect
        );
        
        // Draw rectangles around detected faces
        for (const auto& face : faces) {
            cv::rectangle(frame, face, cv::Scalar(0, 255, 0), 2);
        }
        
        // Display the result
        cv::imshow("Face Detection", frame);
        
        // Wait for key press and check if 'q' was pressed
        char key = (char)cv::waitKey(1);
        if (key == 'q' || key == 'Q') {
            break;
        }
    }
    
    // Clean up
    cap.release();
    cv::destroyAllWindows();
    
    return 0;
}
