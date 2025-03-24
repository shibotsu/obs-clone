#pragma once

#include <QMainWindow>
#include <QTimer>
#include <QLabel>
#include "ScreenCapture.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void updateScreenCapture();

private:
    void setupUi();

    ScreenCapture m_screenCapture;
    QTimer m_captureTimer;
    QLabel* m_displayLabel;
    int m_displayWidth;
    int m_displayHeight;
};