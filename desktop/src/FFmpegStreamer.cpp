#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>

class FFmpegStreamer {
private:
    AVFormatContext* outputContext = nullptr;
    AVStream* videoStream = nullptr;
    AVStream* audioStream = nullptr;
    AVCodecContext* videoCodecContext = nullptr;
    AVCodecContext* audioCodecContext = nullptr;
    SwsContext* swsContext = nullptr;

    int videoFrameCount = 0;
    int audioFrameCount = 0;

public:
    bool initialize(const char* rtmpUrl, int width, int height, int fps,
        int sampleRate, int channels) {
        // Initialize FFmpeg libraries
        avformat_network_init();

        // Allocate output format context for RTMP
        avformat_alloc_output_context2(&outputContext, nullptr, "flv", rtmpUrl);
        if (!outputContext) {
            return false;
        }

        // Set up video stream
        const AVCodec* videoCodec = avcodec_find_encoder(AV_CODEC_ID_H264);
        if (!videoCodec) {
            return false;
        }

        videoStream = avformat_new_stream(outputContext, nullptr);
        if (!videoStream) {
            return false;
        }

        videoCodecContext = avcodec_alloc_context3(videoCodec);
        videoCodecContext->codec_id = AV_CODEC_ID_H264;
        videoCodecContext->codec_type = AVMEDIA_TYPE_VIDEO;
        videoCodecContext->width = width;
        videoCodecContext->height = height;
        videoCodecContext->time_base = AVRational{ 1, fps };
        videoCodecContext->pix_fmt = AV_PIX_FMT_YUV420P;
        videoCodecContext->bit_rate = 2500000;  // 2.5 Mbps
        videoCodecContext->gop_size = fps; // Keyframe every second

        // Use ultrafast preset for real-time streaming
        av_opt_set(videoCodecContext->priv_data, "preset", "ultrafast", 0);
        av_opt_set(videoCodecContext->priv_data, "tune", "zerolatency", 0);

        if (avcodec_open2(videoCodecContext, videoCodec, nullptr) < 0) {
            return false;
        }

        avcodec_parameters_from_context(videoStream->codecpar, videoCodecContext);

        // Set up audio stream
        const AVCodec* audioCodec = avcodec_find_encoder(AV_CODEC_ID_AAC);
        if (!audioCodec) {
            return false;
        }

        audioStream = avformat_new_stream(outputContext, nullptr);
        if (!audioStream) {
            return false;
        }

        audioCodecContext = avcodec_alloc_context3(audioCodec);
        audioCodecContext->codec_id = AV_CODEC_ID_AAC;
        audioCodecContext->codec_type = AVMEDIA_TYPE_AUDIO;
        audioCodecContext->sample_rate = sampleRate;
        audioCodecContext->channels = channels;
        audioCodecContext->channel_layout = av_get_default_channel_layout(channels);
        audioCodecContext->sample_fmt = audioCodec->sample_fmts[0];
        audioCodecContext->bit_rate = 128000;  // 128 kbps
        audioCodecContext->time_base = AVRational{ 1, sampleRate };

        if (avcodec_open2(audioCodecContext, audioCodec, nullptr) < 0) {
            return false;
        }

        avcodec_parameters_from_context(audioStream->codecpar, audioCodecContext);

        // Create conversion context for different pixel formats
        swsContext = sws_getContext(
            width, height, AV_PIX_FMT_BGRA,  // Source format (typical for Direct3D)
            width, height, AV_PIX_FMT_YUV420P, // Target format
            SWS_BICUBIC, nullptr, nullptr, nullptr);

        // Open RTMP connection
        if (!(outputContext->oformat->flags & AVFMT_NOFILE)) {
            if (avio_open(&outputContext->pb, rtmpUrl, AVIO_FLAG_WRITE) < 0) {
                return false;
            }
        }

        // Write stream header
        if (avformat_write_header(outputContext, nullptr) < 0) {
            return false;
        }

        return true;
    }

