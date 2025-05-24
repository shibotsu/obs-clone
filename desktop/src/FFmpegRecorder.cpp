#include "incl/FFmpegRecorder.h"
#include <utility>
#include <cmath>
#include <chrono>

FFmpegRecorder::FFmpegRecorder()
    : outputContext(nullptr), videoStream(nullptr), audioStream(nullptr),
    videoCodecContext(nullptr), audioCodecContext(nullptr),
    swsContext(nullptr), swrContext(nullptr), videoFrameCount(0), audioFrameCount(0),
    startTimestamp(-1), m_lastAudioPts(0), m_audioSamplesProcessed(0), m_audioFifo(nullptr), m_aacFrameSize(0) {
}

bool FFmpegRecorder::initialize(const char* outputFile, int width, int height, int fps,
    int sampleRate, int channels) {
    const AVOutputFormat* outputFormat = av_guess_format(nullptr, outputFile, nullptr);
    if (!outputFormat) return false;

    avformat_alloc_output_context2(&outputContext, outputFormat, outputFormat->name, outputFile);
    if (!outputContext) return false;

    const AVCodec* videoCodec;
    if (outputFormat->video_codec == AV_CODEC_ID_NONE) {
        videoCodec = avcodec_find_encoder(AV_CODEC_ID_H264);
    }
    else {
        videoCodec = avcodec_find_encoder(outputFormat->video_codec);
    }
    if (!videoCodec) return false;

    videoStream = avformat_new_stream(outputContext, nullptr);
    if (!videoStream) return false;

    videoCodecContext = avcodec_alloc_context3(videoCodec);
    videoCodecContext->codec_id = videoCodec->id;
    videoCodecContext->codec_type = AVMEDIA_TYPE_VIDEO;
    videoCodecContext->width = width;
    videoCodecContext->height = height;
    videoCodecContext->time_base = AVRational{ 1, fps };
    videoStream->time_base = videoCodecContext->time_base;
    videoCodecContext->pix_fmt = AV_PIX_FMT_YUV420P;
    videoCodecContext->bit_rate = 5000000;
    videoCodecContext->gop_size = fps * 2;

    av_opt_set(videoCodecContext->priv_data, "preset", "medium", 0);

    if (avcodec_open2(videoCodecContext, videoCodec, nullptr) < 0) return false;

    avcodec_parameters_from_context(videoStream->codecpar, videoCodecContext);

    const AVCodec* audioCodec;
    if (outputFormat->audio_codec == AV_CODEC_ID_NONE) {
        audioCodec = avcodec_find_encoder(AV_CODEC_ID_AAC);
    }
    else {
        audioCodec = avcodec_find_encoder(outputFormat->audio_codec);
    }
    if (!audioCodec) return false;

    audioStream = avformat_new_stream(outputContext, nullptr);
    if (!audioStream) return false;

    audioCodecContext = avcodec_alloc_context3(audioCodec);
    audioCodecContext->codec_id = audioCodec->id;
    audioCodecContext->codec_type = AVMEDIA_TYPE_AUDIO;
    audioCodecContext->sample_rate = sampleRate;
    av_channel_layout_default(&audioCodecContext->ch_layout, channels);

    // Make sure we use a compatible sample format
    if (audioCodec->sample_fmts) {
        audioCodecContext->sample_fmt = audioCodec->sample_fmts[0];
    }
    else {
        audioCodecContext->sample_fmt = AV_SAMPLE_FMT_FLTP;
    }

    audioCodecContext->bit_rate = 192000;
    audioCodecContext->time_base = AVRational{ 1, audioCodecContext->sample_rate };
    audioStream->time_base = audioCodecContext->time_base;

    if (avcodec_open2(audioCodecContext, audioCodec, nullptr) < 0) return false;

    avcodec_parameters_from_context(audioStream->codecpar, audioCodecContext);

    fprintf(stderr, "Video codec timebase: %d/%d\n", videoCodecContext->time_base.num, videoCodecContext->time_base.den);
    fprintf(stderr, "Audio codec timebase: %d/%d\n", audioCodecContext->time_base.num, audioCodecContext->time_base.den);

    // Setup SWS context for video conversion
    swsContext = sws_getContext(width, height, AV_PIX_FMT_BGRA,
        width, height, AV_PIX_FMT_YUV420P,
        SWS_BICUBIC, nullptr, nullptr, nullptr);

    // Setup audio resampler
    swrContext = swr_alloc();
    if (!swrContext) return false;

    // Set input properties
    AVChannelLayout inputLayout;
    av_channel_layout_default(&inputLayout, channels);
    av_opt_set_chlayout(swrContext, "in_chlayout", &inputLayout, 0);
    av_opt_set_int(swrContext, "in_sample_rate", sampleRate, 0);
    av_opt_set_sample_fmt(swrContext, "in_sample_fmt", AV_SAMPLE_FMT_FLT, 0);

    // Set output properties
    av_opt_set_chlayout(swrContext, "out_chlayout", &audioCodecContext->ch_layout, 0);
    av_opt_set_int(swrContext, "out_sample_rate", audioCodecContext->sample_rate, 0);
    av_opt_set_sample_fmt(swrContext, "out_sample_fmt", audioCodecContext->sample_fmt, 0);

    // Initialize the resampler
    if (swr_init(swrContext) < 0) {
        fprintf(stderr, "Failed to initialize audio resampler\n");
        swr_free(&swrContext);
        return false;
    }

    fprintf(stderr, "Audio resampler initialized: %s -> %s, %d Hz -> %d Hz\n",
        av_get_sample_fmt_name(AV_SAMPLE_FMT_FLT),
        av_get_sample_fmt_name(audioCodecContext->sample_fmt),
        sampleRate, audioCodecContext->sample_rate);

    // Set up AAC frame size
    m_aacFrameSize = audioCodecContext->frame_size;
    if (m_aacFrameSize <= 0) {
        m_aacFrameSize = 1024;
    }

    fprintf(stderr, "AAC frame size: %d samples\n", m_aacFrameSize);

    // Initialize audio FIFO for frame size buffering
    m_audioFifo = av_audio_fifo_alloc(audioCodecContext->sample_fmt,
        audioCodecContext->ch_layout.nb_channels,
        m_aacFrameSize * 8);
    if (!m_audioFifo) {
        fprintf(stderr, "Failed to allocate audio FIFO\n");
        swr_free(&swrContext);
        return false;
    }

    if (!(outputContext->oformat->flags & AVFMT_NOFILE)) {
        if (avio_open(&outputContext->pb, outputFile, AVIO_FLAG_WRITE) < 0) return false;
    }

    if (avformat_write_header(outputContext, nullptr) < 0) return false;

    startTimestamp = -1;
    m_audioTimingInitialized = false;
    m_totalAudioSamplesQueued = 0;
    m_audioSamplesProcessed = 0;

    // Get high-resolution start time for consistent timestamping
    m_recordingStartTime = std::chrono::high_resolution_clock::now();

    return true;
}

