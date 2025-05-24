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


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void updateScreenCapture();
    void updateAudioVolume();
    void updateFPS();
    void toggleRecording();
    void onRecordingStateChanged(bool isRecording);
    void onRecordingError(const QString& errorMessage);

private:
    void setupUi();
    void saveSettings();
    void loadSettings();

    QPushButton* m_recordButton;
    QLabel* m_previewLabel;
    QTimer* m_previewTimer;

    std::unique_ptr<RecordingManager> m_recordingManager;
    QString m_lastSavePath;

    ScreenCapture m_screenCapture;
    QTimer m_captureTimer;
    QLabel* m_displayLabel;
    int m_displayWidth;
    int m_displayHeight;

    // Audio capture related
    AudioCapture m_audioCapture;
    QTimer m_volumeTimer;
    QProgressBar* m_volumeBar;
    QProgressBar* m_desktopVolumeBar;
    QProgressBar* m_micVolumeBar;

    VolumeMeter* m_inputMeter = nullptr;   // Mic/Aux
    VolumeMeter* m_outputMeter = nullptr;  // Desktop Audio

    QLabel* m_inputDbLabel = nullptr;
    QLabel* m_outputDbLabel = nullptr;

    // FPS and metrics tracking
    QTimer m_fpsUpdateTimer;
    qint64 m_lastFrameTime;
    int m_frameCount;
    QLabel* m_fpsLabel;
    QLabel* m_latencyLabel;

    float m_smoothedVolume = -60.0f; // Initialize to minimum value
    QString m_currentBarColor = "#4CAF50"; // Start with green
};

#endif
