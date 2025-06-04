#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ScreenCapture.h"
#include "AudioCapture.h"
#include "VolumeMeter.h"
#include "RecordingManager.h"

#include <QMainWindow>
#include <QPushButton.h>
#include <QVBoxLayout>
#include <QFileDialog.h>
#include <QSettings.h>
#include <QMessageBox>
#include <QTimer>
#include <QLabel>

#include <QProgressBar>
#include <QDateTime>

#include <memory>


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    // This slot will receive the captured frame data from ScreenCapture
    void onNewVideoFrameReady(const uchar* data, int width, int height, int bytesPerLine, const PTR_INFO& mousePtrInfo);

    void updateAudioVolume();
    void updateFPS();
    void toggleRecording();
    void toggleStreaming();
    void onRecordingStateChanged(bool isRecording);
    void onRecordingError(const QString& errorMessage);

private:
    void setupUi();
    void saveSettings();
    void loadSettings();

    ScreenCapture m_screenCapture;
    AudioCapture m_audioCapture;
    std::unique_ptr<RecordingManager> m_recordingManager;

    QTimer m_captureTimer;
    QTimer m_volumeTimer;
    QTimer m_fpsUpdateTimer;

    QLabel* m_previewLabel;
    QTimer* m_previewTimer;

    QLabel* m_displayLabel;
    QLabel* m_fpsLabel;
    QLabel* m_latencyLabel;
    QPushButton* m_recordButton;
    QPushButton* m_streamButton;
    VolumeMeter* m_inputMeter = nullptr;   // Mic/Aux
    VolumeMeter* m_outputMeter = nullptr;  // Desktop Audio
    QLabel* m_inputDbLabel = nullptr;
    QLabel* m_outputDbLabel = nullptr;

    qint64 m_lastFrameTime;
    int m_frameCount;
    QString m_lastSavePath;
    int m_displayWidth = 0;
    int m_displayHeight = 0;

    QProgressBar* m_volumeBar;
    QProgressBar* m_desktopVolumeBar;
    QProgressBar* m_micVolumeBar;

    float m_smoothedVolume = -60.0f; // Initialize to minimum value
    QString m_currentBarColor = "#4CAF50"; // Start with green
};

#endif
