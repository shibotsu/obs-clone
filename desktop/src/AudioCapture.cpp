#include "incl/AudioCapture.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <QDebug>

AudioCapture::AudioCapture() {
    HRESULT hr = CoInitialize(nullptr);
    if (FAILED(hr)) {
        qDebug() << "Failed to initialize COM library";
    }

    // Create device enumerator
    hr = CoCreateInstance(
        __uuidof(MMDeviceEnumerator),
        nullptr,
        CLSCTX_ALL,
        __uuidof(IMMDeviceEnumerator),
        (void**)&m_pEnumerator
    );

    if (FAILED(hr)) {
        qDebug() << "Failed to create device enumerator: " << hr;
    }
}

AudioCapture::~AudioCapture() {
    
    stopCapture();

    if (m_pInputAudioClient) m_pInputAudioClient->Release();
    if (m_pOutputAudioClient) m_pOutputAudioClient->Release();
    if (m_pInputCaptureClient) m_pInputCaptureClient->Release();
    if (m_pOutputCaptureClient) m_pOutputCaptureClient->Release();
    if (m_pInputDevice) m_pInputDevice->Release();
    if (m_pOutputDevice) m_pOutputDevice->Release();
    if (m_pEnumerator) m_pEnumerator->Release();

    if (m_pwfxInput) CoTaskMemFree(m_pwfxInput);
    if (m_pwfxOutput) CoTaskMemFree(m_pwfxOutput);

    if (m_hInputEvent) {
        CloseHandle(m_hInputEvent);
        m_hInputEvent = nullptr;
    }

    CoUninitialize();
}

bool AudioCapture::initializeInput() {
    
    if (!m_pEnumerator) {
        qDebug() << "Device enumerator not initialized";
        return false;
    }

    // Get default input device
    HRESULT hr = m_pEnumerator->GetDefaultAudioEndpoint(
        eCapture, eConsole, &m_pInputDevice);
    if (FAILED(hr)) {
        qDebug() << "Failed to get input device: " << hr;
        return false;
    }

    // Activate audio client
    hr = m_pInputDevice->Activate(
        __uuidof(IAudioClient), CLSCTX_ALL, nullptr,
        (void**)&m_pInputAudioClient);
    if (FAILED(hr)) {
        qDebug() << "Failed to activate input audio client: " << hr;
        return false;
    }

    // Get mix format
    hr = m_pInputAudioClient->GetMixFormat(&m_pwfxInput);
    if (FAILED(hr)) {
        qDebug() << "Failed to get input mix format: " << hr;
        return false;
    }

    // Create event for audio processing
    m_hInputEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (!m_hInputEvent) {
        qDebug() << "Failed to create input event";
        return false;
    }

    // Initialize audio client with event
    hr = m_pInputAudioClient->Initialize(
        AUDCLNT_SHAREMODE_SHARED,
        AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
        10000000, 0, m_pwfxInput, nullptr);
    if (FAILED(hr)) {
        qDebug() << "Failed to initialize input audio client: " << hr;
        return false;
    }

    // Set the event handle
    hr = m_pInputAudioClient->SetEventHandle(m_hInputEvent);
    if (FAILED(hr)) {
        qDebug() << "Failed to set input event handle: " << hr;
        return false;
    }

    // Get capture client
    hr = m_pInputAudioClient->GetService(
        __uuidof(IAudioCaptureClient),
        (void**)&m_pInputCaptureClient);
    if (FAILED(hr)) {
        qDebug() << "Failed to get input capture client: " << hr;
        return false;
    }

    qDebug() << "Input audio device initialized successfully";
    return true;
}

