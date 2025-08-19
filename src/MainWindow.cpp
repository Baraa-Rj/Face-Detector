#include "../Headers/MainWindow.h"
#include <QApplication>
#include <QMessageBox>
#include <QStatusBar>
#include <iostream> // Added for debugging

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_centralWidget(nullptr)
    , m_videoLabel(nullptr)
    , m_statusLabel(nullptr)
    , m_startButton(nullptr)
    , m_frameTimer(new QTimer(this))
    , m_cameraManager(this)
    , m_cameraRunning(false)
    , m_faceCount(0)
{
    setupUI();
    setupConnections();
    
    if (!m_faceDetector.loadClassifier()) {
        QMessageBox::critical(this, "Error", "Could not load face cascade classifier!");
    }
}

void MainWindow::setupUI() {
    setWindowTitle("Face Detection - Qt + OpenCV");
    setMinimumSize(800, 600);
    
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);
    
    QVBoxLayout* mainLayout = new QVBoxLayout(m_centralWidget);
    
    // Video display
    m_videoLabel = new QLabel();
    m_videoLabel->setMinimumSize(640, 480);
    m_videoLabel->setStyleSheet("QLabel { border: 2px solid #ddd; background-color: #f8f9fa; }");
    m_videoLabel->setAlignment(Qt::AlignCenter);
    m_videoLabel->setText("Camera Feed");
    
    // Controls
    QHBoxLayout* controlLayout = new QHBoxLayout();
    
    m_startButton = new QPushButton("Start Camera");
    m_startButton->setStyleSheet("QPushButton { background-color: #51cf66; color: white; padding: 10px; font-size: 14px; border-radius: 5px; }");
    m_startButton->setMinimumHeight(40);
    
    controlLayout->addWidget(m_startButton);
    controlLayout->addStretch();
    
    // Status bar
    m_statusLabel = new QLabel("Ready");
    m_statusLabel->setStyleSheet("QLabel { padding: 5px; background-color: #e9ecef; border-radius: 3px; }");
    
    mainLayout->addWidget(m_videoLabel);
    mainLayout->addLayout(controlLayout);
    mainLayout->addWidget(m_statusLabel);
}

void MainWindow::setupConnections() {
    connect(m_startButton, &QPushButton::clicked, this, &MainWindow::toggleCamera);
    connect(m_frameTimer, &QTimer::timeout, this, &MainWindow::processFrame);
    connect(&m_cameraManager, &CameraManager::cameraError, this, &MainWindow::onCameraError);
}

void MainWindow::processFrame() {
    if (!m_cameraRunning) {
        return;
    }
    
    cv::Mat frame = m_cameraManager.captureFrame();
    
    if (frame.empty()) {
        return;
    }
    
    // Detect faces
    std::vector<cv::Rect> faces = m_faceDetector.detectFaces(frame);
    
    // Draw rectangles around faces
    m_faceDetector.drawFaceRectangles(frame, faces);
    
    // Update display
    updateVideoDisplay(frame);
    
    // Update status
    m_faceCount = static_cast<int>(faces.size());
    m_statusLabel->setText(QString("Faces detected: %1").arg(m_faceCount));
}

void MainWindow::toggleCamera() {
    if (m_cameraRunning) {
        stopCamera();
    } else {
        startCamera();
    }
}

void MainWindow::startCamera() {
    if (m_cameraManager.openCamera(0)) {
        m_cameraRunning = true;
        m_frameTimer->start(30); // ~30 FPS
        m_startButton->setText("Stop Camera");
        m_startButton->setStyleSheet("QPushButton { background-color: #ff6b6b; color: white; padding: 10px; font-size: 14px; border-radius: 5px; }");
        m_statusLabel->setText("Camera started");
    } else {
        QMessageBox::critical(this, "Error", "Could not open camera!");
    }
}

void MainWindow::stopCamera() {
    m_cameraRunning = false;
    m_frameTimer->stop();
    m_cameraManager.closeCamera();
    m_startButton->setText("Start Camera");
    m_startButton->setStyleSheet("QPushButton { background-color: #51cf66; color: white; padding: 10px; font-size: 14px; border-radius: 5px; }");
    m_videoLabel->setText("Camera Feed");
    m_statusLabel->setText("Camera stopped");
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
    if (pixmap.isNull()) {
        return;
    }
    
    // Scale the pixmap to fit the label while maintaining aspect ratio
    QPixmap scaledPixmap = pixmap.scaled(m_videoLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    m_videoLabel->setPixmap(scaledPixmap);
}

QImage MainWindow::matToQImage(const cv::Mat& mat) const {
    if (mat.empty()) {
        return QImage();
    }
    
    // Convert BGR to RGB
    cv::Mat rgbMat;
    cv::cvtColor(mat, rgbMat, cv::COLOR_BGR2RGB);
    
    // Create QImage with proper data handling
    QImage qimg(rgbMat.data, rgbMat.cols, rgbMat.rows, 
                static_cast<int>(rgbMat.step), QImage::Format_RGB888);
    
    // Create a deep copy to ensure data ownership
    return qimg.copy();
}

void MainWindow::onCameraError(const QString& message) {
    QMessageBox::warning(this, "Camera Error", message);
    if (m_cameraRunning) {
        stopCamera();
    }
}
