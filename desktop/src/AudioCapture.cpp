#define NOMINMAX

#include "incl/AudioCapture.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <cmath>
#include <QDebug>
#include <QTimer>
#include <filesystem> // Include this header
#include <QStandardPaths> // Add this include at the top of your .cpp file
#include <QDir>            // Already likely included, but double-check

AudioCapture::AudioCapture(QObject* parent)
    : QObject(parent),
    m_pEnumerator(nullptr),
    m_pInputDevice(nullptr),
    m_pOutputDevice(nullptr),
    m_pInputAudioClient(nullptr),
    m_pOutputAudioClient(nullptr),
    m_pInputCaptureClient(nullptr),
    m_pOutputCaptureClient(nullptr),
    m_pwfxInput(nullptr),
    m_pwfxOutput(nullptr),
    m_hInputEvent(nullptr),
    m_isCapturing(false),
    m_audioProcessingTimer(new QTimer(this)) // Initialize QTimer with this
    // as parent
{
    // Initialize timing variables (though not used in provided code snippet,
    // good to keep if intended)
    LARGE_INTEGER qpcFreq;
    QueryPerformanceFrequency(&qpcFreq);

    // Zero out the WAVEFORMATEX structure
    ZeroMemory(&m_wfx, sizeof(WAVEFORMATEX));

    // Initialize COM library
    HRESULT hr = CoInitialize(nullptr);
    if (FAILED(hr)) {
        qDebug() << "Failed to initialize COM library";
    }

    // Create device enumerator
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
        __uuidof(IMMDeviceEnumerator), (void**)&m_pEnumerator);
    if (FAILED(hr)) {
        qDebug() << "Failed to create device enumerator: " << hr;
    }

    // Connect the timer's timeout signal to our new processing slot
    connect(m_audioProcessingTimer, &QTimer::timeout, this,
        &AudioCapture::processAudioBuffer);
}

AudioCapture::~AudioCapture() {
    stopCapture(); // Ensure capture is stopped when object is destroyed

    // Release WASAPI COM interfaces
    if (m_pInputAudioClient)
        m_pInputAudioClient->Release();
    if (m_pOutputAudioClient)
        m_pOutputAudioClient->Release();
    if (m_pInputCaptureClient)
        m_pInputCaptureClient->Release();
    if (m_pOutputCaptureClient)
        m_pOutputCaptureClient->Release();
    if (m_pInputDevice)
        m_pInputDevice->Release();
    if (m_pOutputDevice)
        m_pOutputDevice->Release();
    if (m_pEnumerator)
        m_pEnumerator->Release();

    // Free WAVEFORMATEX memory
    if (m_pwfxInput)
        CoTaskMemFree(m_pwfxInput);
    if (m_pwfxOutput)
        CoTaskMemFree(m_pwfxOutput);

    // Close event handle
    if (m_hInputEvent) { // Note: If you have a separate event for output, close
        // that too.
        CloseHandle(m_hInputEvent);
        m_hInputEvent = nullptr;
    }

    CoUninitialize(); // Uninitialize COM library
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
    hr = m_pInputDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr,
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

    qDebug() << "WASAPI Input Format:";
    qDebug() << "wFormatTag:" << m_pwfxInput->wFormatTag;
    qDebug() << "nChannels:" << m_pwfxInput->nChannels;
    qDebug() << "nSamplesPerSec:" << m_pwfxInput->nSamplesPerSec;
    qDebug() << "wBitsPerSample:" << m_pwfxInput->wBitsPerSample;
    qDebug() << "nBlockAlign:" << m_pwfxInput->nBlockAlign;

    WAVEFORMATEXTENSIBLE* pwfxExt =
        reinterpret_cast<WAVEFORMATEXTENSIBLE*>(m_pwfxInput);
    if (pwfxExt->SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT) {
        qDebug() << "SubFormat: IEEE_FLOAT";
    }
    else if (pwfxExt->SubFormat == KSDATAFORMAT_SUBTYPE_PCM) {
        qDebug() << "SubFormat: PCM";
    }
    else {
        qDebug() << "SubFormat: Unknown";
    }

    // Create event for audio processing
    m_hInputEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (!m_hInputEvent) {
        qDebug() << "Failed to create input event";
        return false;
    }

    // Initialize audio client with event
    hr = m_pInputAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED,
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
    hr = m_pInputAudioClient->GetService(__uuidof(IAudioCaptureClient),
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
    hr = m_pOutputDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr,
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

    qDebug() << "WASAPI Output Format:";
    qDebug() << "wFormatTag:" << m_pwfxOutput->wFormatTag;
    qDebug() << "nChannels:" << m_pwfxOutput->nChannels;
    qDebug() << "nSamplesPerSec:" << m_pwfxOutput->nSamplesPerSec;
    qDebug() << "wBitsPerSample:" << m_pwfxOutput->wBitsPerSample;
    qDebug() << "nBlockAlign:" << m_pwfxOutput->nBlockAlign;

    WAVEFORMATEXTENSIBLE* pwfxExt =
        reinterpret_cast<WAVEFORMATEXTENSIBLE*>(m_pwfxOutput);
    if (pwfxExt->SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT) {
        qDebug() << "SubFormat: IEEE_FLOAT";
    }
    else if (pwfxExt->SubFormat == KSDATAFORMAT_SUBTYPE_PCM) {
        qDebug() << "SubFormat: PCM";
    }
    else {
        qDebug() << "SubFormat: Unknown";
    }

    // Initialize audio client in loopback mode
    hr = m_pOutputAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED,
        AUDCLNT_STREAMFLAGS_LOOPBACK,
        10000000, 0, m_pwfxOutput, nullptr);
    if (FAILED(hr)) {
        qDebug() << "Failed to initialize output audio client: " << hr;
        return false;
    }

    // Get capture client
    hr = m_pOutputAudioClient->GetService(__uuidof(IAudioCaptureClient),
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
            qDebug() << "Failed to start input capture:" << hr;
        }
        else {
            qDebug() << "Input audio client started.";
        }
    }

    if (m_pOutputAudioClient) {
        hr = m_pOutputAudioClient->Start();
        if (FAILED(hr)) {
            qDebug() << "Failed to start output capture:" << hr;
        }
        else {
            qDebug() << "Output audio client started.";
        }
    }

    m_isCapturing = true;
    m_audioProcessingTimer->start(
        10); // Start polling every 10ms for audio data
    qDebug() << "Audio capture and processing timer started.";
}

