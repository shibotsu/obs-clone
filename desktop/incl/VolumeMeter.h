#pragma once

#include <QWidget>
#include <QTimer>

class VolumeMeter : public QWidget
{
    Q_OBJECT

public:
    VolumeMeter(QWidget* parent = nullptr, const QString& label = "Volume");
    void setLevel(float dbLevel); // range: -60.0 to 0.0 dB

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    float m_currentLevel;      // Smoothed level
    float m_targetLevel;       // Latest volume input
    float m_peakLevel;
    QTimer m_animationTimer;

    QString m_label;

    void updateAnimation();
    QLinearGradient createColorGradient() const;
};
