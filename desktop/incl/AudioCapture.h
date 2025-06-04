#ifndef AUDIOCAPTURE_H
#define AUDIOCAPTURE_H

#include <QObject>
#include <QTimer>
#include <QMutex>
#include <chrono>

#include <mmdeviceapi.h>
#include <audioclient.h>
#include <audiopolicy.h>
#include <algorithm>
#include <cmath>
#include <atomic>

#include "FFmpegRecorder.h"

class AudioCapture : public QObject {
	Q_OBJECT

public: 
	AudioCapture(QObject* parent = nullptr);
	~AudioCapture();

	bool initializeInput(); // For microphone
	bool initializeOutput(); // For system audio
	void startCapture();
	void stopCapture();

	float getOutputVolume();
	float getInputVolume();
	float calculateRMSVolume(BYTE* pData, UINT32 dataSize, WAVEFORMATEX* pwfx);
	float convertToDecibels(float rmsValue);
	float getCurrentVolume(); // Returns volume in dbFS

	void processAudioBuffer();

	int getSampleRate() const;
	int getChannels() const;
	int getBitsPerSample() const;
	WAVEFORMATEX* getInputFormat() const { return m_pwfxInput; }
	WAVEFORMATEX* getOutputFormat() const { return m_pwfxOutput; }

signals:
	void newAudioDataReady(const uchar* data, int dataSize);

private:
	// WASAPI COM objects
	IMMDeviceEnumerator* m_pEnumerator = nullptr;
	IMMDevice* m_pInputDevice = nullptr;
	IMMDevice* m_pOutputDevice = nullptr;
	IAudioClient* m_pInputAudioClient = nullptr;
	IAudioClient* m_pOutputAudioClient = nullptr;
	IAudioCaptureClient* m_pInputCaptureClient = nullptr;
	IAudioCaptureClient* m_pOutputCaptureClient = nullptr;

	WAVEFORMATEX* m_pwfxInput = nullptr;
	WAVEFORMATEX* m_pwfxOutput = nullptr;
	WAVEFORMATEX m_wfx;
	HANDLE m_hInputEvent = nullptr;

	bool m_isCapturing;

    // Qt Timer for periodic audio buffer processing
    QTimer* m_audioProcessingTimer;

    // Timestamp reference to ensure audio and video timestamps are relative to the same start point
    //std::chrono::high_resolution_clock::time_point m_recordingStartTime;

    // Helper function to process data from a single capture client
    bool captureAndEmitAudio(IAudioCaptureClient* captureClient, WAVEFORMATEX* pwfx);
};

#endif