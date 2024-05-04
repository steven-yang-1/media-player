#include <cstdio>
#include "MediaDecoder.h"
#include "CommonException.h"
#include "SDL2/SDL.h"
#include "Audio.h"

MediaDecoder::MediaDecoder(YuvRender* yuv_render) {
    video_codec_context = avcodec_alloc_context3(video_codec);
    video_codec_context->flags |= AV_CODEC_FLAG_LOW_DELAY;
    packet = av_packet_alloc();
    av_frame = av_frame_alloc();
    this->yuv_render = yuv_render;
}

int MediaDecoder::open_file(const char *path) {
    if (avformat_open_input(&format_context, path, nullptr, nullptr) != 0) {
        printf("Cannot open the specified file.");
        return -1;
    }

    if (avformat_find_stream_info(format_context, nullptr) < 0) {
        printf("Cannot find the stream info.");
        return -1;
    }

    video_stream_index = av_find_best_stream(format_context, AVMEDIA_TYPE_VIDEO, -1,-1, NULL, 0);
    audio_stream_index = av_find_best_stream(format_context, AVMEDIA_TYPE_AUDIO, -1,-1, NULL, 0);

    if (audio_stream_index == -1 || video_stream_index == -1) {
        printf("Cannot find the stream index.");
        return -1;
    }

    AVCodecParameters* audio_codec_params = format_context->streams[audio_stream_index]->codecpar;
    AVCodecParameters* video_codec_params = format_context->streams[video_stream_index]->codecpar;

    audio_codec = avcodec_find_decoder(audio_codec_params->codec_id);
    audio_codec_context = avcodec_alloc_context3(audio_codec);

    video_codec = avcodec_find_decoder(format_context->streams[video_stream_index]->codecpar->codec_id);
    if (avcodec_parameters_to_context(audio_codec_context, audio_codec_params) < 0 ||
        avcodec_parameters_to_context(video_codec_context, video_codec_params) < 0) {
        printf("Cannot fill the parameters of the audio/video codec context.");
        return -1;
    }

    if (!audio_codec_context || !video_codec_context) {
        printf("Cannot resolve the audio/video codec context.");
        return -1;
    }

    if (avcodec_open2(audio_codec_context, audio_codec, nullptr) < 0 ||
        avcodec_open2(video_codec_context, video_codec, nullptr) < 0) {
        throw CommonException("Failed to open video_codec.");
    }

    this->audio = new Audio(audio_codec_context);

    return 0;
}

MediaDecoder::~MediaDecoder() {
    av_packet_free(&packet);
    avcodec_free_context(&audio_codec_context);
    avcodec_free_context(&video_codec_context);
    av_frame_free(&av_frame);
    avcodec_close(video_codec_context);
    avcodec_close(audio_codec_context);
    delete video_codec;
    delete audio_codec;
    delete audio;
}

int MediaDecoder::play() {
    if (audio_stream_index == -1) {
        printf("Cannot open the audio stream.");
    }
    if (video_stream_index == -1) {
        printf("Cannot open the video stream.");
    }
    while (av_read_frame(format_context, packet)>= 0) {
        if (packet->stream_index == audio_stream_index) {
            int ret = avcodec_send_packet(audio_codec_context, packet);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                printf("Cannot play audio frame.");
                break;
            }
            if (ret < 0) {
                printf("Cannot play audio frame.");
                break;
            }
            while (avcodec_receive_frame(audio_codec_context, av_frame) == 0) {
                audio->render(av_frame);
                SDL_Delay(0.1);
            }
            av_frame_unref(av_frame);
        } else if (packet->stream_index == video_stream_index) {
            int ret = avcodec_send_packet(video_codec_context, packet);
            if (ret < 0) {
                printf("Cannot play video frame.");
                break;
            }
            while (avcodec_receive_frame(video_codec_context, av_frame) == 0) {
                yuv_render->render(av_frame);
                SDL_Delay(int(((double)(video_codec_context->framerate.den * 1.0 / video_codec_context->framerate.num)) * 1000));
                //SDL_Delay(int(video_codec_context->framerate.num * 1.0 / video_codec_context->framerate.den));
            }
        }
        av_packet_unref(packet);
        SDL_Event event;
        SDL_PollEvent(&event);
        switch (event.type)
        {
            case SDL_QUIT:
                goto __QUIT__;
            default:
                break;
        }
    }
    __QUIT__:
        return 0;
}

int MediaDecoder::pause() {
    return 0;
}