bool AudioCapture::initializeOutput() {
    if (!m_pEnumerator) {
        qDebug() << "Device enumerator not initialized";
        return false;
    }

    // Get default output device
    HRESULT hr = m_pEnumerator->GetDefaultAudioEndpoint(
        eRender, eConsole, &m_pOutputDevice);
    if (FAILED(hr)) {
        qDebug() << "Failed to get output device: " << hr;
        return false;
    }

    // Activate audio client
    hr = m_pOutputDevice->Activate(
        __uuidof(IAudioClient), CLSCTX_ALL, nullptr,
        (void**)&m_pOutputAudioClient);
    if (FAILED(hr)) {
        qDebug() << "Failed to activate output audio client: " << hr;
        return false;
    }

    // Get mix format
    hr = m_pOutputAudioClient->GetMixFormat(&m_pwfxOutput);
    if (FAILED(hr)) {
        qDebug() << "Failed to get output mix format: " << hr;
        return false;
    }

    // Initialize audio client in loopback mode
    hr = m_pOutputAudioClient->Initialize(
        AUDCLNT_SHAREMODE_SHARED,
        AUDCLNT_STREAMFLAGS_LOOPBACK,
        10000000, 0, m_pwfxOutput, nullptr);
    if (FAILED(hr)) {
        qDebug() << "Failed to initialize output audio client: " << hr;
        return false;
    }

    // Get capture client
    hr = m_pOutputAudioClient->GetService(
        __uuidof(IAudioCaptureClient),
        (void**)&m_pOutputCaptureClient);
    if (FAILED(hr)) {
        qDebug() << "Failed to get output capture client: " << hr;
        return false;
    }

    qDebug() << "Output audio device initialized successfully";
    return true;
}

void AudioCapture::startCapture() {
    
    HRESULT hr;
    
    if (m_pInputAudioClient) {
        hr = m_pInputAudioClient->Start();
        if (FAILED(hr)) {
            qDebug() << "Failed to start input capture: " << hr;
        }
    }

    if (m_pOutputAudioClient) {
        hr = m_pOutputAudioClient->Start();
        if (FAILED(hr)) {
            qDebug() << "Failed to start output capture: " << hr;
        }
    }

    m_isCapturing = true;
}

void AudioCapture::stopCapture() {

    if (m_pInputAudioClient) m_pInputAudioClient->Stop();
    if (m_pOutputAudioClient) m_pOutputAudioClient->Stop();
    m_isCapturing = false;
}

float AudioCapture::getOutputVolume() {

    if (!m_pOutputAudioClient || !m_pOutputCaptureClient) {
        qDebug() << "Output audio client not initialized";
        return -100.0f;
    }

    UINT32 packetLength = 0;
    HRESULT hr = m_pOutputCaptureClient->GetNextPacketSize(&packetLength);

    if (FAILED(hr) || packetLength == 0) {
        return -100.0f;
    }

    BYTE* pData;
    UINT32 numFramesAvailable;
    DWORD flags;

    hr = m_pOutputCaptureClient->GetBuffer(
        &pData, &numFramesAvailable, &flags, nullptr, nullptr);

    if (FAILED(hr) || numFramesAvailable == 0) {
        return -100.0f;
    }

    float rmsVolume = calculateRMSVolume(pData, numFramesAvailable, m_pwfxOutput);
    float volumeDb = convertToDecibels(rmsVolume);

    // Always release the buffer
    m_pOutputCaptureClient->ReleaseBuffer(numFramesAvailable);

    return volumeDb;
}

float AudioCapture::getInputVolume() {
    if (!m_pInputAudioClient || !m_pInputCaptureClient) {
        qDebug() << "Input audio client not initialized";
        return -100.0f;
    }

    UINT32 packetLength = 0;
    HRESULT hr = m_pInputCaptureClient->GetNextPacketSize(&packetLength);

    if (FAILED(hr) || packetLength == 0) {
        return -100.0f;
    }

    BYTE* pData;
    UINT32 numFramesAvailable;
    DWORD flags;

    hr = m_pInputCaptureClient->GetBuffer(
        &pData, &numFramesAvailable, &flags, nullptr, nullptr);

    if (FAILED(hr) || numFramesAvailable == 0) {
        return -100.0f;
    }

    float rmsVolume = calculateRMSVolume(pData, numFramesAvailable, m_pwfxInput);
    float volumeDb = convertToDecibels(rmsVolume);

    // Always release the buffer
    m_pInputCaptureClient->ReleaseBuffer(numFramesAvailable);

    return volumeDb;
}

