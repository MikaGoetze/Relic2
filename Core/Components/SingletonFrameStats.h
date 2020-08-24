//
// Created by mikag on 24/08/2020.
//

#ifndef RELIC_SINGLETONFRAMESTATS_H
#define RELIC_SINGLETONFRAMESTATS_H

struct SingletonFrameStats
{
    static const size_t FRAME_COUNT = 512;
    float averageFrameTime;
    float frameTimes[FRAME_COUNT];
    int currentFrameIndex;
};

#endif //RELIC_SINGLETONFRAMESTATS_H
