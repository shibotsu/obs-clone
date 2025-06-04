#ifndef RECORDINGMANAGER_H
#define RECORDINGMANAGER_H

#include <QObject>
#include <QWaitCondition>
#include <QMutex>
#include <string>
#include <memory>
#include <thread>
#include <queue>
#include <chrono>
#include <atomic>

class ScreenCapture;
class AudioCapture;
class FFmpegRecorder;
#include "PTR_INFO.h"

struct VideoFrameData {
    std::vector<uchar> data;
    int width;
    int height;
    int bytesPerLine;
    PTR_INFO mousePtrInfo;
    int64_t timestamp; 
};

struct AudioData {
    std::vector<uchar> data;
    int size;
    int64_t timestamp;
};

class RecordingManager : public QObject
{
    Q_OBJECT
public:
    explicit RecordingManager(QObject* parent = nullptr);
    ~RecordingManager();

    bool startRecording(const std::string& outputFile, int videoWidth, int videoHeight, int videoFps,
                        int audioSampleRate, int audioChannels);
    void stopRecording();
    bool isRecording() const { return m_isRecording; }

signals:
    void recordingStateChanged(bool isRecording);
    void recordingError(const QString& errorMessage);

public slots:
    void onNewVideoFrame(const uchar* data, int width, int height, int bytesPerLine, const PTR_INFO& mousePtrInfo);
    void onNewAudioData(const uchar* data, int size);

private:
    void recordingLoop(); // Worker thread function

    std::atomic<bool> m_isRecording;
    std::unique_ptr<FFmpegRecorder> m_recorder;
    std::thread m_recordingThread;

    // Use queues to buffer frames/audio data that arrive from signals
    // for the recording thread to process.
    std::queue<VideoFrameData> m_videoQueue;
    std::queue<AudioData> m_audioQueue;
    QMutex m_queueMutex; // Protect access to m_videoQueue and m_audioQueue
    QWaitCondition m_dataAvailable; // Signal when data is added to queues

    // Store parameters needed to initialize FFmpegRecorder
    int m_videoWidth = 0;
    int m_videoHeight = 0;
    int m_videoFps = 0;
    int m_audioSampleRate = 0;
    int m_audioChannels = 0;
};
#endif // RECORDINGMANAGER_H