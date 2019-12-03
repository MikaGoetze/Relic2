//
// Created by Mika Goetze on 2019-08-02.
//

#include "Renderer.h"

Renderer::~Renderer()
= default;

Renderer::Renderer(Window *window)
{
    this->window = window;
}
