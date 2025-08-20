#include "../Headers/MainWindow.h"
#include "../Headers/FrameProcessor.h"
#include <QApplication>
#include <QImage>
#include <QPixmap>
#include <iostream>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , centralWidget(nullptr)
    , videoLabel(nullptr)
    , statusLabel(nullptr)
    , startButton(nullptr)
    , frameProcessor(std::make_unique<FrameProcessor>())
    , cameraRunning(false)
    , faceCount(0)
{
    setupUI();
    setupConnections();
    
    // Connect the pure C++ processor to our Qt UI through callback
    frameProcessor->setFrameUpdateCallback([this]() {
        // This callback runs in the C++ thread, so we need to post to Qt's event loop
        QMetaObject::invokeMethod(this, "onFrameUpdate", Qt::QueuedConnection);
    });
}

MainWindow::~MainWindow() {
    if (cameraRunning) {
        stopCamera();
    }
    // Smart pointer automatically cleans up
}

void MainWindow::closeEvent(QCloseEvent* event) {
    if (cameraRunning) {
        stopCamera();
    }
    event->accept();
}

void MainWindow::setupUI() {
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    
    // Video display area
    videoLabel = new QLabel();
    videoLabel->setMinimumSize(640, 480);
    videoLabel->setStyleSheet("QLabel { border: 2px solid #ddd; background-color: #f8f9fa; }");
    videoLabel->setAlignment(Qt::AlignCenter);
    videoLabel->setText("Camera Feed");
    
    // Control buttons
    QHBoxLayout* controlLayout = new QHBoxLayout();
    startButton = new QPushButton("Start Camera");
    startButton->setStyleSheet("QPushButton { background-color: #51cf66; color: white; padding: 10px; font-size: 14px; border-radius: 5px; }");
    startButton->setMinimumHeight(40);
    
    controlLayout->addWidget(startButton);
    
    // Status display
    statusLabel = new QLabel("Ready");
    statusLabel->setStyleSheet("QLabel { padding: 5px; background-color: #e9ecef; border-radius: 3px; }");
    
    mainLayout->addWidget(videoLabel);
    mainLayout->addLayout(controlLayout);
    mainLayout->addWidget(statusLabel);
}

void MainWindow::setupConnections() {
    connect(startButton, &QPushButton::clicked, this, &MainWindow::toggleCamera);
}

void MainWindow::toggleCamera() {
    if (cameraRunning) {
        stopCamera();
    } else {
        startCamera();
    }
}

void MainWindow::startCamera() {
    if (cameraRunning) {
        return;
    }
    
    // Start the frame processor
    frameProcessor->startProcessing();
    
    if (frameProcessor->isProcessing()) {
        cameraRunning = true;
        
        startButton->setText("Stop Camera");
        startButton->setStyleSheet("QPushButton { background-color: #ff6b6b; color: white; padding: 10px; font-size: 14px; border-radius: 5px; }");
        statusLabel->setText("Camera started - Processing frames...");
    } else {
        statusLabel->setText("Failed to start camera");
    }
}

void MainWindow::stopCamera() {
    if (!cameraRunning) {
        return;
    }
    
    cameraRunning = false;
    frameProcessor->stopProcessing();
    
    startButton->setText("Start Camera");
    startButton->setStyleSheet("QPushButton { background-color: #51cf66; color: white; padding: 10px; font-size: 14px; border-radius: 5px; }");
    videoLabel->setText("Camera Feed");
    statusLabel->setText("Camera stopped");
}

void MainWindow::onFrameUpdate() {
    if (!cameraRunning) {
        return;
    }
    
    // Get the latest processed frame
    cv::Mat frame = frameProcessor->getLatestFrame();
    
    if (!frame.empty()) {
        updateVideoDisplay(frame);
        
        // Update face count display
        faceCount = frameProcessor->getFaceCount();
        statusLabel->setText(QString("Faces detected: %1").arg(faceCount));
    }
}

void MainWindow::updateVideoDisplay(const cv::Mat& frame) {
    // Convert OpenCV Mat to QImage
    QImage qImage = matToQImage(frame);
    
    // Convert to QPixmap and scale to fit the label
    QPixmap pixmap = QPixmap::fromImage(qImage);
    QPixmap scaledPixmap = pixmap.scaled(videoLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    
    // Display the image
    videoLabel->setPixmap(scaledPixmap);
}

QImage MainWindow::matToQImage(const cv::Mat& mat) const {
    if (mat.type() == CV_8UC3) {
        // BGR to RGB conversion
        cv::Mat rgb;
        cv::cvtColor(mat, rgb, cv::COLOR_BGR2RGB);
        
        QImage img(rgb.data, rgb.cols, rgb.rows, rgb.step, QImage::Format_RGB888);
        return img.copy(); // Deep copy to ensure data ownership
    }
    
    return QImage();
}
