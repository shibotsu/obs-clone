#include "incl/MainWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>
#include <QScreen>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setupUi();

    // Initialize screen capture
    if (!m_screenCapture.initialize()) {
        qDebug() << "Failed to initialize screen capture";
        return;
    }

    // Initialize audio capture
    if (!m_audioCapture.initializeInput() ||
        !m_audioCapture.initializeOutput()) {
        qDebug() << "Failed to initialize audio capture";
        return;
    }

    // Start audio capture
    m_audioCapture.startCapture();

    // Get the refresh rate of the primary screen
    QScreen* primaryScreen = QGuiApplication::primaryScreen();
    int refreshRate = primaryScreen->refreshRate();

    // Default to 60 if we couldn't determine the refresh rate
    if (refreshRate < 30) {
        refreshRate = 60;
        qDebug() << "Could not determine screen refresh rate, defaulting to 60Hz";
    }
    else {
        qDebug() << "Screen refresh rate:" << refreshRate << "Hz";
    }

    // Calculate timer interval in milliseconds (1000 ms / refresh rate)
    int captureInterval = 1000 / refreshRate;

    // Set up timers with refresh rate matching
    connect(&m_captureTimer, &QTimer::timeout, this, &MainWindow::updateScreenCapture);
    m_captureTimer.start(captureInterval);


    // Make audio updates more frequent than screen updates for responsiveness
    // Using a much shorter interval for audio capturing
    connect(&m_volumeTimer, &QTimer::timeout, this, &MainWindow::updateAudioVolume);
    m_volumeTimer.start(10); // 10ms intervals = ~100 updates per second

    // Enable Qt's high DPI scaling
    setWindowFlag(Qt::Window);
}

MainWindow::~MainWindow()
{
    m_captureTimer.stop();
    m_volumeTimer.stop();
    m_audioCapture.stopCapture();
}

void MainWindow::setupUi()
{
    setWindowTitle("obs");
    resize(800, 600);

    // Create central widget and main layout
    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

    // Create display label
    m_displayLabel = new QLabel(this);
    m_displayLabel->setMinimumSize(640, 480);
    m_displayLabel->setAlignment(Qt::AlignCenter);
    m_displayLabel->setScaledContents(false);

    // Add FPS and latency indicators
    QHBoxLayout* statsLayout = new QHBoxLayout();
    m_fpsLabel = new QLabel("FPS: --", this);
    m_latencyLabel = new QLabel("Update Latency: -- ms", this);
    statsLayout->addWidget(m_fpsLabel);
    statsLayout->addWidget(m_latencyLabel);
    statsLayout->addStretch();

    // Add widgets to layout
    mainLayout->addWidget(m_displayLabel);
    mainLayout->addLayout(statsLayout);

    setCentralWidget(centralWidget);

    // Store display dimensions
    m_displayWidth = m_displayLabel->width();
    m_displayHeight = m_displayLabel->height();

    // Initialize frame timing variables
    m_lastFrameTime = QDateTime::currentMSecsSinceEpoch();
    m_frameCount = 0;
    m_fpsUpdateTimer.setInterval(1000);
    connect(&m_fpsUpdateTimer, &QTimer::timeout, this, &MainWindow::updateFPS);
    m_fpsUpdateTimer.start();

    // Custom volume meter layout
    QHBoxLayout* volumeMeterLayout = new QHBoxLayout();
    m_inputMeter = new VolumeMeter(this, "Mic/Aux");
    m_outputMeter = new VolumeMeter(this, "Desktop Audio");

    volumeMeterLayout->addWidget(m_inputMeter);
    volumeMeterLayout->addWidget(m_outputMeter);

    // Add meters below latency/FPS
    mainLayout->addLayout(volumeMeterLayout);
}

void MainWindow::updateScreenCapture()
{
    // Time the capture operation
    qint64 startTime = QDateTime::currentMSecsSinceEpoch();

    if (m_screenCapture.captureFrame()) {
        QImage frame = m_screenCapture.getLatestFrame();

        // Only scale if necessary (avoid unnecessary operations)
        QSize targetSize = m_displayLabel->size();
        if (frame.size() != targetSize) {
            // Use fast transformation for real-time display
            QImage scaledFrame = frame.scaled(
                targetSize,
                Qt::KeepAspectRatio,
                Qt::FastTransformation
            );
            m_displayLabel->setPixmap(QPixmap::fromImage(scaledFrame));
        }
        else {
            m_displayLabel->setPixmap(QPixmap::fromImage(frame));
        }

        // Track frame timing for FPS calculation
        m_frameCount++;
        qint64 now = QDateTime::currentMSecsSinceEpoch();
        qint64 frameDuration = now - startTime;

        // Update latency display (for screen capture operation)
        m_latencyLabel->setText(QString("Update Latency: %1 ms").arg(frameDuration));
    }
}

void MainWindow::updateAudioVolume()
{
    float inputLevel = m_audioCapture.getInputVolume();
    float outputLevel = m_audioCapture.getOutputVolume();

    if (m_inputMeter)
        m_inputMeter->setLevel(inputLevel);

    if (m_outputMeter)
        m_outputMeter->setLevel(outputLevel);
}

void MainWindow::updateFPS()
{
    // Calculate and display current FPS
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    qint64 elapsed = now - m_lastFrameTime;

    if (elapsed > 0) {
        double fps = (m_frameCount * 1000.0) / elapsed;
        m_fpsLabel->setText(QString("FPS: %1").arg(fps, 0, 'f', 1));
    }

    m_frameCount = 0;
    m_lastFrameTime = now;
}