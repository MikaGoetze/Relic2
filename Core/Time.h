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

    static float currentFrame;
    static float lastFrame;
    static float averageTime;

    static float frameTimes[FRAME_COUNT];
    static size_t currentFrameIndex;

    friend class Relic;

    static void Update();

public:
    static float CurrentTime();
    static float DeltaTime();
};


#endif //RELIC_TIME_H
