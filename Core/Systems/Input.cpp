//
// Created by mikag on 9/09/2020.
//

#include <Core/Components/SingletonInput.h>
#include "Input.h"
#include <Core/World.h>
#include <GLFW/glfw3.h>
#include <Core/Relic.h>

void Input::Tick(World &world)
{
}

void Input::FrameTick(World &world)
{
    auto& input = *world.Registry()->ctx<SingletonInput*>();
    auto window = Relic::Instance()->GetActiveWindow()->GetInternalWindow();

    for(uint16_t key = KEY_SPACE; key < KEY_MENU; key++)
    {
        try {
            input.lastKeys.at(key) = input.keys.at(key);
        }
        catch (std::out_of_range& exception)
        {
            input.lastKeys.insert(std::pair<unsigned, bool>(key, false));
        }

        try
        {
            input.keys.at(key) = glfwGetKey(window, key);
        }
        catch (std::out_of_range& exception)
        {
            input.keys.insert(std::pair<unsigned, bool>(key, glfwGetKey(window, key)));
        }
    }

    for(uint8_t button = 0; button < MOUSE_BUTTON_8; button++)
    {
        try
        {
            input.lastMouse.at(button) = input.mouse.at(button);
        }
        catch (std::out_of_range& exception)
        {
            input.lastMouse.insert(std::pair<unsigned, bool>(button, false));
        }

        try
        {
            input.mouse.at(button) = glfwGetMouseButton(window, button);
        }
        catch (std::out_of_range& exception)
        {
            input.mouse.insert(std::pair<unsigned, bool>(button, glfwGetMouseButton(window, button)));
        }
    }
}

void Input::Init(World &world)
{
    NeedsTick = false;
    NeedsFrameTick = true;

    auto registry = world.Registry();
    auto entity = registry->create();

    auto &component = registry->emplace<SingletonInput>(entity);

    //Setup our context variables to make them easy to access.
    registry->set<SingletonInput*>(&component);

    GLFWwindow* window = Relic::Instance()->GetActiveWindow()->GetInternalWindow();
    glfwSetCursorPosCallback(window, OnMouseMoved);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void Input::Shutdown(World &world)
{

}

void Input::OnMouseMoved(GLFWwindow *window, double x, double y)
{
    auto& input = Relic::Instance()->GetPrimaryWorld()->Registry()->ctx<SingletonInput*>();
    input->mousePosX = (float) x;
    input->mousePosY = (float) y;
}

SystemRegistrar Input::registrar(new Input());
