//
// Created by Mika Goetze on 2019-07-31.
//

#ifndef RELIC_RENDERER_H
#define RELIC_RENDERER_H

#include "Window.h"
#include "Model.h"

/// Interface for creating render back ends.
class Renderer
{
public:
    explicit Renderer(Window *window);

    virtual ~Renderer() = 0;

    virtual void Render() = 0;
    virtual void FinishPendingRenderingOperations() = 0;

    virtual void PrepareModel(Model& model) = 0;
    virtual void DestroyModel(Model& model) = 0;

protected:
    Window *window;
};

#endif //RELIC_RENDERER_H
