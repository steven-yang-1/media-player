#ifndef MEDIASERVER_MEDIADECODER_H
#define MEDIASERVER_MEDIADECODER_H

#include "YuvRender.h"
#include "Audio.h"

extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
    #include <libavutil/audio_fifo.h>
    #include <libswresample/swresample.h>
    #include <libavutil/opt.h>
    #include <libavutil/frame.h>
}

class MediaDecoder {
public:
    MediaDecoder(YuvRender* yuv_render);
    int open_file(const char* path);
    int play();
    int pause();
    ~MediaDecoder();
private:
    const AVCodec* audio_codec{};
    const AVCodec* video_codec{};
    long video_stream_index = -1;
    long audio_stream_index = -1;
    AVCodecContext* video_codec_context{};
    AVCodecContext* audio_codec_context{};
    AVPacket* packet{};
    AVFrame* av_frame{};
    AVFormatContext* format_context{};
    YuvRender* yuv_render;
    Audio* audio;
};

#endif //MEDIASERVER_MEDIADECODER_H