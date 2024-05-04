#include "YuvRender.h"

YuvRender::YuvRender(int video_width, int video_height) {
    SDL_Rect bounds;
    SDL_GetDisplayUsableBounds(0, &bounds);
    video_width = video_width;
    video_height = video_height;
    if (video_width > bounds.w || video_height > bounds.h) {
        float widthRatio = 1.0 * video_width / bounds.w;
        float heightRatio = 1.0 * video_height / bounds.h;
        float maxRatio = widthRatio > heightRatio ? widthRatio : heightRatio;
        video_width = int(video_width / maxRatio);
        video_height = int(video_height / maxRatio);
    }
    window = SDL_CreateWindow(
            "NetCameraViewer",
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            video_width,
            video_height,
            SDL_WINDOW_OPENGL
    );
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);  // SDL_RENDERER_SOFTWARE for cpu
    texture = SDL_CreateTexture(
            renderer,
            SDL_PIXELFORMAT_IYUV,
            SDL_TEXTUREACCESS_STREAMING,
            video_width,
            video_height
    );
    rect.x = 0;
    rect.y = 0;
    rect.w = video_width;
    rect.h = video_height;
    this->video_width = video_width;
    this->video_height = video_height;
}

int YuvRender::render(AVFrame* frame) {
    if (frame == nullptr) {
        printf("Unavailable video frame.");
        return -1;
    }

    int ret = SDL_UpdateYUVTexture(texture, NULL,
                         frame->data[0], frame->linesize[0],
                         frame->data[1], frame->linesize[1],
                         frame->data[2], frame->linesize[2]);

    if (ret != 0) {
        printf("SDL_UpdateYUVTexture fail.");
    }

    ret = SDL_RenderClear(renderer);
    if (ret != 0) {
        printf("SDL_RenderClear fail.");
    }

    ret = SDL_RenderCopy(renderer, texture, nullptr, &rect);
    if (ret != 0) {
        printf("SDL_RenderCopy fail.");
    }
    SDL_RenderPresent(renderer);

    return 0;
}

YuvRender::~YuvRender() {
    SDL_DestroyWindow(window);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_Quit();
}