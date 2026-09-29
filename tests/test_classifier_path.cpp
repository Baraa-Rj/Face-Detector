#include "FaceDetector.h"
#include <iostream>

// Run by CTest from several working directories: loading the classifier
// must not depend on where the binary is started from.
int main() {
    FaceDetector detector;
    if (!detector.loadClassifier()) {
        std::cerr << "FAIL: classifier did not load" << std::endl;
        return 1;
    }
    std::cout << "PASS: classifier loaded" << std::endl;
    return 0;
}
