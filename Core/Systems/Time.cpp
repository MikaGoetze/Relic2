//
// Created by mikag on 18/08/2020.
//

#include <GLFW/glfw3.h>
#include <Core/Components/SingletonTime.h>
#include <Core/Components/SingletonFrameStats.h>
#include "Time.h"
#include <Core/World.h>

void Time::Tick(World &world)
{
    auto registry = world.Registry();
    SingletonTime* time = registry->ctx<SingletonTime*>();

    time->lastTickTime = time->currentTime;
}

void Time::FrameTick(World &world)
{
    auto registry = world.Registry();
    SingletonTime* time = registry->ctx<SingletonTime*>();

    time->lastFrameTime = time->currentTime;
    time->currentTime = glfwGetTime();

    SingletonFrameStats** pFrameStats = registry->try_ctx<SingletonFrameStats*>();
    if(pFrameStats != nullptr)
    {
        SingletonFrameStats* frameStats = *pFrameStats;
        if(frameStats->currentFrameIndex >= SingletonFrameStats::FRAME_COUNT)
        {
            memcpy(frameStats->frameTimes, frameStats->frameTimes + 1, (SingletonFrameStats::FRAME_COUNT - 1) * sizeof(float));
            frameStats->frameTimes[SingletonFrameStats::FRAME_COUNT - 1] = time->FrameDelta();
        }
        else
        {
            frameStats->frameTimes[frameStats->currentFrameIndex] = time->FrameDelta();
            frameStats->currentFrameIndex++;
        }

        frameStats->averageFrameTime = (0.1f * time->FrameDelta()) + (0.9f * frameStats->averageFrameTime);
    }
}

void Time::Init(World &world)
{
    NeedsTick = true;
    NeedsFrameTick = true;

    auto registry = world.Registry();
    auto entity = registry->create();

    auto &time = registry->emplace<SingletonTime>(entity);
    auto &frameStats = registry->emplace<SingletonFrameStats>(entity);

    //Setup our context variables to make them easy to access.
    registry->set<SingletonTime*>(&time);
    registry->set<SingletonFrameStats*>(&frameStats);
}

void Time::Shutdown(World &world)
{
    auto registry = world.Registry();
    registry->unset<SingletonTime*>();
    registry->unset<SingletonFrameStats*>();
}
