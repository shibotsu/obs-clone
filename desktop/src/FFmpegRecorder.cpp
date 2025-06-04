#include "incl/FFmpegRecorder.h"
#include <utility>
#include <cmath>
#include <chrono>

FFmpegRecorder::FFmpegRecorder()
    : outputContext(nullptr), videoStream(nullptr), audioStream(nullptr),
    videoCodecContext(nullptr), audioCodecContext(nullptr),
    swsContext(nullptr), swrContext(nullptr), videoFrameCount(0), audioFrameCount(0),
    startTimestamp(-1), m_audioSamplesProcessed(0), m_audioFifo(nullptr), m_aacFrameSize(0) {
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

    av_opt_set(videoCodecContext->priv_data, "crf", "23", 0); 

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

    // Remove this line, it's not needed with the new PTS calculation
    // int64_t recordingTimestamp = getRecordingTimestamp(); 

    // Initialize startTimestamp if not set (This logic is still good for *initial* alignment)
    if (startTimestamp == -1) {
        startTimestamp = getRecordingTimestamp(); // Use wall-clock for overall start
        fprintf(stderr, "Video start timestamp initialized: %" PRId64 " ms\n", startTimestamp);
    }

    // Calculate relative timestamp based on the incoming 'timestamp' from the capture source,
    // adjusted by startTimestamp. This is for logging/debugging the capture time,
    // but not for the final PTS if we are using frame count.
    // If you *must* use the timestamp argument, use this:
    // int64_t relativeTimestamp = timestamp - startTimestamp;
    // However, for strict linearity, it's better to rely on frame count for video.
    // So, let's keep it simple and base it on frameCount.
    int64_t current_video_pts = videoFrameCount; // This is the frame number in video timebase (1/fps)

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

    // --- CRITICAL CHANGE FOR VIDEO PTS ---
    // Video PTS is now based on sequential frame count.
    // This creates a perfectly monotonic video timeline that aligns with audio.
    frame->pts = current_video_pts;

    // Encode frame
    bool success = (encodeVideoFrame(frame) >= 0);

    // Increment frame count *only if encoding was successful*
    if (success) {
        videoFrameCount++;
    }

    fprintf(stderr, "Video PTS: %" PRId64 " (%.3fs), frame count: %d\n",
        frame->pts,
        (double)frame->pts * av_q2d(videoCodecContext->time_base),
        videoFrameCount); // Log videoFrameCount for clarity

    // Free frame
    av_frame_free(&frame);

    return success;
}

bool FFmpegRecorder::processAudio(unsigned char* audioData, int dataSize, int64_t timestamp) {
    if (!audioData || dataSize <= 0 || !audioCodecContext || !m_audioFifo) {
        return false;
    }

    if (!m_audioTimingInitialized) {
        m_audioTimingInitialized = true;
        // m_audioSamplesProcessed was already set to 0 in initialize().
        // This is where the *first* audio processing event happens for the recording.
        fprintf(stderr, "Audio processing timing initialized for this recording.\n");
    }

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

    // m_totalAudioSamplesQueued += convertedFrame->nb_samples; // Not directly used for PTS calculation now

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

        // --- CORRECTED PTS CALCULATION ---
        // Calculate PTS based on the total number of samples *that have been encoded and sent out*.
        // This ensures the audio stream's PTS progresses at the exact pace dictated by its sample rate.
        outputFrame->pts = av_rescale_q(
            m_audioSamplesProcessed,
            AVRational{ 1, audioCodecContext->sample_rate }, // Source is samples at sample_rate
            audioStream->time_base                          // Destination is stream's time_base
        );

        m_audioSamplesProcessed += m_aacFrameSize; // Increment AFTER calculating PTS for the current frame

        double frameTimeSeconds = (double)outputFrame->pts * av_q2d(audioStream->time_base);
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

    fprintf(stderr, "Pre-swr_convert: inputFrame->format = %s, channels = %d, nb_samples = %d\n",
        av_get_sample_fmt_name((AVSampleFormat)inputFrame->format),
        inputFrame->ch_layout.nb_channels, inputFrame->nb_samples);
    fprintf(stderr, "Pre-swr_convert: Output format = %s, channels = %d\n",
        av_get_sample_fmt_name(audioCodecContext->sample_fmt),
        audioCodecContext->ch_layout.nb_channels);

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
    // Set the unified recording start timestamp if this is the very first frame (video or audio)
    if (startTimestamp == -1) {
        startTimestamp = getRecordingTimestamp(); // Capture the current elapsed time
        fprintf(stderr, "Unified recording start timestamp initialized by audio (from sendAudioFrame): %" PRId64 " ms\n", startTimestamp);
    }

    // Now call the private processing function
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
}

FFmpegRecorder::~FFmpegRecorder() {
    close();
}