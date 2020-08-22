//
// Created by Mika Goetze on 2019-08-02.
//

#include <Core/Components/TransformComponent.h>
#include "Renderer.h"
#include "MeshComponent.h"
#include <Core/World.h>

Renderer::~Renderer()
= default;

Renderer::Renderer(Window *window)
{
    this->window = window;
}

void Renderer::FrameTick(World &world)
{
    StartFrame();

    auto group = world.Registry()->group<MeshComponent>(entt::get<TransformComponent>);

    for(auto entity : group)
    {
        MeshComponent& meshComponent = group.get<MeshComponent>(entity);
        RenderMesh(*meshComponent.mesh);
    }

    EndFrame();
}
