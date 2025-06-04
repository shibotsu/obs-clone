#define NOMINMAX

#include <QDebug.h>
#include <chrono>
#include <thread>
#include <algorithm>

#include "incl/RecordingManager.h"
#include "incl/FFmpegRecorder.h" // Include FFmpegRecorder.h here
#include "incl/ScreenCapture.h" // Needed for PTR_INFO
#include "incl/AudioCapture.h" // Needed for AudioCapture (for processAudioForRecording)

#include <QMutexLocker>
#include <QDebug>
#include <QCoreApplication>

RecordingManager::RecordingManager(QObject* parent)
    : QObject(parent), m_isRecording(false)
{
    // Constructor no longer takes ScreenCapture* and AudioCapture*
    // Connections will be made externally in MainWindow.
}

RecordingManager::~RecordingManager()
{
    stopRecording();
}

bool RecordingManager::startRecording(const std::string& outputFile, int videoWidth, int videoHeight, int videoFps,
    int audioSampleRate, int audioChannels)
{
    if (m_isRecording) {
        emit recordingError("Already recording");
        return false;
    }

    m_videoWidth = videoWidth;
    m_videoHeight = videoHeight;
    m_videoFps = videoFps;
    m_audioSampleRate = audioSampleRate;
    m_audioChannels = audioChannels;

    // FFmpegRecorder is now created and initialized here, *inside* the RecordingManager,
    // and *before* the recording thread starts.
    m_recorder = std::make_unique<FFmpegRecorder>();
    if (!m_recorder->initialize(outputFile.c_str(), m_videoWidth, m_videoHeight, m_videoFps,
        m_audioSampleRate, m_audioChannels)) {
        emit recordingError("Failed to initialize recorder");
        m_recorder.reset(); // Clean up if initialization fails
        return false;
    }

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

    m_isRecording = false; // Signal the thread to stop
    m_dataAvailable.wakeAll(); // Wake up the thread if it's waiting

    if (m_recordingThread.joinable()) {
        m_recordingThread.join(); // Wait for the thread to finish
    }

    // Clean up recorder
    if (m_recorder) {
        m_recorder->close();
        m_recorder.reset();
    }

    // Clear any remaining buffered data
    QMutexLocker locker(&m_queueMutex);
    std::queue<VideoFrameData> emptyVideo;
    std::swap(m_videoQueue, emptyVideo);
    std::queue<AudioData> emptyAudio;
    std::swap(m_audioQueue, emptyAudio);

    emit recordingStateChanged(false);
}

// Slot to receive video frames from ScreenCapture
void RecordingManager::onNewVideoFrame(const uchar* data, int width, int height, int bytesPerLine, const PTR_INFO& mousePtrInfo)
{
    // Create a copy of the video frame data to put in the queue.
    // The `data` pointer is only valid for the duration of this slot call.
    VideoFrameData frameData;
    //frameData.data.assign(data, data + (height * bytesPerLine)); // Copy the raw bytes
    frameData.data.assign(data, data + (static_cast<size_t>(height) * bytesPerLine)); // Use static_cast for size_t
    
    frameData.width = width;
    frameData.height = height;
    frameData.bytesPerLine = bytesPerLine;
    
    frameData.mousePtrInfo = mousePtrInfo; // Copy mouse info
    frameData.timestamp = m_recorder->getRecordingTimestamp(); // Get current wall-clock time for synchronization

    QMutexLocker locker(&m_queueMutex);
    m_videoQueue.push(std::move(frameData)); // Move to avoid extra copy
    m_dataAvailable.wakeOne(); // Notify the recording thread that new data is available
}

// Slot to receive audio data from AudioCapture
void RecordingManager::onNewAudioData(const uchar* data, int size)
{
    // Create a copy of the audio data
    AudioData audioData;
    audioData.data.assign(data, data + size); // Copy the raw bytes
    audioData.size = size;

    // IMPORTANT: Free the memory received from AudioCapture immediately after copying
	delete[] data;

    audioData.timestamp = m_recorder->getRecordingTimestamp(); // Get current wall-clock time for synchronization

    QMutexLocker locker(&m_queueMutex);
    m_audioQueue.push(std::move(audioData));
    m_dataAvailable.wakeOne(); // Notify the recording thread
}

