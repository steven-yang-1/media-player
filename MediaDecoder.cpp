#include <cstdio>
#include "MediaDecoder.h"
#include "CommonException.h"
#include "SDL2/SDL.h"
#include "Audio.h"
#include "Timer.h"

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
    timer = new Timer();
    timer->start();
    while (true) {
        int ret = av_read_frame(format_context, packet);
        if (ret < 0) {
            printf("Cannot read video frame.");
            break;
        }
        AVStream *stream = format_context->streams[packet->stream_index];
        double timebase = av_q2d(stream->time_base);
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
            }
            av_frame_unref(av_frame);
        } else if (packet->stream_index == video_stream_index) {
            int ret = avcodec_send_packet(video_codec_context, packet);
            if (ret < 0) {
                printf("Cannot play video frame.");
                break;
            }
            while (avcodec_receive_frame(video_codec_context, av_frame) == 0) {
                uint32_t video_time = (double)(av_frame->best_effort_timestamp) * timebase * 1000;
                while (true) {
                    if (timer->elapsed_ticks() >= video_time) {
                        break;
                    }
                }
                if (!moving_window) {
                    yuv_render->render(av_frame);
                }
            }
        }
        av_packet_unref(packet);

        SDL_Event event;
        while (SDL_PollEvent(&event) || is_pause) {
            int is_break = move_window_event(this->yuv_render->window, &event);
            if (is_break) {
                break;
            }
            if (event.type == SDL_KEYDOWN) {
                toggle();
                break;
            }
            if (event.type == SDL_QUIT) {
                goto __QUIT__;
            }
            if (is_pause) {
                SDL_Delay(1);
            }
        }
    }
    __QUIT__:
        return 0;
}

void MediaDecoder::pause() {
    is_pause = true;
    timer->pause();
}

void MediaDecoder::unpause() {
    is_pause = false;
    timer->unpause();
}

bool MediaDecoder::move_window_event(SDL_Window *window, SDL_Event *event) {
    if (event->type == SDL_MOUSEBUTTONDOWN && event->button.button == SDL_BUTTON_LEFT) {
        moving_window = true;
        pause();
    }
    if (event->type == SDL_MOUSEBUTTONUP && event->button.button == SDL_BUTTON_LEFT) {
        moving_window = false;
        unpause();
        return true;
    }
    if (event->type == SDL_MOUSEMOTION && moving_window) {
    }
    return false;
}

void MediaDecoder::toggle() {
    if (timer->paused) {
        unpause();
    } else {
        pause();
    }
}