int64_t FFmpegRecorder::getRecordingTimestamp() {
    auto now = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_recordingStartTime);
    return elapsed.count();
}

bool FFmpegRecorder::sendVideoFrame(unsigned char* rgbaData, int64_t timestamp) {
    if (!rgbaData || !videoCodecContext || !swsContext) {
        return false;
    }

    // CRITICAL FIX: Use consistent recording timestamp instead of external timestamp
    int64_t recordingTimestamp = getRecordingTimestamp();

    // Initialize startTimestamp if not set
    if (startTimestamp == -1) {
        startTimestamp = recordingTimestamp;
        fprintf(stderr, "Video start timestamp initialized: %" PRId64 " ms\n", startTimestamp);
    }

    // Calculate relative timestamp
    int64_t relativeTimestamp = recordingTimestamp - startTimestamp;

    // Allocate video frame
    AVFrame* frame = av_frame_alloc();
    if (!frame) {
        return false;
    }

    // Configure frame
    frame->width = videoCodecContext->width;
    frame->height = videoCodecContext->height;
    frame->format = videoCodecContext->pix_fmt;

    // Allocate buffer
    int ret = av_frame_get_buffer(frame, 32);
    if (ret < 0) {
        av_frame_free(&frame);
        return false;
    }

    // Make sure frame is writable
    ret = av_frame_make_writable(frame);
    if (ret < 0) {
        av_frame_free(&frame);
        return false;
    }

    // Set up source data pointers
    uint8_t* srcData[1] = { rgbaData };
    int srcLinesize[1] = { 4 * videoCodecContext->width };

    // Convert from RGBA to YUV420P
    sws_scale(swsContext, srcData, srcLinesize, 0,
        videoCodecContext->height, frame->data, frame->linesize);

    // Convert timestamps properly from milliseconds to video timebase
    frame->pts = av_rescale_q(relativeTimestamp,
        AVRational{ 1, 1000 },
        videoCodecContext->time_base);

    // Encode frame
    bool success = (encodeVideoFrame(frame) >= 0);

    fprintf(stderr, "Video PTS: %" PRId64 " (%.3fs), relative timestamp: %" PRId64 " ms\n",
        frame->pts,
        (double)frame->pts * av_q2d(videoCodecContext->time_base),
        relativeTimestamp);

    // Free frame
    av_frame_free(&frame);

    return success;
}

