#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "ModManager.h"

class Application
{
private:
    GLFWwindow* m_window;

    ModManager  m_modManager;

protected:

public:
    Application();
    ~Application();

    void Run();
};