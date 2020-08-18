//
// Created by mikag on 18/08/2020.
//

#include <GLFW/glfw3.h>
#include <memory.h>
#include "Time.h"

float Time::CurrentTime()
{
    return currentFrame;
}

float Time::DeltaTime()
{
    return currentFrame - lastFrame;
}

void Time::Update()
{
    lastFrame = currentFrame;
    currentFrame = glfwGetTime();

    float currentDelta = DeltaTime();
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

float Time::frameTimes[FRAME_COUNT];
size_t Time::currentFrameIndex = 0;
float Time::lastFrame;
float Time::currentFrame;
float Time::averageTime;
