#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QGroupBox>
#include <QStatusBar>
#include <QTimer>
#include <QFileDialog>
#include <QMessageBox>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/objdetect.hpp>

class FaceDetectionWidget : public QWidget {
    Q_OBJECT

public:
    FaceDetectionWidget(QWidget *parent = nullptr) : QWidget(parent) {
        setupUI();
        setupCamera();
        setupFaceDetection();
        
        connect(timer, &QTimer::timeout, this, &FaceDetectionWidget::processFrame);
        timer->start(30);
    }

private slots:
    void processFrame() {
        if (!cap.isOpened()) return;
        
        cv::Mat frame;
        cap >> frame;
        
        if (frame.empty()) return;
        
        cv::Mat gray;
        cv::cvtColor(frame, gray, cv::COLOR_BGR2RGB);
        
        std::vector<cv::Rect> faces;
        faceCascade.detectMultiScale(gray, faces, scaleFactor, minNeighbors, 0, cv::Size(30, 30));
        
        for (const auto& face : faces) {
            cv::rectangle(frame, face, cv::Scalar(0, 255, 0), 2);
        }
        
        QImage qimg(frame.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888);
        videoLabel->setPixmap(QPixmap::fromImage(qimg).scaled(videoLabel->size(), Qt::KeepAspectRatio));
        
        statusLabel->setText(QString("Faces detected: %1").arg(faces.size()));
    }
    
    void startCamera() {
        if (!cap.isOpened()) {
            setupCamera();
        }
        timer->start(30);
        startButton->setText("Stop Camera");
        startButton->setStyleSheet("QPushButton { background-color: #ff6b6b; color: white; }");
    }
    
    void stopCamera() {
        timer->stop();
        startButton->setText("Start Camera");
        startButton->setStyleSheet("QPushButton { background-color: #51cf66; color: white; }");
    }
    
    void toggleCamera() {
        if (timer->isActive()) {
            stopCamera();
        } else {
            startCamera();
        }
    }
    
    void updateScaleFactor(int value) {
        scaleFactor = 1.0 + value / 100.0;
        scaleLabel->setText(QString("Scale: %1").arg(scaleFactor, 0, 'f', 2));
    }
    
    void updateMinNeighbors(int value) {
        minNeighbors = value;
        neighborsLabel->setText(QString("Min Neighbors: %1").arg(minNeighbors));
    }

private:
    void setupUI() {
        QVBoxLayout *mainLayout = new QVBoxLayout(this);
        
        videoLabel = new QLabel();
        videoLabel->setMinimumSize(640, 480);
        videoLabel->setStyleSheet("QLabel { border: 2px solid #ddd; background-color: #f8f9fa; }");
        videoLabel->setAlignment(Qt::AlignCenter);
        videoLabel->setText("Camera Feed");
        
        QHBoxLayout *controlLayout = new QHBoxLayout();
        
        startButton = new QPushButton("Start Camera");
        startButton->setStyleSheet("QPushButton { background-color: #51cf66; color: white; padding: 10px; font-size: 14px; border-radius: 5px; }");
        startButton->setMinimumHeight(40);
        connect(startButton, &QPushButton::clicked, this, &FaceDetectionWidget::toggleCamera);
        
        QGroupBox *settingsGroup = new QGroupBox("Detection Settings");
        QVBoxLayout *settingsLayout = new QVBoxLayout(settingsGroup);
        
        QHBoxLayout *scaleLayout = new QHBoxLayout();
        QLabel *scaleTitle = new QLabel("Scale Factor:");
        QSlider *scaleSlider = new QSlider(Qt::Horizontal);
        scaleSlider->setRange(5, 50);
        scaleSlider->setValue(10);
        scaleLabel = new QLabel("Scale: 1.10");
        connect(scaleSlider, &QSlider::valueChanged, this, &FaceDetectionWidget::updateScaleFactor);
        
        scaleLayout->addWidget(scaleTitle);
        scaleLayout->addWidget(scaleSlider);
        scaleLayout->addWidget(scaleLabel);
        
        QHBoxLayout *neighborsLayout = new QHBoxLayout();
        QLabel *neighborsTitle = new QLabel("Min Neighbors:");
        QSlider *neighborsSlider = new QSlider(Qt::Horizontal);
        neighborsSlider->setRange(1, 10);
        neighborsSlider->setValue(3);
        neighborsLabel = new QLabel("Min Neighbors: 3");
        connect(neighborsSlider, &QSlider::valueChanged, this, &FaceDetectionWidget::updateMinNeighbors);
        
        neighborsLayout->addWidget(neighborsTitle);
        neighborsLayout->addWidget(neighborsSlider);
        neighborsLayout->addWidget(neighborsLabel);
        
        settingsLayout->addLayout(scaleLayout);
        settingsLayout->addLayout(neighborsLayout);
        
        controlLayout->addWidget(startButton);
        controlLayout->addWidget(settingsGroup);
        controlLayout->addStretch();
        
        statusLabel = new QLabel("Ready");
        statusLabel->setStyleSheet("QLabel { padding: 5px; background-color: #e9ecef; border-radius: 3px; }");
        
        mainLayout->addWidget(videoLabel);
        mainLayout->addLayout(controlLayout);
        mainLayout->addWidget(statusLabel);
        
        setLayout(mainLayout);
        setWindowTitle("Face Detection - OpenCV + Qt");
    }
    
    void setupCamera() {
        cap.open(0);
        if (!cap.isOpened()) {
            QMessageBox::critical(this, "Error", "Could not open camera!");
        }
    }
    
    void setupFaceDetection() {
        std::vector<std::string> classifierPaths = {
            "haarcascade_frontalface_alt.xml",
            "../haarcascade_frontalface_alt.xml",
            "../../haarcascade_frontalface_alt.xml"
        };
        
        bool classifierLoaded = false;
        for (const auto& path : classifierPaths) {
            if (faceCascade.load(path)) {
                classifierLoaded = true;
                break;
            }
        }
        
        if (!classifierLoaded) {
            QMessageBox::critical(this, "Error", "Could not load face cascade classifier!");
        }
    }

private:
    QLabel *videoLabel;
    QLabel *statusLabel;
    QPushButton *startButton;
    QLabel *scaleLabel;
    QLabel *neighborsLabel;
    QTimer *timer = new QTimer(this);
    
    cv::VideoCapture cap;
    cv::CascadeClassifier faceCascade;
    double scaleFactor = 1.1;
    int minNeighbors = 3;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    FaceDetectionWidget widget;
    widget.show();
    
    return app.exec();
}

#include "main_qt.moc"
