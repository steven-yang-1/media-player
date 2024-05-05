#include "Audio.h"
#include "SDL2/SDL.h"

Audio::Audio(AVCodecContext* audio_codec_context) {
    this->audio_codec_context = audio_codec_context;

    SDL_zero(wanted_spec);
    SDL_zero(have_spec);

    wanted_spec.freq = 44100;
    wanted_spec.format = AUDIO_S16SYS;
    wanted_spec.channels = audio_codec_context->channels;
    wanted_spec.silence = 0;
    wanted_spec.samples = 1024;

    if (SDL_OpenAudio(&wanted_spec, &have_spec) < 0) {
        printf("Cannot open audio device.");
    }

    audio_frame = av_frame_alloc();
    audio_frame->channel_layout = audio_codec_context->channel_layout;
    audio_frame->sample_rate = 44100;
    audio_frame->format = AV_SAMPLE_FMT_S16;

    resampler = swr_alloc_set_opts(NULL,
               audio_codec_context->channel_layout,
               AV_SAMPLE_FMT_S16,
               44100,
               audio_codec_context->channel_layout,
               audio_codec_context->sample_fmt,
               audio_codec_context->sample_rate,
               0,
               NULL);
    swr_init(resampler);

    dev = SDL_OpenAudioDevice(NULL, 0, &wanted_spec, &have_spec, 0);
    SDL_PauseAudioDevice(dev, 0);

    SDL_PauseAudio(0);
}

int Audio::render(AVFrame* av_frame) {
    int dst_samples = av_frame->channels * av_rescale_rnd(
            swr_get_delay(resampler, av_frame->sample_rate) + av_frame->nb_samples,
                44100,
                av_frame->sample_rate,
               AV_ROUND_UP);
    uint8_t *audiobuf = NULL;
    int ret = av_samples_alloc(&audiobuf,
                           NULL,
                           1,
                           dst_samples,
                           AV_SAMPLE_FMT_S16,
                           1);
    if (ret < 0) {
        printf("Failed to alloc audio samples.");
        return -1;
    }
    dst_samples = av_frame->channels * swr_convert(resampler,
                                                &audiobuf,
                                                dst_samples,
                                                (const uint8_t**) av_frame->data,
                                                   av_frame->nb_samples);
    ret = av_samples_fill_arrays(audio_frame->data,
                                 audio_frame->linesize,
                                 audiobuf,
                                 1,
                                 dst_samples,
                                 AV_SAMPLE_FMT_S16,
                                 1);
    if (ret < 0) {
        printf("Failed to fill audio buffer.");
        return -1;
    }
    SDL_QueueAudio(dev, audio_frame->data[0], audio_frame->linesize[0]);
    return 0;
}

Audio::~Audio() {
    av_frame_free(&audio_frame);
    avcodec_close(audio_codec_context);
    avcodec_free_context(&audio_codec_context);
    SDL_PauseAudio(1);
    SDL_CloseAudio();
    swr_free(&resampler);
    SDL_CloseAudioDevice(dev);
}