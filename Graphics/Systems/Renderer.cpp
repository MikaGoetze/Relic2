//
// Created by Mika Goetze on 2019-08-02.
//

#include <Core/Components/TransformComponent.h>
#include "Renderer.h"
#include "Graphics/Components/MeshComponent.h"
#include "Graphics/Components/CameraComponent.h"
#include <Core/World.h>
#include <Core/Relic.h>

Renderer::~Renderer()
= default;

Renderer::Renderer()
{

}

void Renderer::FrameTick(World &world)
{
    SingletonRenderState& state = *world.Registry()->ctx<SingletonRenderState*>();
    StartFrame(state);

    auto objects = world.Registry()->group<MeshComponent>(entt::get<TransformComponent>);
    auto cameras = world.Registry()->group<CameraComponent>(entt::get<TransformComponent>);

    //For now we only render one camera, since the renderer doesn't support writing to textures yet.

    for(auto cameraEntity : cameras)
    {
        CameraComponent& cameraComponent = cameras.get<CameraComponent>(cameraEntity);
        if(!cameraComponent.isActive) continue;

        TransformComponent& cameraTransform = cameras.get<TransformComponent>(cameraEntity);

        glm::vec3 cameraDir = cameraTransform.rotation * glm::vec3(0,0,1);
        glm::vec3 right = glm::normalize(glm::cross(glm::vec3(0, 1, 0), -cameraDir));
        glm::vec3 up = glm::normalize(glm::cross(right, cameraDir));
        glm::mat4 view = glm::lookAt(cameraTransform.position, cameraTransform.position + cameraDir, up);

        glm::mat4 proj = glm::perspective(cameraComponent.fov, (float) window->GetWindowWidth() / (float) window->GetWindowHeight(), cameraComponent.nearPlane, cameraComponent.farPlane);
        proj[1][1] *= -1;

        vpMatrix = proj * view;

        for(auto entity : objects)
        {
            MeshComponent& meshComponent = objects.get<MeshComponent>(entity);
            TransformComponent& transformComponent = objects.get<TransformComponent>(entity);
            RenderMesh(state, *meshComponent.mesh, transformComponent);
        }
        break;
    }

    EndFrame(state);
}

void Renderer::Init(World &world)
{
    NeedsFrameTick = true;
    NeedsTick = false;
    this->window = Relic::Instance()->GetActiveWindow();

    world.Registry()->on_construct<MeshComponent>().connect<&Renderer::OnMeshComponentConstruction>(this);
    world.Registry()->on_destroy<MeshComponent>().connect<&Renderer::OnMeshComponentDestruction>(this);
}

void Renderer::OnMeshComponentConstruction(entt::registry& registry, entt::entity entity)
{
    MeshComponent &comp = registry.get<MeshComponent>(entity);
    SingletonRenderState & state = *registry.ctx<SingletonRenderState*>();
    PrepareMesh(state, *comp.mesh);
}

void Renderer::OnMeshComponentDestruction(entt::registry &registry, entt::entity entity)
{
    MeshComponent &comp = registry.get<MeshComponent>(entity);
    SingletonRenderState & state = *registry.ctx<SingletonRenderState*>();
    CleanupMesh(state, *comp.mesh);
}