float AudioCapture::calculateRMSVolume(
    BYTE* pData, UINT32 numFramesAvailable, WAVEFORMATEX* pwfx) {

    if (!pData || numFramesAvailable == 0 || !pwfx) return 0.0f;

    double sumSquared = 0.0;
    int bytesPerSample = pwfx->wBitsPerSample / 8;
    int channelCount = pwfx->nChannels;
    int totalSamples = numFramesAvailable * channelCount;

    // Process each sample
    for (int i = 0; i < totalSamples; ++i) {
        // Handle different bit depths correctly
        float sample = 0.0f;

        if (pwfx->wFormatTag == WAVE_FORMAT_PCM ||
            (pwfx->wFormatTag == WAVE_FORMAT_EXTENSIBLE &&
                pwfx->wBitsPerSample <= 16)) {

            // For 16-bit PCM audio
            if (pwfx->wBitsPerSample == 16) {
                int16_t sample16 = 0;
                memcpy(&sample16, pData + (i * bytesPerSample), bytesPerSample);
                sample = sample16 / 32768.0f;
            }
            // For 8-bit PCM audio
            else if (pwfx->wBitsPerSample == 8) {
                uint8_t sample8 = pData[i];
                sample = (sample8 - 128) / 128.0f; // 8-bit is unsigned
            }
        }
        // For float format (common in WASAPI)
        else if (pwfx->wFormatTag == WAVE_FORMAT_IEEE_FLOAT ||
            (pwfx->wFormatTag == WAVE_FORMAT_EXTENSIBLE &&
                pwfx->wBitsPerSample == 32)) {

            float sampleFloat = 0.0f;
            memcpy(&sampleFloat, pData + (i * bytesPerSample), bytesPerSample);
            sample = sampleFloat; // already normalized between -1.0 and 1.0
        }
        // For 24-bit or 32-bit integer PCM
        else if (pwfx->wBitsPerSample == 24 || pwfx->wBitsPerSample == 32) {
            int32_t sample32 = 0;

            if (pwfx->wBitsPerSample == 24) {
                // 24-bit samples need special handling
                sample32 = (pData[i * 3] << 8) | (pData[i * 3 + 1] << 16) | (pData[i * 3 + 2] << 24);
                sample32 >>= 8; // Align to 24 bits
                sample = sample32 / 8388608.0f; // 2^23
            }
            else {
                memcpy(&sample32, pData + (i * bytesPerSample), bytesPerSample);
                sample = sample32 / 2147483648.0f; // 2^31
            }
        }

        // Square and accumulate
        sumSquared += sample * sample;
    }

    // Calculate RMS
    if (totalSamples > 0) {
        return std::sqrt(sumSquared / totalSamples);
    }

    return 0.0f;
}

float AudioCapture::convertToDecibels(float rmsValue) {
    // Prevent log of very small values
    const float MIN_RMS = 0.0000001f; // -140 dBFS floor

    if (rmsValue <= MIN_RMS) return -100.0f;

    // Convert to decibels full scale (dBFS)
    float db = 20.0f * std::log10(rmsValue);

    // Limit range
    if (db < -100.0f) db = -100.0f;

    return db;
}

float AudioCapture::getCurrentVolume() {
    float outputVolume = getOutputVolume();
    float inputVolume = getInputVolume();

    // Debug info
    qDebug() << "Raw volumes - Input:" << inputVolume << "Output:" << outputVolume;

    // Return the maximum volume
    return std::max(outputVolume, inputVolume);
}