void AudioCapture::stopCapture() {
    if (m_audioProcessingTimer->isActive()) {
        m_audioProcessingTimer->stop();
        qDebug() << "Audio processing timer stopped.";
    }

    if (m_pInputAudioClient)
        m_pInputAudioClient->Stop();
    if (m_pOutputAudioClient)
        m_pOutputAudioClient->Stop();

    m_isCapturing = false;
    qDebug() << "Audio capture stopped.";
}

// Helper function to capture and emit audio data from a specific client
bool AudioCapture::captureAndEmitAudio(IAudioCaptureClient* captureClient,
    WAVEFORMATEX* pwfx) {
    if (!m_isCapturing || !captureClient || !pwfx) {
        return false;
    }

    UINT32 packetLength = 0;
    HRESULT hr = captureClient->GetNextPacketSize(&packetLength);
    if (FAILED(hr)) {
        qWarning() << "GetNextPacketSize failed:" << hr;
        return false;
    }

    // Only proceed if there's actual data
    if (packetLength == 0) {
        return false; // No data available right now
    }

    BYTE* pData;
    UINT32 numFramesAvailable;
    DWORD flags;
    UINT64 devicePosition; // Frame position of the captured data
    UINT64 qpcPosition;    // QPC timestamp of the captured data

    hr = captureClient->GetBuffer(&pData, &numFramesAvailable, &flags,
        &devicePosition, &qpcPosition);
    if (FAILED(hr)) {
        qWarning() << "GetBuffer failed:" << hr;
        return false;
    }

    if (numFramesAvailable == 0) {
        captureClient->ReleaseBuffer(0); // Release empty buffer if any
        return false;
    }

    UINT32 expectedBlockAlign = pwfx->nChannels * (pwfx->wBitsPerSample / 8);
    UINT32 dataSize = numFramesAvailable * expectedBlockAlign;

    if (!(flags & AUDCLNT_BUFFERFLAGS_SILENT) && pData != nullptr) {
        // CRITICAL: Make a deep copy of the audio data.
        // The `pData` pointer is only valid until `ReleaseBuffer` is called.
        // The `RecordingManager` slot will process this on potentially another
        // thread
        // and needs its own copy.
        uchar* audioBufferCopy = new uchar[dataSize];
        memcpy(audioBufferCopy, pData, dataSize);

        static FILE* raw_audio_dump_file = nullptr;
        static bool file_initialized =
            false; // Add a flag to ensure path generation only happens once

        if (!file_initialized) {
            // Get the path to the user's Desktop
            QString desktopPath =
                QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
            if (desktopPath.isEmpty()) {
                qWarning() << "Failed to find Desktop location!";
                return false; // Or handle error appropriately
            }

            // Construct the full file path
            QString filePath = QDir(desktopPath).filePath("audio_dump.f32le");
            qDebug() << "Attempting to save raw audio dump to:" << filePath;

            raw_audio_dump_file = fopen(filePath.toLocal8Bit().constData(), "wb");
            if (!raw_audio_dump_file) {
                qWarning() << "Failed to open audio_dump.f32le for writing at:"
                    << filePath;
            }
            file_initialized = true; // Mark as initialized regardless of success, to
            // avoid re-attempting path creation
        }

        if (raw_audio_dump_file) {
            fwrite(audioBufferCopy, 1, dataSize, raw_audio_dump_file);
        }

        // Emit the signal with the copied data
        emit newAudioDataReady(audioBufferCopy, dataSize);

        // Ownership of `audioBufferCopy` is transferred to the receiver
        // (RecordingManager).
        // The receiver *must* call `delete[] audioBufferCopy` after processing it.
    }
    else {
        qDebug() << "[WASAPI] Silent or null audio data received, flags:" << flags;
    }

    // Always release the buffer
    hr = captureClient->ReleaseBuffer(numFramesAvailable);
    if (FAILED(hr)) {
        qWarning() << "Failed to release audio buffer:" << hr;
    }

    return true; // Successfully processed a packet
}

