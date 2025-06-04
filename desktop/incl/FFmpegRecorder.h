#ifndef FFMPEGRECORDER_H
#define FFMPEGRECORDER_H

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavutil/audio_fifo.h>
}

#include <chrono>

class FFmpegRecorder {
public:
    FFmpegRecorder();
    ~FFmpegRecorder();

    // Initialize the recorder with specific parameters
    bool initialize(const char* outputFile, int width, int height, int fps,
        int sampleRate, int channels);

    bool sendVideoFrame(unsigned char* rgbaData, int64_t timestamp);
    bool sendAudioFrame(unsigned char* audioData, int dataSize, int64_t timestamp);

    int64_t getRecordingTimestamp();

    void close();

private:
    // Process audio, handling proper frame sizes for AAC
    bool processAudio(unsigned char* audioData, int dataSize, int64_t timestamp);

    // Helper to sanitize audio data
    void sanitizeAudioData(float* data, int numSamples);

    // Helper to convert audio format
    AVFrame* convertAudioFormat(unsigned char* audioData, int numSamples);

    int encodeVideoFrame(AVFrame* frame);
    int encodeAudioFrame(AVFrame* frame);

    // Helper to receive and write packets from encoders
    int receiveAndWritePackets(AVCodecContext* codecContext, AVStream* stream);

    // Ffmpeg context and codec contexts
    AVFormatContext* outputContext;
    AVStream* videoStream;
    AVStream* audioStream;
    AVCodecContext* videoCodecContext;
    AVCodecContext* audioCodecContext;
    SwsContext* swsContext;
    SwrContext* swrContext;
    int videoFrameCount;
    int audioFrameCount;
    int64_t startTimestamp = -1;
    //int64_t m_audioStartTimestamp = -1; // <--- This seems redundant

    // Audio buffering and timestamp management
    AVAudioFifo* m_audioFifo;
    int m_aacFrameSize;
    //int64_t m_lastAudioPts; // <--- This seems redundant if PTS is sample-based
    int64_t m_audioSamplesProcessed; // Correctly int64_t now

    //std::chrono::steady_clock::time_point m_audioStartTime; // <--- This seems redundant
    //std::chrono::steady_clock::time_point m_lastAudioEncodeTime; // <--- This seems redundant for overall sync
    bool m_audioTimingInitialized = false;
    //int64_t m_totalAudioSamplesQueued = 0; // Track total samples we've queued (can be removed if not used)

    std::chrono::high_resolution_clock::time_point m_recordingStartTime;

    //int64_t m_audioBasePts = 0; // <--- This might be the source of the audio being ahead if used incorrectly
};

#endif // FFMPEGRECORDER_H