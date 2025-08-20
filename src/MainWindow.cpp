#include "../Headers/MainWindow.h"
#include <QApplication>
#include <QMessageBox>
#include <QStatusBar>
#include <iostream>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_centralWidget(nullptr)
    , m_videoLabel(nullptr)
    , m_statusLabel(nullptr)
    , m_startButton(nullptr)
    , m_frameProcessor(this)
    , m_cameraRunning(false)
    , m_faceCount(0)
{
    setupUI();
    setupConnections();
    
    // Set up display timer for smooth UI updates
    m_displayTimer.setInterval(16); // ~60 FPS for UI updates
    m_displayTimer.setSingleShot(false);
}

void MainWindow::setupUI() {
    setWindowTitle("Face Detection - Qt + OpenCV (Multi-threaded)");
    setMinimumSize(800, 600);
    
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);
    
    QVBoxLayout* mainLayout = new QVBoxLayout(m_centralWidget);
    
    m_videoLabel = new QLabel();
    m_videoLabel->setMinimumSize(640, 480);
    m_videoLabel->setStyleSheet("QLabel { border: 2px solid #ddd; background-color: #f8f9fa; }");
    m_videoLabel->setAlignment(Qt::AlignCenter);
    m_videoLabel->setText("Camera Feed");
    
    QHBoxLayout* controlLayout = new QHBoxLayout();
    
    m_startButton = new QPushButton("Start Camera");
    m_startButton->setStyleSheet("QPushButton { background-color: #51cf66; color: white; padding: 10px; font-size: 14px; border-radius: 5px; }");
    m_startButton->setMinimumHeight(40);
    
    controlLayout->addWidget(m_startButton);
    controlLayout->addStretch();
    
    m_statusLabel = new QLabel("Ready");
    m_statusLabel->setStyleSheet("QLabel { padding: 5px; background-color: #e9ecef; border-radius: 3px; }");
    
    mainLayout->addWidget(m_videoLabel);
    mainLayout->addLayout(controlLayout);
    mainLayout->addWidget(m_statusLabel);
}

void MainWindow::setupConnections() {
    connect(m_startButton, &QPushButton::clicked, this, &MainWindow::toggleCamera);
    connect(&m_frameProcessor, &FrameProcessor::frameProcessed, this, &MainWindow::onFrameProcessed);
    connect(&m_frameProcessor, &FrameProcessor::processingError, this, &MainWindow::onProcessingError);
    connect(&m_displayTimer, &QTimer::timeout, this, &MainWindow::updateDisplay);
}

void MainWindow::toggleCamera() {
    if (m_cameraRunning) {
        stopCamera();
    } else {
        startCamera();
    }
}

void MainWindow::startCamera() {
    m_frameProcessor.startProcessing();
    
    if (m_frameProcessor.isProcessing()) {
        m_cameraRunning = true;
        
        m_startButton->setText("Stop Camera");
        m_startButton->setStyleSheet("QPushButton { background-color: #ff6b6b; color: white; padding: 10px; font-size: 14px; border-radius: 5px; }");
        m_statusLabel->setText("Camera started - Processing frames...");
        
        // Start the display timer for smooth UI updates
        m_displayTimer.start();
    } else {
        QMessageBox::critical(this, "Error", "Could not start camera processing!");
    }
}

void MainWindow::stopCamera() {
    m_cameraRunning = false;
    m_frameProcessor.stopProcessing();
    
    // Stop the display timer
    m_displayTimer.stop();
    
    m_startButton->setText("Start Camera");
    m_startButton->setStyleSheet("QPushButton { background-color: #51cf66; color: white; padding: 10px; font-size: 14px; border-radius: 5px; }");
    m_videoLabel->setText("Camera Feed");
    m_statusLabel->setText("Camera stopped");
}

void MainWindow::onFrameProcessed() {
    // This slot is called when a new frame is processed
    // The actual display update happens in the timer-based updateDisplay method
    // for smooth UI updates
}

void MainWindow::onProcessingError(const QString& message) {
    QMessageBox::warning(this, "Processing Error", message);
    if (m_cameraRunning) {
        stopCamera();
    }
}

void MainWindow::updateDisplay() {
    if (!m_cameraRunning) {
        return;
    }
    
    // Get the latest processed frame from the frame processor
    cv::Mat frame = m_frameProcessor.getLatestFrame();
    
    if (!frame.empty()) {
        updateVideoDisplay(frame);
        
        // Update face count
        m_faceCount = m_frameProcessor.getFaceCount();
        m_statusLabel->setText(QString("Faces detected: %1").arg(m_faceCount));
    }
}

void MainWindow::updateVideoDisplay(const cv::Mat& frame) {
    if (frame.empty()) {
        return;
    }
    
    QImage qimg = matToQImage(frame);
    if (qimg.isNull()) {
        return;
    }
    
    QPixmap pixmap = QPixmap::fromImage(qimg);
    QPixmap scaledPixmap = pixmap.scaled(m_videoLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    m_videoLabel->setPixmap(scaledPixmap);
}

QImage MainWindow::matToQImage(const cv::Mat& mat) const {
    if (mat.empty()) {
        return QImage();
    }
    
    cv::Mat rgbMat;
    cv::cvtColor(mat, rgbMat, cv::COLOR_BGR2RGB);
    
    QImage qimg(rgbMat.data, rgbMat.cols, rgbMat.rows, 
                static_cast<int>(rgbMat.step), QImage::Format_RGB888);
    
    return qimg.copy();
}