bool FFmpegRecorder::processAudio(unsigned char* audioData, int dataSize, int64_t timestamp) {
    if (!audioData || dataSize <= 0 || !audioCodecContext || !m_audioFifo) {
        return false;
    }

    // Use consistent wall-clock timestamp
    int64_t recordingTimestamp = getRecordingTimestamp();

    // Initialize audio timing
    if (!m_audioTimingInitialized) {
        m_audioTimingInitialized = true;

        if (startTimestamp == -1) {
            startTimestamp = recordingTimestamp;
            fprintf(stderr, "Audio start timestamp initialized: %" PRId64 " ms\n", startTimestamp);
        }

        m_audioStartTimestamp = startTimestamp;
        m_lastAudioPts = 0; // This can remain 0, as we will derive PTS differently
        m_audioSamplesProcessed = 0;

        fprintf(stderr, "Audio timing initialized at recording start\n");
    }

    // int64_t relativeTimestamp = recordingTimestamp - startTimestamp; // Keep for logging if needed

    AVSampleFormat inputFormat = AV_SAMPLE_FMT_FLT;
    int bytesPerSample = av_get_bytes_per_sample(inputFormat);
    int numChannels = audioCodecContext->ch_layout.nb_channels;
    int numSamples = dataSize / (bytesPerSample * numChannels);

    if (numSamples <= 0) {
        return true;
    }

    sanitizeAudioData(reinterpret_cast<float*>(audioData), numSamples * numChannels);

    AVFrame* convertedFrame = convertAudioFormat(audioData, numSamples);
    if (!convertedFrame) {
        return false;
    }

    int samplesWritten = av_audio_fifo_write(m_audioFifo, (void**)convertedFrame->data, convertedFrame->nb_samples);
    if (samplesWritten < convertedFrame->nb_samples) {
        fprintf(stderr, "Failed to write audio data to FIFO\n");
        av_frame_free(&convertedFrame);
        return false;
    }

    m_totalAudioSamplesQueued += convertedFrame->nb_samples;

    // fprintf(stderr, "Audio processed %d samples, relative timestamp: %" PRId64 " ms\n",
    //     numSamples, relativeTimestamp); // You can keep this for debugging if you want

    av_frame_free(&convertedFrame);

    while (av_audio_fifo_size(m_audioFifo) >= m_aacFrameSize) {
        AVFrame* outputFrame = av_frame_alloc();
        if (!outputFrame) {
            fprintf(stderr, "Could not allocate output frame\n");
            return false;
        }

        outputFrame->nb_samples = m_aacFrameSize;
        outputFrame->format = audioCodecContext->sample_fmt;
        av_channel_layout_copy(&outputFrame->ch_layout, &audioCodecContext->ch_layout);

        if (av_frame_get_buffer(outputFrame, 0) < 0) {
            fprintf(stderr, "Could not allocate output frame data\n");
            av_frame_free(&outputFrame);
            return false;
        }

        av_audio_fifo_read(m_audioFifo, (void**)outputFrame->data, m_aacFrameSize);

        // --- CRITICAL FIX START ---
        // Calculate the PTS for the audio frame based on the wall-clock recording time
        // This ensures audio frames are timestamped relative to when they "should" be played
        // based on the overall recording duration.
        int64_t currentRecordingRelativeMs = recordingTimestamp - startTimestamp;

        // Estimate the PTS for this audio frame.
        // It's the total samples processed so far, converted to the audio timebase.
        // This effectively *pushes* audio frames forward in time if the audio processing
        // is lagging the real-time clock.
        outputFrame->pts = av_rescale_q(
            m_audioSamplesProcessed,
            AVRational{ 1, audioCodecContext->sample_rate }, // Input timebase: samples
            audioStream->time_base                           // Output timebase: audio stream's timebase
        );

        // Adjust PTS based on the relative recording timestamp.
        // This attempts to synchronize the audio PTS with the video's wall-clock driven PTS.
        // If the audio is behind, its PTS will be made larger (later) to catch up.
        // If the audio is ahead, its PTS will be made smaller (earlier) to align.
        int64_t expectedAudioPtsAtCurrentTime = av_rescale_q(
            currentRecordingRelativeMs,
            AVRational{ 1, 1000 },
            audioStream->time_base
        );

        // If the current audio PTS is significantly behind the expected PTS based on wall-clock,
        // we advance it. This helps correct for any processing delays.
        // A threshold of 50ms is a common starting point for small adjustments.
        int64_t pts_diff_ms = av_rescale_q(
            expectedAudioPtsAtCurrentTime - outputFrame->pts,
            audioStream->time_base,
            AVRational{ 1, 1000 }
        );

        // Apply a correction if the audio is falling behind.
        // This is a dynamic adjustment to keep audio in sync with the video's progression.
        // You might need to experiment with the 50ms threshold.
        if (pts_diff_ms > 50) {
            outputFrame->pts = expectedAudioPtsAtCurrentTime;
            fprintf(stderr, "[AUDIO SYNC] Correcting audio PTS from %" PRId64 " to %" PRId64 " (Diff: %" PRId64 "ms)\n",
                outputFrame->pts - av_rescale_q(50, audioStream->time_base, AVRational{ 1, 1000 }), outputFrame->pts, pts_diff_ms);
        }
        // --- CRITICAL FIX END ---

        m_audioSamplesProcessed += m_aacFrameSize;

        double frameTimeSeconds = (double)outputFrame->pts * av_q2d(audioStream->time_base); // Use audioStream->time_base
        fprintf(stderr, "Audio frame PTS: %" PRId64 " (%.3fs), samples processed: %" PRId64 "\n",
            outputFrame->pts, frameTimeSeconds, m_audioSamplesProcessed);

        if (encodeAudioFrame(outputFrame) < 0) {
            fprintf(stderr, "Failed to encode audio frame\n");
            av_frame_free(&outputFrame);
            return false;
        }

        av_frame_free(&outputFrame);
    }

    return true;
}

