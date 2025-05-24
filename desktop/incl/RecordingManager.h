#ifndef RECORDINGMANAGER_H
#define RECORDINGMANAGER_H
#include <QObject>
#include <QImage>
#include <QMutex>
#include <atomic>
#include <thread>
#include <memory>
#include "ScreenCapture.h"
#include "AudioCapture.h"
#include "FFmpegRecorder.h"

class RecordingManager : public QObject
{
    Q_OBJECT
public:
    // Single constructor that takes pointers to existing objects
    explicit RecordingManager(ScreenCapture* screenCapture = nullptr,
                              AudioCapture* audioCapture = nullptr,
                              QObject* parent = nullptr);
    ~RecordingManager();

    bool startRecording(const std::string& outputFile);
    void stopRecording();
    QImage getPreviewFrame() const;
    bool isRecording() const { return m_isRecording; }

    // Add method to process new frames if needed
    //void processNewFrame();

signals:
    void recordingStateChanged(bool isRecording);
    void recordingError(const QString& errorMessage);

private:
    void recordingLoop();

    // Change to raw pointers - these are references, not owned objects
    ScreenCapture* m_screenCapture;
    AudioCapture* m_audioCapture;

    // This you still own and should be a unique_ptr
    std::unique_ptr<FFmpegRecorder> m_recorder;

    std::atomic<bool> m_isRecording;
    std::thread m_recordingThread;
    mutable QMutex m_frameMutex;
};
#endif // RECORDINGMANAGER_H