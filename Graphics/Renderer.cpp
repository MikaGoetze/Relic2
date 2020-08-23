//
// Created by Mika Goetze on 2019-08-02.
//

#include <Core/Components/TransformComponent.h>
#include "Renderer.h"
#include "MeshComponent.h"
#include "CameraComponent.h"
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
            RenderMesh(*meshComponent.mesh, transformComponent);
        }
        break;
    }



    EndFrame();
}
