//
// Created by Mika Goetze on 2019-07-31.
//

#ifndef RELIC_RENDERER_H
#define RELIC_RENDERER_H

#include <Core/ISystem.h>
#include "Graphics/Window.h"
#include "Graphics/Model.h"
#include <Core/Components/TransformComponent.h>
#include <Graphics/Components/SingletonRenderState.h>

/// Interface for creating render back ends.
class Renderer : public ISystem
{
private:
    void OnMeshComponentConstruction(entt::registry& registry, entt::entity);
    void OnMeshComponentDestruction(entt::registry& registry, entt::entity);
public:
    explicit Renderer();

    virtual ~Renderer() = 0;

    virtual void StartFrame(SingletonRenderState &state) = 0;
    virtual void RenderMesh(SingletonRenderState &state, Mesh &mesh, Material &material, TransformComponent component) = 0;
    virtual void EndFrame(SingletonRenderState &state) = 0;

    virtual void PrepareMesh(SingletonRenderState &state, Mesh &mesh) = 0;
    virtual void CleanupMesh(SingletonRenderState &state, Mesh &mesh) = 0;

    virtual void RegisterMaterial(Material *material);

    void Init(World &world) override;

    void FrameTick(World &world) override;

protected:
    Window *window;
    glm::mat4 vpMatrix;

    std::vector<Material*> materials;
};

#endif //RELIC_RENDERER_H
