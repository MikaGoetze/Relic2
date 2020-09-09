//
// Created by mikag on 9/09/2020.
//

#include "FPSCameraSystem.h"
#include <Core/Components/SingletonInput.h>
#include <Core/World.h>
#include <Core/Components/TransformComponent.h>
#include <Gameplay/Components/FPSCameraComponent.h>
#include <Core/Components/SingletonTime.h>

void FPSCameraSystem::Tick(World &world)
{

}

void FPSCameraSystem::FrameTick(World &world)
{
    auto registry = world.Registry();
    auto& input = *registry->ctx<SingletonInput*>();
    auto view = registry->view<FPSCameraComponent, TransformComponent>();
    auto& time = *registry->ctx<SingletonTime*>();

    for(auto entity : view)
    {
        auto &transform = view.get<TransformComponent>(entity);
        auto &camera = view.get<FPSCameraComponent>(entity);

        if(!camera.initialised)
        {
            camera.lastMouseX = input.mousePosX;
            camera.lastMouseY = input.mousePosY;
            camera.initialised = true;
        }

        float xOffset = camera.lastMouseX - input.mousePosX;
        float yOffset = camera.lastMouseY - input.mousePosY;

        camera.lastMouseX = input.mousePosX;
        camera.lastMouseY = input.mousePosY;

        xOffset *= camera.sensitivity;
        yOffset *= -camera.sensitivity;

        camera.yaw += xOffset;
        camera.pitch += yOffset;

        if (camera.pitch > 89) camera.pitch = 89;
        if (camera.pitch < -89) camera.pitch = -89;

        float yaw = glm::radians(camera.yaw);
        float pitch = glm::radians(camera.pitch);

        transform.rotation = glm::quat(glm::vec3(pitch, yaw, 0));

        float speed = camera.moveSpeed;

        if(input.GetKey(KEY_LEFT_SHIFT))
        {
            speed *= 2.0f;
        }

        if(input.GetKey(KEY_A))
        {
            transform.position += transform.rotation * glm::vec3(1,0,0) * speed * time.FrameDelta();
        }
        if(input.GetKey(KEY_D))
        {
            transform.position -= transform.rotation * glm::vec3(1,0,0) * speed * time.FrameDelta();
        }
        if(input.GetKey(KEY_W))
        {
            transform.position += transform.rotation * glm::vec3(0,0,1) * speed * time.FrameDelta();
        }
        if(input.GetKey(KEY_S))
        {
            transform.position -= transform.rotation * glm::vec3(0,0,1) * speed * time.FrameDelta();
        }
        if(input.GetKey(KEY_E))
        {
            transform.position += transform.rotation * glm::vec3(0,1,0) * speed * time.FrameDelta();
        }
        if(input.GetKey(KEY_Q))
        {
            transform.position -= transform.rotation * glm::vec3(0,1,0) * speed * time.FrameDelta();
        }
    }
}

void FPSCameraSystem::Init(World &world)
{
    NeedsFrameTick = true;
    NeedsTick = false;
}

void FPSCameraSystem::Shutdown(World &world)
{

}

SystemRegistrar FPSCameraSystem::registrar(new FPSCameraSystem());