    bool sendVideoFrame(unsigned char* rgbaData, int64_t timestamp) {
        // Allocate frame buffer
        AVFrame* frame = av_frame_alloc();
        frame->width = videoCodecContext->width;
        frame->height = videoCodecContext->height;
        frame->format = videoCodecContext->pix_fmt;

        av_frame_get_buffer(frame, 32);

        // Convert BGRA to YUV420P
        uint8_t* srcData[1] = { rgbaData };
        int srcLinesize[1] = { 4 * videoCodecContext->width };

        sws_scale(swsContext, srcData, srcLinesize, 0,
            videoCodecContext->height, frame->data, frame->linesize);

        // Set presentation timestamp
        frame->pts = av_rescale_q(timestamp,
            AVRational{ 1, 1000 }, // Milliseconds
            videoCodecContext->time_base);

        // Encode frame
        if (encodeVideoFrame(frame) < 0) {
            av_frame_free(&frame);
            return false;
        }

        av_frame_free(&frame);
        return true;
    }

    bool sendAudioFrame(unsigned char* audioData, int dataSize, int64_t timestamp) {
        AVFrame* frame = av_frame_alloc();
        frame->nb_samples = audioCodecContext->frame_size;
        frame->format = audioCodecContext->sample_fmt;
        frame->channel_layout = audioCodecContext->channel_layout;

        av_frame_get_buffer(frame, 0);

        // Copy audio data to frame
        // (You may need to convert formats depending on your WASAPI setup)
        memcpy(frame->data[0], audioData, dataSize);

        // Set presentation timestamp
        frame->pts = av_rescale_q(timestamp,
            AVRational{ 1, 1000 }, // Milliseconds
            audioCodecContext->time_base);

        // Encode frame
        if (encodeAudioFrame(frame) < 0) {
            av_frame_free(&frame);
            return false;
        }

        av_frame_free(&frame);
        return true;
    }

private:
    int encodeVideoFrame(AVFrame* frame) {
        AVPacket pkt;
        av_init_packet(&pkt);
        pkt.data = nullptr;
        pkt.size = 0;

        int ret = avcodec_send_frame(videoCodecContext, frame);
        if (ret < 0) {
            return ret;
        }

        while (ret >= 0) {
            ret = avcodec_receive_packet(videoCodecContext, &pkt);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                break;
            }
            else if (ret < 0) {
                return ret;
            }

            // Convert packet timestamps
            pkt.stream_index = videoStream->index;
            av_packet_rescale_ts(&pkt, videoCodecContext->time_base, videoStream->time_base);

            // Write packet to output
            ret = av_interleaved_write_frame(outputContext, &pkt);
            av_packet_unref(&pkt);
            if (ret < 0) {
                return ret;
            }
        }

        return 0;
    }

    int encodeAudioFrame(AVFrame* frame) {
        AVPacket pkt;
        av_init_packet(&pkt);
        pkt.data = nullptr;
        pkt.size = 0;

        int ret = avcodec_send_frame(audioCodecContext, frame);
        if (ret < 0) {
            return ret;
        }

        while (ret >= 0) {
            ret = avcodec_receive_packet(audioCodecContext, &pkt);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                break;
            }
            else if (ret < 0) {
                return ret;
            }

            pkt.stream_index = audioStream->index;
            av_packet_rescale_ts(&pkt, audioCodecContext->time_base, audioStream->time_base);

            ret = av_interleaved_write_frame(outputContext, &pkt);
            av_packet_unref(&pkt);
            if (ret < 0) {
                return ret;
            }
        }

        return 0;
    }

public:
    void close() {
        // Write trailer and close everything
        if (outputContext) {
            av_write_trailer(outputContext);
            if (!(outputContext->oformat->flags & AVFMT_NOFILE)) {
                avio_closep(&outputContext->pb);
            }
        }

        avcodec_free_context(&videoCodecContext);
        avcodec_free_context(&audioCodecContext);
        avformat_free_context(outputContext);
        sws_freeContext(swsContext);
    }

    ~FFmpegStreamer() {
        close();
    }
};