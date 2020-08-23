//
// Created by mikag on 18/08/2020.
//

#ifndef RELIC_TIME_H
#define RELIC_TIME_H

#include <chrono>

class Time
{
private:
    static const size_t FRAME_COUNT = 512;

    static float currentTime;
    static float lastFrame;
    static float lastTick;
    static float averageTime;

    static float frameTimes[FRAME_COUNT];
    static size_t currentFrameIndex;

    friend class Relic;

    static void FrameTick();

public:
    static float CurrentTime();
    static float FrameDelta();
    static float TickDelta();

    static void Tick();
};


#endif //RELIC_TIME_H