// Helper method to sanitize audio data
void FFmpegRecorder::sanitizeAudioData(float* data, int numSamples) {
    int nanCount = 0;
    int infCount = 0;
    int outOfRangeCount = 0;

    for (int i = 0; i < numSamples; i++) {
        if (std::isnan(data[i])) {
            data[i] = 0.0f;
            nanCount++;
        }
        else if (std::isinf(data[i])) {
            data[i] = 0.0f;
            infCount++;
        }
        else if (data[i] > 1.0f || data[i] < -1.0f) {
            data[i] = std::max(-1.0f, std::min(1.0f, data[i]));
            outOfRangeCount++;
        }
    }

    if (nanCount > 0 || infCount > 0 || outOfRangeCount > 0) {
        fprintf(stderr, "Audio sanitization: %d NaN, %d Inf, %d out-of-range values\n",
            nanCount, infCount, outOfRangeCount);
    }
}

// Helper method to convert audio format
AVFrame* FFmpegRecorder::convertAudioFormat(unsigned char* audioData, int numSamples) {
    // Create input frame
    AVFrame* inputFrame = av_frame_alloc();
    if (!inputFrame) {
        fprintf(stderr, "Could not allocate input frame\n");
        return nullptr;
    }

    inputFrame->format = AV_SAMPLE_FMT_FLT;
    av_channel_layout_copy(&inputFrame->ch_layout, &audioCodecContext->ch_layout);
    inputFrame->nb_samples = numSamples;

    if (av_frame_get_buffer(inputFrame, 0) < 0) {
        fprintf(stderr, "Could not allocate input frame data\n");
        av_frame_free(&inputFrame);
        return nullptr;
    }

    // Copy audio data to input frame
    memcpy(inputFrame->data[0], audioData,
        numSamples * audioCodecContext->ch_layout.nb_channels * sizeof(float));

    // Create output frame for converted format
    AVFrame* outputFrame = av_frame_alloc();
    if (!outputFrame) {
        fprintf(stderr, "Could not allocate output frame\n");
        av_frame_free(&inputFrame);
        return nullptr;
    }

    outputFrame->format = audioCodecContext->sample_fmt;
    av_channel_layout_copy(&outputFrame->ch_layout, &audioCodecContext->ch_layout);
    outputFrame->nb_samples = numSamples;

    if (av_frame_get_buffer(outputFrame, 0) < 0) {
        fprintf(stderr, "Could not allocate output frame data\n");
        av_frame_free(&inputFrame);
        av_frame_free(&outputFrame);
        return nullptr;
    }

    // Convert FLT to FLTP (or whatever format the codec requires)
    int ret = swr_convert(swrContext,
        outputFrame->data, numSamples,
        (const uint8_t**)inputFrame->data, numSamples);

    if (ret < 0) {
        char errBuf[AV_ERROR_MAX_STRING_SIZE] = { 0 };
        av_strerror(ret, errBuf, AV_ERROR_MAX_STRING_SIZE);
        fprintf(stderr, "Error converting audio: %s\n", errBuf);
        av_frame_free(&inputFrame);
        av_frame_free(&outputFrame);
        return nullptr;
    }

    av_frame_free(&inputFrame);
    return outputFrame;
}