void RecordingManager::recordingLoop()
{
    // Video timing
    const std::chrono::microseconds videoFrameInterval(1000000 / m_videoFps);
    auto nextExpectedVideoFrameTime = std::chrono::high_resolution_clock::now();

    // Audio timing (no fixed interval, process as available, but ensure it doesn't fall behind)
    // We don't need a `nextExpectedAudioFrameTime` as audio is processed as it arrives
    // and its PTS is adjusted by FFmpegRecorder based on wall-clock.

    try {
        while (m_isRecording) {
            auto now = std::chrono::high_resolution_clock::now();
            bool processed_something = false; // Flag to avoid busy-waiting if queues are empty

            // --- Process Video Frames ---
            // Only process if it's time for a new video frame AND there's data in the queue
            if (now >= nextExpectedVideoFrameTime && !m_videoQueue.empty()) {
                QMutexLocker locker(&m_queueMutex); // Lock for queue access
                if (!m_videoQueue.empty()) { // Double check after locking
                    VideoFrameData frameData = m_videoQueue.front();
                    m_videoQueue.pop();
                    locker.unlock(); // Release mutex while encoding (can be long)

                    // Send the copied data to FFmpegRecorder
                    if (!m_recorder->sendVideoFrame(frameData.data.data(), frameData.timestamp)) {
                        qDebug() << "RecordingManager: Failed to send video frame to FFmpegRecorder";
                        // Consider emitting recordingError signal here if this is critical
                    }
                    processed_something = true;
                    nextExpectedVideoFrameTime += videoFrameInterval; // Advance expected time

                    // Adjust nextExpectedVideoFrameTime if we fall behind significantly
                    // This helps prevent accumulating lag.
                    // If we are more than one frame interval behind, jump to now + one interval
                    // to try and catch up without processing too many frames at once.
                    if (nextExpectedVideoFrameTime < now) {
                        nextExpectedVideoFrameTime = now + videoFrameInterval;
                    }
                }
            }

            // --- Process Audio Frames ---
            // Process all available audio frames in the queue
            while (!m_audioQueue.empty()) {
                QMutexLocker locker(&m_queueMutex); // Lock for queue access
                if (!m_audioQueue.empty()) { // Double check after locking
                    AudioData audioData = m_audioQueue.front();
                    m_audioQueue.pop();
                    locker.unlock(); // Release mutex while encoding

                    // Send the copied data to FFmpegRecorder
                    if (!m_recorder->sendAudioFrame(audioData.data.data(), audioData.size, audioData.timestamp)) {
                        qDebug() << "RecordingManager: Failed to send audio frame to FFmpegRecorder";
                        // Consider emitting recordingError signal here
                    }
                    processed_something = true;
                }
                else {
                    break; // Queue became empty while processing
                }
            }

            // --- Sleep/Wait Logic ---
            if (!processed_something) {
                // If nothing was processed in this iteration, wait for new data or timeout
                QMutexLocker locker(&m_queueMutex); // Reacquire mutex for wait condition
                if (m_videoQueue.empty() && m_audioQueue.empty()) {
                    // Calculate the shortest time until the next video frame is due
                    auto timeUntilNextVideo = nextExpectedVideoFrameTime - std::chrono::high_resolution_clock::now();
                    long long wait_ms = std::max(1LL, std::chrono::duration_cast<std::chrono::milliseconds>(timeUntilNextVideo).count());

                    // Wait for data or for the next video frame time, with a max timeout
                    // The 100ms timeout is good for periodically checking m_isRecording
                    m_dataAvailable.wait(locker.mutex(), std::min(wait_ms, 100LL));
                }
            }
            // If something was processed, we might loop immediately to check for more data
            // or let the scheduler decide. A very small sleep (0 or 1ms) can be beneficial
            // to yield the CPU if there's nothing else to do *right now*.
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
    catch (const std::exception& e) {
        m_isRecording = false;
        emit recordingError(QString("Recording error: %1").arg(e.what()));
    }
    qDebug() << "Recording loop finished.";
}