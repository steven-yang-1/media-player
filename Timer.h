#ifndef MEDIASERVER_TIMER_H
#define MEDIASERVER_TIMER_H

#include <cstdint>
#include "SDL2/SDL.h"

class Timer {
public:
    Timer();
    void start();
    void pause();
    void unpause();
    uint32_t elapsed_ticks() const;
    ~Timer();

    uint32_t start_ticks;
    uint32_t pause_ticks;
    bool paused {false};
    bool started {false};
};

#endif //MEDIASERVER_TIMER_H
