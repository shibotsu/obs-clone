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

   // Send video frame data do be encoded
   bool sendVideoFrame(unsigned char* rgbaData, int64_t timestamp);  
   
   // Send audio frame data to be encoded
   bool sendAudioFrame(unsigned char* audioData, int dataSize, int64_t timestamp);  
   
   // Close the recorder and finalize the output file
   void close();  

private:
    // Process audio, handling proper frame sizes for AAC
    bool processAudio(unsigned char* audioData, int dataSize, int64_t timestamp);

    // Helper to sanitize audio data
    void sanitizeAudioData(float* data, int numSamples);

    // Helper to convert audio format
    AVFrame* convertAudioFormat(unsigned char* audioData, int numSamples);

   // Encode a video frame
   int encodeVideoFrame(AVFrame* frame);  
   
   // Encode an audio frame
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
   int64_t m_audioStartTimestamp = -1;

   // Audio buffering and timestamp management
   AVAudioFifo* m_audioFifo;
   int m_aacFrameSize;
   int64_t m_lastAudioPts;  
   int m_audioSamplesProcessed;

   std::chrono::steady_clock::time_point m_audioStartTime;
   std::chrono::steady_clock::time_point m_lastAudioEncodeTime;
   bool m_audioTimingInitialized = false;
   int64_t m_totalAudioSamplesQueued = 0; // Track total samples we've queued

   std::chrono::high_resolution_clock::time_point m_recordingStartTime;
   int64_t getRecordingTimestamp();  // New method declaration

   int64_t m_audioBasePts = 0;
};  

#endif // FFMPEGRECORDER_H