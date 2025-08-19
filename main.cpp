#include <opencv2/opencv.hpp>
#include <iostream>

int main() {
    cv::VideoCapture cap(0);
    
    if (!cap.isOpened()) {
        std::cerr << "Error: Could not open camera!" << std::endl;
        return -1;
    }
    
    cv::CascadeClassifier faceCascade;
    
    std::vector<std::string> classifierPaths = {
        "haarcascade_frontalface_alt.xml",
        "../haarcascade_frontalface_alt.xml",
        "../../haarcascade_frontalface_alt.xml"
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
        cap >> frame;
        
        if (frame.empty()) {
            std::cerr << "Error: Could not capture frame!" << std::endl;
            break;
        }
        
        cv::Mat gray;
        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
        
        faceCascade.detectMultiScale(gray, faces, 1.1, 3, 0, cv::Size(30, 30));
        
        for (const auto& face : faces) {
            cv::rectangle(frame, face, cv::Scalar(0, 255, 0), 2);
        }
        
        cv::imshow("Face Detection", frame);
        
        char key = (char)cv::waitKey(1);
        if (key == 'q' || key == 'Q') {
            break;
        }
    }
    
    cap.release();
    cv::destroyAllWindows();
    
    return 0;
}
