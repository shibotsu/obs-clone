#include "incl/MainWindow.h"
#include <QVBoxLayout>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUi();
    
    // Initialize screen capture
    if (!m_screenCapture.initialize()) {
        qDebug() << "Failed to initialize screen capture";
        return;
    }
    
    // Set up timer to update the display
    connect(&m_captureTimer, &QTimer::timeout, this, &MainWindow::updateScreenCapture);
    m_captureTimer.start(33); // ~30 FPS
}

MainWindow::~MainWindow()
{
    m_captureTimer.stop();
}

void MainWindow::setupUi()
{
    setWindowTitle("Screen Capture Preview");
    resize(800, 600);
    
    // Create central widget and layout
    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(centralWidget);
    
    // Create display label
    m_displayLabel = new QLabel(this);
    m_displayLabel->setMinimumSize(640, 480);
    m_displayLabel->setAlignment(Qt::AlignCenter);
    m_displayLabel->setScaledContents(false); // We'll handle scaling manually
    
    layout->addWidget(m_displayLabel);
    setCentralWidget(centralWidget);
    
    // Store display dimensions
    m_displayWidth = m_displayLabel->width();
    m_displayHeight = m_displayLabel->height();
}

void MainWindow::updateScreenCapture()
{
    // Capture new frame
    if (m_screenCapture.captureFrame()) {
        // Get the latest frame
        QImage frame = m_screenCapture.getLatestFrame();
        
        // Scale the frame to fit the display area while maintaining aspect ratio
        QImage scaledFrame = frame.scaled(m_displayLabel->size(), 
                                         Qt::KeepAspectRatio, 
                                         Qt::SmoothTransformation);
        
        // Display the scaled frame
        m_displayLabel->setPixmap(QPixmap::fromImage(scaledFrame));
    }
}