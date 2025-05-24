#include "incl/MainWindow.h"
<<<<<<< Updated upstream
#include <QVBoxLayout>
#include <QDebug>
=======
#include <QHBoxLayout>
#include <QDebug>
#include <QScreen>
#include <QStandardPaths>
#include <QDateTime>
>>>>>>> Stashed changes

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setupUi();
<<<<<<< Updated upstream
    
    // Initialize screen capture
=======
    loadSettings();

    // Initialize screen capture first
>>>>>>> Stashed changes
    if (!m_screenCapture.initialize()) {
        qDebug() << "Failed to initialize screen capture";
        return;
    }
<<<<<<< Updated upstream
    
    // Set up timer to update the display
    connect(&m_captureTimer, &QTimer::timeout, this, &MainWindow::updateScreenCapture);
    m_captureTimer.start(33); // ~30 FPS
=======

    // Initialize audio capture
    if (!m_audioCapture.initializeInput() ||
        !m_audioCapture.initializeOutput()) {
        qDebug() << "Failed to initialize audio capture";
        return;
    }

    // Create recording manager with reference to our screen capture
    m_recordingManager = std::make_unique<RecordingManager>(&m_screenCapture, &m_audioCapture);


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

    // Setup connections for recording state changes
    connect(m_recordingManager.get(), &RecordingManager::recordingStateChanged, this, &MainWindow::onRecordingStateChanged);
    connect(m_recordingManager.get(), &RecordingManager::recordingError, this, &MainWindow::onRecordingError);
>>>>>>> Stashed changes
}

MainWindow::~MainWindow()
{
    m_captureTimer.stop();
<<<<<<< Updated upstream
=======
    m_volumeTimer.stop();
    m_audioCapture.stopCapture();

    // Make sure recording is stopped before destroying
    if (m_recordingManager->isRecording()) {
        m_recordingManager->stopRecording();
    }

    saveSettings();
>>>>>>> Stashed changes
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
<<<<<<< Updated upstream
    m_displayLabel->setScaledContents(false); // We'll handle scaling manually
    
    layout->addWidget(m_displayLabel);
=======
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

    // Add widgets to layout
    mainLayout->addWidget(m_displayLabel);
    mainLayout->addWidget(m_recordButton);
    mainLayout->addLayout(statsLayout);

>>>>>>> Stashed changes
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

void MainWindow::updateScreenCapture()
{
    // Time the capture operation
    qint64 startTime = QDateTime::currentMSecsSinceEpoch();

    if (m_screenCapture.captureFrame()) {
        QImage frame = m_screenCapture.getLatestFrame();
        
        // Scale the frame to fit the display area while maintaining aspect ratio
        QImage scaledFrame = frame.scaled(m_displayLabel->size(), 
                                         Qt::KeepAspectRatio, 
                                         Qt::SmoothTransformation);
        
        // Display the scaled frame
        m_displayLabel->setPixmap(QPixmap::fromImage(scaledFrame));
    }
<<<<<<< Updated upstream
=======
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

        // Start recording
        if (!m_recordingManager->startRecording(filePath.toStdString())) {
            QMessageBox::critical(this, "Error",
                "Failed to start recording. Check console for details.");
        }
    }
    else {
        // Stop recording
        m_recordingManager->stopRecording();
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
>>>>>>> Stashed changes
}