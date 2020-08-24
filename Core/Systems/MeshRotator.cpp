//
// Created by mikag on 23/08/2020.
//

#include "MeshRotator.h"
#include <Core/World.h>
#include <Graphics/Components/MeshComponent.h>
#include <Core/Components/TransformComponent.h>
#include <Core/Components/SingletonTime.h>

void MeshRotator::Tick(World &world)
{
}

void MeshRotator::FrameTick(World &world)
{
    auto registry = world.Registry();
    auto view = registry->view<const MeshComponent, TransformComponent>();
    auto time = registry->ctx<SingletonTime*>();

    for(auto entity : view)
    {
        auto &transform = view.get<TransformComponent>(entity);
        transform.rotation = glm::rotate(transform.rotation, glm::radians(45.0f) * time->FrameDelta() , glm::vec3(0,0,1));
    }
}

MeshRotator::MeshRotator()
{
    NeedsTick = false;
    NeedsFrameTick = true;
}

SystemRegistrar MeshRotator::registrar(new MeshRotator());

void MeshRotator::Init(World &world)
{

}

void MeshRotator::Shutdown(World &world)
{

}
