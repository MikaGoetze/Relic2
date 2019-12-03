//
// Created by Mika Goetze on 2019-07-31.
//

#include <Debugging/Logger.h>

#include <utility>
#include "Window.h"

const int &Window::GetWindowWidth() const
{
    return windowWidth;
}

const int &Window::GetWindowHeight() const
{
    return windowHeight;
}

const std::string &Window::GetWindowTitle() const
{
    return windowTitle;
}

GLFWwindow *Window::GetInternalWindow() const
{
    return window;
}

bool Window::ShouldClose() const
{
    return glfwWindowShouldClose(window);
}

Window::Window(int width, int height, std::string title, bool windowed, int monitor)
{
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    windowWidth = width;
    windowHeight = height;
    windowTitle = std::move(title);

    if (windowed)
    {
        window = glfwCreateWindow(width, height, windowTitle.c_str(), nullptr, nullptr);
    } else
    {
        int monitorCount;
        GLFWmonitor **monitors = glfwGetMonitors(&monitorCount);

        if (monitor >= monitorCount)
        {
            Logger::Log(1, "[Window] Monitor index out of range. Using default monitor.");
            monitor = 0;
        }

        window = glfwCreateWindow(width, height, windowTitle.c_str(), monitors[monitor], nullptr);
    }

    glfwMakeContextCurrent(window);
}

Window::Window(std::string title, int monitor)
{
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    windowTitle = std::move(title);

    int monitorCount;
    GLFWmonitor **monitors = glfwGetMonitors(&monitorCount);

    if (monitor >= monitorCount)
    {
        Logger::Log(1, "[Window] Monitor index out of range : ", std::to_string(monitor).data(),
                    ". Using default monitor.");
        monitor = 0;
    }

    const GLFWvidmode *videoMode = glfwGetVideoMode(monitors[monitor]);
    windowHeight = videoMode->height;
    windowWidth = videoMode->width;

    window = glfwCreateWindow(windowWidth, windowHeight, windowTitle.c_str(), monitors[monitor], nullptr);

    glfwMakeContextCurrent(window);
}

void Window::SetWindowTitle(std::string title)
{
    windowTitle = std::move(title);
}

void Window::SetWindowWidth(int width)
{
    windowWidth = width;
}

void Window::SetWindowHeight(int height)
{
    windowHeight = height;
}

Window::~Window()
{
    glfwDestroyWindow(window);
}
