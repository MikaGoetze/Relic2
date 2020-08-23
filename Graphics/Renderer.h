//
// Created by Mika Goetze on 2019-07-31.
//

#ifndef RELIC_RENDERER_H
#define RELIC_RENDERER_H

#include <Core/ISystem.h>
#include "Window.h"
#include "Model.h"
#include <Core/Components/TransformComponent.h>

/// Interface for creating render back ends.
class Renderer : public ISystem
{
public:
    explicit Renderer(Window *window);

    virtual ~Renderer() = 0;

    virtual void StartFrame() = 0;
    virtual void RenderMesh(Mesh &mesh, TransformComponent component) = 0;
    virtual void EndFrame() = 0;
    virtual void FinishPendingRenderingOperations() = 0;

    virtual void PrepareModel(Model& model) = 0;
    virtual void DestroyModel(Model& model) = 0;

    void FrameTick(World &world) override;

protected:
    Window *window;

    glm::mat4 vpMatrix;
};

#endif //RELIC_RENDERER_H
