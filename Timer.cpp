//
// Created by steve on 2024/5/5.
//

#include "Timer.h"

Timer::Timer() {
}

void Timer::start() {
    started = true;
    start_ticks = SDL_GetTicks();
}

void Timer::pause() {
    if (started && !paused) {
        paused = true;
        pause_ticks = SDL_GetTicks() - start_ticks;
    }
}

void Timer::unpause() {
    if (started && paused) {
        paused = false;
        start_ticks = SDL_GetTicks() - pause_ticks;
        pause_ticks = 0;
    }
}

uint32_t Timer::elapsed_ticks() const {
    return SDL_GetTicks() - start_ticks;
}

Timer::~Timer() {
}