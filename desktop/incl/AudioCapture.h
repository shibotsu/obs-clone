#pragma once

#include <mmdeviceapi.h>
#include <audioclient.h>
#include <audiopolicy.h>
#include <algorithm>
#include <cmath>

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

	HANDLE m_hInputEvent = nullptr;

private:
	// device interfaces
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

	// Method to safely get audio buffer
	bool getCaptureBuffer(
		IAudioCaptureClient* pCaptureClient,
		WAVEFORMATEX* pwfx,
		BYTE** ppData,
		UINT32* pNumFramesAvailable
	);

	bool m_isCapturing = false;
};