#include "incl/MainWindow.h"
#include <QHBoxLayout>
#include <QDebug>
#include <QScreen>
#include <QStandardPaths>
#include <QDateTime>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setupUi();
    loadSettings();

    // Initialize screen capture first
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

    m_recordingManager = std::make_unique<RecordingManager>();
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

    // Connect the ScreenCapture's signal to MainWindow's slot for preview
    // Now m_captureTimer will trigger acquireAndProcessFrame, which in turn emits newFrameReady
    // This newFrameReady signal is connected to onNewVideoFrameReady for UI update.
    connect(&m_captureTimer, &QTimer::timeout, &m_screenCapture, &ScreenCapture::captureFrame);
    connect(&m_screenCapture, &ScreenCapture::newFrameReady, this, &MainWindow::onNewVideoFrameReady);
    m_captureTimer.start(captureInterval);

    connect(&m_volumeTimer, &QTimer::timeout, this, &MainWindow::updateAudioVolume);
    m_volumeTimer.start(10);

    // Enable Qt's high DPI scaling
    setWindowFlag(Qt::Window);

    // Setup connections for recording state changes
    connect(m_recordingManager.get(), &RecordingManager::recordingStateChanged, this, &MainWindow::onRecordingStateChanged);
    connect(m_recordingManager.get(), &RecordingManager::recordingError, this, &MainWindow::onRecordingError);
}

MainWindow::~MainWindow()
{
    m_captureTimer.stop();
    m_volumeTimer.stop();
    m_audioCapture.stopCapture();

    // Make sure recording is stopped before destroying
    if (m_recordingManager->isRecording()) {
        m_recordingManager->stopRecording();
    }

    saveSettings();
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

    // Recording button
    m_recordButton = new QPushButton("Start Recording", this);
    m_recordButton->setCheckable(false);
    connect(m_recordButton, &QPushButton::clicked, this, &MainWindow::toggleRecording);

    // Streaming button
    m_streamButton = new QPushButton("Start Streaming", this); // Initialize the new button
    m_streamButton->setCheckable(false);
    connect(m_streamButton, &QPushButton::clicked, this, &MainWindow::toggleStreaming); // Connect to the new slot

    // Add widgets to layout
    mainLayout->addWidget(m_displayLabel);
    mainLayout->addWidget(m_recordButton);
    mainLayout->addWidget(m_streamButton);
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
    QVBoxLayout* inputLayout = new QVBoxLayout();
    m_inputMeter = new VolumeMeter(this, "Mic/Aux");
    m_inputDbLabel = new QLabel("-60 dB", this); // initial value
    m_inputDbLabel->setAlignment(Qt::AlignCenter);
    inputLayout->addWidget(m_inputMeter);
    inputLayout->addWidget(m_inputDbLabel);

    QVBoxLayout* outputLayout = new QVBoxLayout();
    m_outputMeter = new VolumeMeter(this, "Desktop Audio");
    m_outputDbLabel = new QLabel("-60 dB", this);
    m_outputDbLabel->setAlignment(Qt::AlignCenter);
    outputLayout->addWidget(m_outputMeter);
    outputLayout->addWidget(m_outputDbLabel);

    QHBoxLayout* volumeMeterLayout = new QHBoxLayout();
    volumeMeterLayout->addLayout(inputLayout);
    volumeMeterLayout->addLayout(outputLayout);

    // Add meters with dB labels
    mainLayout->addLayout(volumeMeterLayout);
}

