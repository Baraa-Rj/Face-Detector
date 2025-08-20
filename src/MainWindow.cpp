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
    
    frameProcessor->setFrameUpdateCallback([this]() {
        QMetaObject::invokeMethod(this, "onFrameUpdate", Qt::QueuedConnection);
    });
}

MainWindow::~MainWindow() {
    if (cameraRunning) {
        stopCamera();
    }
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
    
    videoLabel = new QLabel();
    videoLabel->setMinimumSize(640, 480);
    videoLabel->setStyleSheet("QLabel { border: 2px solid #ddd; background-color: #f8f9fa; }");
    videoLabel->setAlignment(Qt::AlignCenter);
    videoLabel->setText("Camera Feed");
    
    QHBoxLayout* controlLayout = new QHBoxLayout();
    startButton = new QPushButton("Start Camera");
    startButton->setStyleSheet("QPushButton { background-color: #51cf66; color: white; padding: 10px; font-size: 14px; border-radius: 5px; }");
    startButton->setMinimumHeight(40);
    
    controlLayout->addWidget(startButton);
    
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
    
    cv::Mat frame = frameProcessor->getLatestFrame();
    
    if (!frame.empty()) {
        updateVideoDisplay(frame);
        
        faceCount = frameProcessor->getFaceCount();
        statusLabel->setText(QString("Faces detected: %1").arg(faceCount));
    }
}

void MainWindow::updateVideoDisplay(const cv::Mat& frame) {
    QImage qImage = matToQImage(frame);
    
    QPixmap pixmap = QPixmap::fromImage(qImage);
    QPixmap scaledPixmap = pixmap.scaled(videoLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    
    videoLabel->setPixmap(scaledPixmap);
}

QImage MainWindow::matToQImage(const cv::Mat& mat) const {
    if (mat.type() == CV_8UC3) {
        cv::Mat rgb;
        cv::cvtColor(mat, rgb, cv::COLOR_BGR2RGB);
        
        QImage img(rgb.data, rgb.cols, rgb.rows, rgb.step, QImage::Format_RGB888);
        return img.copy();
    }
    
    return QImage();
}
