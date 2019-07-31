//
// Created by Mika Goetze on 2019-07-31.
//

#ifndef RELIC_WINDOW_H
#define RELIC_WINDOW_H

#include <string>
#include <GLFW/glfw3.h>

class Window
{
public:
    /// Create a new window.
    /// \param width Width of the window (pixels).
    /// \param height Height of the window (pixels).
    /// \param title Title of the window.
    /// \param windowed Should the window be windowed or fullscreen.
    /// \param monitor The index of the monitor on which the window should be created (optional).
    Window(int width, int height, std::string title, bool windowed, int monitor = 0);

    /// Create a new fullscreen window.
    /// \param title Title of the window.
    /// \param monitor The index of the monitor on which the window should be created.
    Window(std::string title, int monitor);

    ~Window();

    /// Set the title of the window
    /// \param title Window Title
    void SetWindowTitle(std::string title);

    /// Set the width of the window
    /// \param width Width of the window (pixels).
    void SetWindowWidth(int width);

    /// Set the height of the window.
    /// \param height Height of the window (pixels).
    void SetWindowHeight(int height);

    /// Get the width of the window.
    /// \return The width of the window (pixels).
    const int &GetWindowWidth() const;

    /// Get the height of the window.
    /// \return The height of the window (pixels).
    const int &GetWindowHeight() const;

    /// Get the title of the window.
    /// \return Title of the window.
    const std::string &GetWindowTitle() const;

    /// Get the internal representation of the window.
    /// \return The GLFW3 representation.
    GLFWwindow *GetInternalWindow() const;

    /// Whether or not the window has requested to be closed.
    /// \return Whether or not it should be closed.
    bool ShouldClose() const;

private:
    int windowWidth;
    int windowHeight;
    std::string windowTitle;
    GLFWwindow *window = nullptr;
};


#endif //RELIC_WINDOW_H