void MainWindow::onNewVideoFrameReady(const uchar* data, int width, int height, int bytesPerLine, const PTR_INFO& mousePtrInfo)
{
    // This replaces the old updateScreenCapture() content that fetched m_latestFrame
    // Now we get the raw data directly from the signal.

    qint64 startTime = QDateTime::currentMSecsSinceEpoch();

    // Create a QImage from the received raw data.
    // This QImage does NOT take ownership of 'data'.
    QImage frame(data, width, height, bytesPerLine, QImage::Format_ARGB32);

    // Only scale if necessary (avoid unnecessary operations)
    QSize targetSize = m_displayLabel->size();
    if (frame.size() != targetSize) {
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

    m_frameCount++;
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    qint64 frameDuration = now - startTime;
    m_latencyLabel->setText(QString("Update Latency: %1 ms").arg(frameDuration));
}

void MainWindow::updateAudioVolume()
{
    float inputLevel = m_audioCapture.getInputVolume();
    float outputLevel = m_audioCapture.getOutputVolume();

    if (m_inputMeter) {
        m_inputMeter->setLevel(inputLevel);
        if (m_inputDbLabel)
            m_inputDbLabel->setText(QString::number(inputLevel, 'f', 1) + " dB");
    }

    if (m_outputMeter) {
        m_outputMeter->setLevel(outputLevel);
        if (m_outputDbLabel)
            m_outputDbLabel->setText(QString::number(outputLevel, 'f', 1) + " dB");
    }
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

void MainWindow::toggleRecording()
{
    if (!m_recordingManager->isRecording()) {
        // Get output file
        QString defaultPath = m_lastSavePath;
        if (defaultPath.isEmpty()) {
            defaultPath = QStandardPaths::writableLocation(QStandardPaths::MoviesLocation);
        }

        QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss");
        QString defaultFilename = defaultPath + "/recording_" + timestamp + ".mp4";

        QString filePath = QFileDialog::getSaveFileName(this,
            "Save Recording As",
            defaultFilename,
            "Video Files (*.mp4)");

        if (filePath.isEmpty()) {
            return; // User cancelled
        }

        m_lastSavePath = QFileInfo(filePath).path();

        // Get video and audio parameters
        int videoWidth = m_screenCapture.getWidth();
        int videoHeight = m_screenCapture.getHeight();
        //int videoFps = QGuiApplication::primaryScreen()->refreshRate(); // Use primary screen's refresh rate
        int videoFps = 60;

        int audioSampleRate = m_audioCapture.getSampleRate();
        int audioChannels = m_audioCapture.getChannels();

        // Initialize and start the recording manager
        if (!m_recordingManager->startRecording(
            filePath.toStdString(),
            videoWidth,
            videoHeight,
            videoFps,
            audioSampleRate,
            audioChannels)) {
            QMessageBox::critical(this, "Error", "Failed to start recording. Check console for details.");
            return; // Exit if recording failed to start
        }

        // Connect the ScreenCapture's newFrameReady signal to RecordingManager's slot
        // This is crucial: the RecordingManager now *receives* the frame, doesn't *pull* it.
        connect(&m_screenCapture, &ScreenCapture::newFrameReady,
            m_recordingManager.get(), &RecordingManager::onNewVideoFrame);

        // Connect AudioCapture's signal to RecordingManager's slot
        connect(&m_audioCapture, &AudioCapture::newAudioDataReady,
            m_recordingManager.get(), &RecordingManager::onNewAudioData);

        qDebug() << "Recording started, saving to:" << filePath;
    }
    else {
        // Disconnect signals when stopping recording
        disconnect(&m_screenCapture, &ScreenCapture::newFrameReady,
            m_recordingManager.get(), &RecordingManager::onNewVideoFrame);
        disconnect(&m_audioCapture, &AudioCapture::newAudioDataReady,
            m_recordingManager.get(), &RecordingManager::onNewAudioData);

        m_recordingManager->stopRecording();
        qDebug() << "Recording stopped.";
    }
}

void MainWindow::toggleStreaming()
{
    // Placeholder for streaming logic.
    // You would typically have a separate StreamingManager or
    // extend RecordingManager to handle streaming instead of recording to a file.
    if (m_streamButton->text() == "Start Streaming") {
        qDebug() << "Starting streaming...";
        // Implement your streaming initialization here
        // For example:
        // if (m_streamingManager->startStreaming(streamUrl, videoWidth, videoHeight, videoFps, audioSampleRate, audioChannels)) {
        //     connect(&m_screenCapture, &ScreenCapture::newFrameReady, m_streamingManager.get(), &StreamingManager::onNewVideoFrame);
        //     connect(&m_audioCapture, &AudioCapture::newAudioDataReady, m_streamingManager.get(), &StreamingManager::onNewAudioData);
        //     m_streamButton->setText("Stop Streaming");
        // } else {
        //     QMessageBox::critical(this, "Error", "Failed to start streaming.");
        // }
        m_streamButton->setText("Stop Streaming"); // For demonstration
    }
    else {
        qDebug() << "Stopping streaming...";
        // Implement your streaming stopping logic here
        // For example:
        // disconnect(&m_screenCapture, &ScreenCapture::newFrameReady, m_streamingManager.get(), &StreamingManager::onNewVideoFrame);
        // disconnect(&m_audioCapture, &AudioCapture::newAudioDataReady, m_streamingManager.get(), &StreamingManager::onNewAudioData);
        // m_streamingManager->stopStreaming();
        m_streamButton->setText("Start Streaming");
    }
}

void MainWindow::onRecordingStateChanged(bool isRecording)
{
    if (isRecording) {
        m_recordButton->setText("Stop Recording");
    }
    else {
        m_recordButton->setText("Start Recording");
    }
}

void MainWindow::onRecordingError(const QString& errorMessage)
{
    QMessageBox::critical(this, "Recording Error", errorMessage);

    // Ensure button state is correct if recording failed
    if (m_recordingManager->isRecording()) {
        m_recordingManager->stopRecording();
    }
}

void MainWindow::saveSettings()
{
    QSettings settings("YourCompany", "ScreenRecorder");
    settings.setValue("lastSavePath", m_lastSavePath);
}

void MainWindow::loadSettings()
{
    QSettings settings("YourCompany", "ScreenRecorder");
    m_lastSavePath = settings.value("lastSavePath").toString();
}