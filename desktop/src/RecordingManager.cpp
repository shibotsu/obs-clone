#include "incl/RecordingManager.h"
#include <QMutexLocker>
#include <qdebug.h>
#include <chrono>
#include <thread>
#include <algorithm>

RecordingManager::RecordingManager(ScreenCapture* screenCapture,
    AudioCapture* audioCapture,
    QObject* parent)
    : QObject(parent)
    , m_screenCapture(screenCapture)
    , m_audioCapture(audioCapture)
    , m_isRecording(false)
{
    if (!m_screenCapture) {
        emit recordingError("No screen capture provided");
    }
    if (!m_audioCapture) {
        emit recordingError("No audio capture provided");
    }
}

RecordingManager::~RecordingManager()
{
    stopRecording();
}

bool RecordingManager::startRecording(const std::string& outputFile)
{
    if (m_isRecording) {
        emit recordingError("Already recording");
        return false;
    }

    if (!m_screenCapture || !m_audioCapture) {
        emit recordingError("Screen or audio capture not initialized");
        return false;
    }

    // Create the recorder
    int width = m_screenCapture->getWidth();
    int height = m_screenCapture->getHeight();

    m_recorder = std::make_unique<FFmpegRecorder>();
    if (!m_recorder->initialize(outputFile.c_str(), width, height, 60, 44100, 2)) {
        emit recordingError("Failed to initialize recorder");
        return false;
    }

    // Start the recording thread
    m_isRecording = true;
    m_recordingThread = std::thread(&RecordingManager::recordingLoop, this);

    emit recordingStateChanged(true);
    return true;
}

void RecordingManager::stopRecording()
{
    if (!m_isRecording) {
        return;
    }

    m_isRecording = false;

    // Wait for recording thread to finish
    if (m_recordingThread.joinable()) {
        m_recordingThread.join();
    }

    // Clean up recorder
    if (m_recorder) {
        m_recorder->close();
        m_recorder.reset();
    }

    emit recordingStateChanged(false);
}

//QImage RecordingManager::getPreviewFrame() const
//{
//    QMutexLocker locker(&m_frameMutex);
//    return m_screenCapture->getLatestFrame();
//}

void RecordingManager::recordingLoop()
{
    // Video timing - 60 FPS
    const int targetVideoFPS = 60;
    const std::chrono::microseconds videoFrameTime(1000000 / targetVideoFPS);
    auto nextVideoFrameTime = std::chrono::high_resolution_clock::now();

    const int targetAudioFPS = 100;
    const std::chrono::microseconds audioFrameTime(1000000 / targetAudioFPS);
    auto nextAudioFrameTime = std::chrono::high_resolution_clock::now();

    try {
        while (m_isRecording) {
            auto now = std::chrono::high_resolution_clock::now();

            // Process video at 60 FPS
            if (now >= nextVideoFrameTime) {
                if (!m_screenCapture->captureFrameAndRecord(m_recorder.get())) {
                    qDebug() << "Failed to capture video frame";
                }
                nextVideoFrameTime += videoFrameTime;
            }

            // Process audio at separate, controlled rate
            if (now >= nextAudioFrameTime) {
                if (!m_audioCapture->processAudioForRecording(m_recorder.get())) {
                    qDebug() << "Failed to process audio";
                }
                nextAudioFrameTime += audioFrameTime;
            }

            // Sleep for a short time to prevent busy waiting
            // Sleep until the next event (either video or audio)
            // 
			// Sometimes std::min is ambiguous because Qt has its own min/max.
            // To avoid conflict, call is fully qualified
            auto nextEvent = std::min<std::chrono::high_resolution_clock::time_point>(nextVideoFrameTime, nextAudioFrameTime);
            if (nextEvent > now) {
                std::this_thread::sleep_until(nextEvent);
            }
        }
    }
    catch (const std::exception& e) {
        m_isRecording = false;
        emit recordingError(QString("Recording error: %1").arg(e.what()));
    }
}