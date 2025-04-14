#include "incl/VolumeMeter.h"
#include <QPainter>
#include <QLinearGradient>
#include <QFontMetrics>

VolumeMeter::VolumeMeter(QWidget* parent, const QString& label)
    : QWidget(parent),
    m_currentLevel(-60.0f),
    m_targetLevel(-60.0f),
    m_peakLevel(-60.0f),
    m_label(label)
{
    setMinimumHeight(30);
    m_animationTimer.setInterval(16); // ~60 FPS
    connect(&m_animationTimer, &QTimer::timeout, this, &VolumeMeter::updateAnimation);
    m_animationTimer.start();
}

void VolumeMeter::setLevel(float dbLevel)
{
    dbLevel = qBound(-60.0f, dbLevel, 0.0f);
    m_targetLevel = dbLevel;

    if (dbLevel > m_peakLevel)
        m_peakLevel = dbLevel;
}

void VolumeMeter::updateAnimation()
{
    // Simple smoothing for animation
    const float smoothing = 0.2f;
    m_currentLevel = m_currentLevel * (1.0f - smoothing) + m_targetLevel * smoothing;

    // Slowly decay peak
    m_peakLevel -= 0.5f;
    if (m_peakLevel < m_currentLevel)
        m_peakLevel = m_currentLevel;

    update();
}

QLinearGradient VolumeMeter::createColorGradient() const
{
    QLinearGradient gradient(0, 0, width(), 0);
    gradient.setColorAt(0.0, Qt::green);
    gradient.setColorAt(0.66, Qt::yellow);
    gradient.setColorAt(1.0, Qt::red);
    return gradient;
}

void VolumeMeter::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QRect barRect = rect().adjusted(100, 8, -10, -8); // leave space for label

    // Draw background
    painter.setBrush(Qt::black);
    painter.setPen(Qt::NoPen);
    painter.drawRect(barRect);

    // Draw volume bar
    float normalizedLevel = (m_currentLevel + 60.0f) / 60.0f;
    int levelWidth = static_cast<int>(barRect.width() * normalizedLevel);

    QRect levelRect = barRect;
    levelRect.setWidth(levelWidth);

    QBrush gradientBrush(createColorGradient());
    painter.setBrush(gradientBrush);
    painter.drawRect(levelRect);

    // Draw peak indicator
    float peakNormalized = (m_peakLevel + 60.0f) / 60.0f;
    int peakX = barRect.left() + static_cast<int>(barRect.width() * peakNormalized);

    painter.setPen(Qt::white);
    painter.drawLine(peakX, barRect.top(), peakX, barRect.bottom());

    // Draw label
    painter.setPen(Qt::white);
    painter.drawText(10, height() / 2 + 6, m_label);
}
