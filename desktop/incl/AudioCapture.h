#pragma once

#include <mmdeviceapi.h>
#include <audioclient.h>
#include <audiopolicy.h>
#include <algorithm>
#include <cmath>
#include <atomic>

#include "FFmpegRecorder.h"

class AudioCapture {
public: 
	AudioCapture();
	~AudioCapture();

	bool initializeInput(); // For microphone
	bool initializeOutput(); // For system audio
	
	void startCapture();
	void stopCapture();

	float getOutputVolume();
	float getInputVolume();
	float getCurrentVolume(); // Returns volums in dbFS

	// Process audio data for recording
	bool processAudioForRecording(FFmpegRecorder* recorder);

	int getSampleRate() const { return m_wfx.nSamplesPerSec; }
	int getChannels() const { return m_wfx.nChannels; }

	HANDLE m_hInputEvent = nullptr;

private:
	// device interfaces (WASAPI components)
	IMMDeviceEnumerator* m_pEnumerator = nullptr;
	IMMDevice* m_pInputDevice = nullptr;
	IMMDevice* m_pOutputDevice = nullptr;
	IAudioClient* m_pInputAudioClient = nullptr;
	IAudioClient* m_pOutputAudioClient = nullptr;
	IAudioCaptureClient* m_pInputCaptureClient = nullptr;
	IAudioCaptureClient* m_pOutputCaptureClient = nullptr;

	WAVEFORMATEX* m_pwfxInput = nullptr;
	WAVEFORMATEX* m_pwfxOutput = nullptr;

	float calculateRMSVolume(BYTE* pData, UINT32 numFramesAvailable, WAVEFORMATEX* pwfx);
	float convertToDecibels(float rmsValue);

	// Audio format
	WAVEFORMATEX m_wfx;

	// Performance counter frequency for timing
	LONGLONG m_qpcFreq;

	// State tracking
	std::atomic<bool> m_initialized;

	bool m_isCapturing = false;
};