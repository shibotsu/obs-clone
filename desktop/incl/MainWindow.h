#pragma once
#include <QMainWindow>
#include <QTimer>
#include <QLabel>
#include <QProgressBar>
#include <QDateTime>
#include "ScreenCapture.h"
#include "AudioCapture.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void updateScreenCapture();
    void updateAudioVolume();
    void updateFPS();

private:
    void setupUi();

    // Screen capture related
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

    // FPS and metrics tracking
    QTimer m_fpsUpdateTimer;
    qint64 m_lastFrameTime;
    int m_frameCount;
    QLabel* m_fpsLabel;
    QLabel* m_latencyLabel;

    float m_smoothedVolume = -60.0f; // Initialize to minimum value
    QString m_currentBarColor = "#4CAF50"; // Start with green
};