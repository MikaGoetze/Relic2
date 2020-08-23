//
// Created by mikag on 18/08/2020.
//

#include <GLFW/glfw3.h>
#include <memory.h>
#include "Time.h"

float Time::CurrentTime()
{
    return currentTime;
}

float Time::FrameDelta()
{
    return currentTime - lastFrame;
}

void Time::FrameTick()
{
    lastFrame = currentTime;
    currentTime = glfwGetTime();

    float currentDelta = FrameDelta();
    averageTime = (0.05f * currentDelta) + (0.95f * averageTime);

    if(currentFrameIndex < FRAME_COUNT)
    {
        frameTimes[currentFrameIndex] = currentDelta;
        currentFrameIndex++;
    }
    else
    {
        memcpy(frameTimes, frameTimes + 1, sizeof(float) * FRAME_COUNT - 1);
        frameTimes[currentFrameIndex - 1] = currentDelta;
    }
}

float Time::TickDelta()
{
    return currentTime - lastTick;
}

void Time::Tick()
{
    lastTick = currentTime;
}


float Time::frameTimes[FRAME_COUNT];
size_t Time::currentFrameIndex = 0;
float Time::lastFrame;
float Time::currentTime;
float Time::lastTick;
float Time::averageTime;