bool FFmpegRecorder::sendAudioFrame(unsigned char* audioData, int dataSize, int64_t timestamp) {
    return processAudio(audioData, dataSize, timestamp);
}

int FFmpegRecorder::encodeVideoFrame(AVFrame* frame) {
    int ret = avcodec_send_frame(videoCodecContext, frame);
    if (ret < 0) {
        char errBuf[AV_ERROR_MAX_STRING_SIZE] = { 0 };
        av_strerror(ret, errBuf, AV_ERROR_MAX_STRING_SIZE);
        fprintf(stderr, "Error sending frame to video encoder: %s\n", errBuf);
        return ret;
    }

    return receiveAndWritePackets(videoCodecContext, videoStream);
}

int FFmpegRecorder::encodeAudioFrame(AVFrame* frame) {
    int ret = avcodec_send_frame(audioCodecContext, frame);
    if (ret < 0) {
        char errBuf[AV_ERROR_MAX_STRING_SIZE] = { 0 };
        av_strerror(ret, errBuf, AV_ERROR_MAX_STRING_SIZE);
        fprintf(stderr, "Error sending frame to audio encoder: %s\n", errBuf);
        return ret;
    }

    return receiveAndWritePackets(audioCodecContext, audioStream);
}

int FFmpegRecorder::receiveAndWritePackets(AVCodecContext* codecContext, AVStream* stream) {
    int ret = 0;
    AVPacket* pkt = av_packet_alloc();
    if (!pkt) {
        return AVERROR(ENOMEM);
    }

    while (true) {
        ret = avcodec_receive_packet(codecContext, pkt);

        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            break;
        }
        else if (ret < 0) {
            char errBuf[AV_ERROR_MAX_STRING_SIZE] = { 0 };
            av_strerror(ret, errBuf, AV_ERROR_MAX_STRING_SIZE);
            fprintf(stderr, "Error receiving packet: %s\n", errBuf);
            break;
        }

        pkt->stream_index = stream->index;
        av_packet_rescale_ts(pkt, codecContext->time_base, stream->time_base);

        ret = av_interleaved_write_frame(outputContext, pkt);
        if (ret < 0) {
            char errBuf[AV_ERROR_MAX_STRING_SIZE] = { 0 };
            av_strerror(ret, errBuf, AV_ERROR_MAX_STRING_SIZE);
            fprintf(stderr, "Error writing frame: %s\n", errBuf);
            break;
        }

        av_packet_unref(pkt);
    }

    av_packet_free(&pkt);
    return ret == AVERROR(EAGAIN) ? 0 : ret;
}

void FFmpegRecorder::close() {
    if (outputContext) {
        if (!(outputContext->oformat->flags & AVFMT_NOFILE) && outputContext->pb) {
            av_write_trailer(outputContext);
        }

        if (!(outputContext->oformat->flags & AVFMT_NOFILE) && outputContext->pb) {
            avio_closep(&outputContext->pb);
        }
    }

    if (videoCodecContext) {
        avcodec_free_context(&videoCodecContext);
        videoCodecContext = nullptr;
    }

    if (audioCodecContext) {
        avcodec_free_context(&audioCodecContext);
        audioCodecContext = nullptr;
    }

    if (outputContext) {
        avformat_free_context(outputContext);
        outputContext = nullptr;
    }

    if (swsContext) {
        sws_freeContext(swsContext);
        swsContext = nullptr;
    }

    if (swrContext) {
        swr_free(&swrContext);
        swrContext = nullptr;
    }

    if (m_audioFifo) {
        av_audio_fifo_free(m_audioFifo);
        m_audioFifo = nullptr;
    }

    videoStream = nullptr;
    audioStream = nullptr;
    videoFrameCount = 0;
    audioFrameCount = 0;
    startTimestamp = -1;

    m_audioTimingInitialized = false;
    m_totalAudioSamplesQueued = 0;
}

FFmpegRecorder::~FFmpegRecorder() {
    close();
}