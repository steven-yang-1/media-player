#ifndef MEDIASERVER_YUVRENDER_H
#define MEDIASERVER_YUVRENDER_H

#include "SDL2/SDL.h"

extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
}

class YuvRender {
public:
    YuvRender(int video_width, int video_height);
    int render(AVFrame* frame);
    ~YuvRender();
    SDL_Window* window;
private:
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    SDL_Rect rect;
    SDL_Event event;
    int video_height;
    int video_width;
};

#endif //MEDIASERVER_YUVRENDER_H