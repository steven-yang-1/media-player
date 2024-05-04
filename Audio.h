#ifndef MEDIASERVER_AUDIO_H
#define MEDIASERVER_AUDIO_H

extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
    #include <libavutil/audio_fifo.h>
    #include <libswresample/swresample.h>
    #include <libavutil/opt.h>
    #include <libavutil/frame.h>
}
#include "SDL2/SDL.h"
#include "SDL2/SDL_audio.h"

typedef struct {
    uint8_t* buffer;
    Uint32 size;
    Uint8* pos;
} AudioParams;

class Audio {
public:
    Audio(AVCodecContext* audio_codec_context);
    int render(AVFrame* av_frame);
    ~Audio();
private:
    AVCodecContext* audio_codec_context;
    SDL_AudioSpec wanted_spec;
    SDL_AudioSpec have_spec;
    AVFrame *audio_frame;
    SwrContext *resampler;
    SDL_AudioDeviceID dev;
};

#endif //MEDIASERVER_AUDIO_H