// Public slot to be called by the QTimer
void AudioCapture::processAudioBuffer() {
    if (!m_isCapturing) {
        return;
    }

    // Process input audio (microphone) if initialized
    if (m_pInputCaptureClient && m_pwfxInput) {
        captureAndEmitAudio(m_pInputCaptureClient, m_pwfxInput);
    }

    // Process output audio (system sound) if initialized
    if (m_pOutputCaptureClient && m_pwfxOutput) {
        captureAndEmitAudio(m_pOutputCaptureClient, m_pwfxOutput);
    }
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
    hr = m_pOutputCaptureClient->GetBuffer(&pData, &numFramesAvailable, &flags,
        nullptr, nullptr);
    if (FAILED(hr) || numFramesAvailable == 0) {
        return -100.0f;
    }

    float rmsVolume =
        calculateRMSVolume(pData, numFramesAvailable, m_pwfxOutput);
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
    hr = m_pInputCaptureClient->GetBuffer(&pData, &numFramesAvailable, &flags,
        nullptr, nullptr);
    if (FAILED(hr) || numFramesAvailable == 0) {
        return -100.0f;
    }

    float rmsVolume = calculateRMSVolume(pData, numFramesAvailable, m_pwfxInput);
    float volumeDb = convertToDecibels(rmsVolume);

    // Always release the buffer
    m_pInputCaptureClient->ReleaseBuffer(numFramesAvailable);

    return volumeDb;
}

float AudioCapture::calculateRMSVolume(BYTE* pData, UINT32 numFramesAvailable,
    WAVEFORMATEX* pwfx) {
    if (!pData || numFramesAvailable == 0 || !pwfx)
        return 0.0f;

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
                sample32 = (pData[i * 3] << 8) | (pData[i * 3 + 1] << 16) |
                    (pData[i * 3 + 2] << 24);
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
    if (rmsValue <= MIN_RMS)
        return -100.0f;

    // Convert to decibels full scale (dBFS)
    float db = 20.0f * std::log10(rmsValue);

    // Limit range
    if (db < -100.0f)
        db = -100.0f;
    return db;
}

float AudioCapture::getCurrentVolume() {
    float outputVolume = getOutputVolume();
    float inputVolume = getInputVolume();

    // Debug info
    qDebug() << "Raw volumes - Input:" << inputVolume
        << "Output:" << outputVolume;

    // Return the maximum volume
    return std::max(outputVolume, inputVolume);
}

// Add getters for format info, essential for FFmpegRecorder initialization
int AudioCapture::getSampleRate() const {
    // Prioritize output format for system sound, or input if only mic is used
    if (m_pwfxOutput)
        return m_pwfxOutput->nSamplesPerSec;
    if (m_pwfxInput)
        return m_pwfxInput->nSamplesPerSec;
    return 0; // Return 0 if no format is initialized
}

int AudioCapture::getChannels() const {
    if (m_pwfxOutput)
        return m_pwfxOutput->nChannels;
    if (m_pwfxInput)
        return m_pwfxInput->nChannels;
    return 0;
}

int AudioCapture::getBitsPerSample() const {
    if (m_pwfxOutput)
        return m_pwfxOutput->wBitsPerSample;
    if (m_pwfxInput)
        return m_pwfxInput->wBitsPerSample;
    return 0;
}