// Icarian Engine - C# Game Engine
// 
// License at end of file.

#include "AppWindow/GLFWAppWindow.h"

#ifdef WIN32
#include "Core/WindowsHeaders.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#define GLFW_NATIVE_INCLUDE_NONE
#include <GLFW/glfw3native.h>
#endif

#include "Application.h"
#include "Config.h"
#include "InputManager.h"
#include "Rendering/UI/UIControl.h"
#include "Profiler.h"

static constexpr int GLFWKeyTable[] =
{
    GLFW_KEY_SPACE,
    GLFW_KEY_APOSTROPHE,
    GLFW_KEY_COMMA,
    GLFW_KEY_MINUS,
    GLFW_KEY_EQUAL,
    GLFW_KEY_PERIOD,
    GLFW_KEY_SLASH,
    GLFW_KEY_BACKSLASH,
    GLFW_KEY_LEFT_BRACKET,
    GLFW_KEY_RIGHT_BRACKET,
    GLFW_KEY_SEMICOLON,
    GLFW_KEY_GRAVE_ACCENT,

    GLFW_KEY_A,
    GLFW_KEY_B,
    GLFW_KEY_C,
    GLFW_KEY_D,
    GLFW_KEY_E,
    GLFW_KEY_F,
    GLFW_KEY_G,
    GLFW_KEY_H,
    GLFW_KEY_I,
    GLFW_KEY_J,
    GLFW_KEY_K,
    GLFW_KEY_L,
    GLFW_KEY_M,
    GLFW_KEY_N,
    GLFW_KEY_O,
    GLFW_KEY_P,
    GLFW_KEY_Q,
    GLFW_KEY_R,
    GLFW_KEY_S,
    GLFW_KEY_T,
    GLFW_KEY_U,
    GLFW_KEY_V,
    GLFW_KEY_W,
    GLFW_KEY_X,
    GLFW_KEY_Y,
    GLFW_KEY_Z,

    GLFW_KEY_0,
    GLFW_KEY_1,
    GLFW_KEY_2,
    GLFW_KEY_3,
    GLFW_KEY_4,
    GLFW_KEY_5,
    GLFW_KEY_6,
    GLFW_KEY_7,
    GLFW_KEY_8,
    GLFW_KEY_9,

    GLFW_KEY_KP_DECIMAL,
    GLFW_KEY_KP_DIVIDE,
    GLFW_KEY_KP_MULTIPLY,
    GLFW_KEY_KP_SUBTRACT,
    GLFW_KEY_KP_ADD,
    GLFW_KEY_KP_EQUAL,
    GLFW_KEY_KP_ENTER,

    GLFW_KEY_KP_0,
    GLFW_KEY_KP_1,
    GLFW_KEY_KP_2,
    GLFW_KEY_KP_3,
    GLFW_KEY_KP_4,
    GLFW_KEY_KP_5,
    GLFW_KEY_KP_6,
    GLFW_KEY_KP_7,
    GLFW_KEY_KP_8,
    GLFW_KEY_KP_9,

    GLFW_KEY_ESCAPE,
    GLFW_KEY_ENTER,
    GLFW_KEY_TAB,
    GLFW_KEY_BACKSPACE,
    GLFW_KEY_INSERT,
    GLFW_KEY_DELETE,
    GLFW_KEY_HOME,
    GLFW_KEY_END,
    GLFW_KEY_PAGE_UP,
    GLFW_KEY_PAGE_DOWN,
    GLFW_KEY_LEFT,
    GLFW_KEY_RIGHT,
    GLFW_KEY_UP,
    GLFW_KEY_DOWN,
    GLFW_KEY_CAPS_LOCK,
    GLFW_KEY_NUM_LOCK,
    GLFW_KEY_SCROLL_LOCK,
    GLFW_KEY_PRINT_SCREEN,
    GLFW_KEY_PAUSE,

    GLFW_KEY_LEFT_SHIFT,
    GLFW_KEY_LEFT_CONTROL,
    GLFW_KEY_LEFT_ALT,
    GLFW_KEY_LEFT_SUPER,

    GLFW_KEY_RIGHT_SHIFT,
    GLFW_KEY_RIGHT_CONTROL,
    GLFW_KEY_RIGHT_ALT,
    GLFW_KEY_RIGHT_SUPER,

    GLFW_KEY_F1, 
    GLFW_KEY_F2, 
    GLFW_KEY_F3, 
    GLFW_KEY_F4, 
    GLFW_KEY_F5, 
    GLFW_KEY_F6, 
    GLFW_KEY_F7, 
    GLFW_KEY_F8, 
    GLFW_KEY_F9, 
    GLFW_KEY_F10,
    GLFW_KEY_F11,
    GLFW_KEY_F12,
    GLFW_KEY_F13,
    GLFW_KEY_F14,
    GLFW_KEY_F15,
    GLFW_KEY_F16,
    GLFW_KEY_F17,
    GLFW_KEY_F18,
    GLFW_KEY_F19,
    GLFW_KEY_F20,
    GLFW_KEY_F21,
    GLFW_KEY_F22,
    GLFW_KEY_F23,
    GLFW_KEY_F24,
    GLFW_KEY_F25,

    GLFW_KEY_MENU
};

GLFWAppWindow::GLFWAppWindow(Application* a_app, Config* a_config) : AppWindow(a_app)
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    // glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    m_window = glfwCreateWindow(1280, 720, a_config->GetApplicationName().data(), NULL, NULL);

    glfwSetWindowSizeLimits(m_window, 640, 480, GLFW_DONT_CARE, GLFW_DONT_CARE);

    glfwMakeContextCurrent(m_window);

    m_time = glfwGetTime();
    m_prevTime = m_time;
    m_startTime = m_time;

    glfwGetCursorPos(m_window, &m_lastCursorPos.x, &m_lastCursorPos.y);

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN
    m_surface = nullptr;
#endif

    m_shouldClose = false;
}
GLFWAppWindow::~GLFWAppWindow()
{
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

bool GLFWAppWindow::ShouldClose() const
{
    return m_shouldClose;
}

double GLFWAppWindow::GetDelta() const
{
    return m_time - m_prevTime;
}
double GLFWAppWindow::GetTime() const
{
    return m_time - m_startTime;
}

void GLFWAppWindow::SetCursorState(e_CursorState a_state)
{
    switch (a_state) 
    {
    case CursorState_Normal:
    {
        glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

        break;
    }
    case CursorState_Hidden:
    {
        glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

        break;
    }
    case CursorState_Locked:
    {
        glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        if (glfwRawMouseMotionSupported())
        {
            glfwSetInputMode(m_window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
        }

        break;
    }
    }
}

void GLFWAppWindow::Update()
{
    glfwPollEvents();

    m_prevTime = m_time;
    m_time = glfwGetTime();

    // Putting a 1KHz limit in place cause dont need to hit 16KHz on a 5950x on Linux
    // Still probably a little high
    // This feels so wrong that this is needed in the standalone app
    while (GetDelta() < 0.001f)
    {
        std::this_thread::yield();

        glfwPollEvents();
        m_time = glfwGetTime();
    }

    m_shouldClose = glfwWindowShouldClose(m_window);

    const Application* app = GetApplication();
    
    {
        PROFILESTACK("Input");
        InputManager* inputManager = app->GetInputManager();

        glm::ivec2 winSize;
        glfwGetWindowSize(m_window, &winSize.x, &winSize.y);

        glm::dvec2 cPos;
        glfwGetCursorPos(m_window, &cPos.x, &cPos.y);

        if (app->GetCursorState() == CursorState_Locked)
        {
            const glm::dvec2 delta = cPos - m_lastCursorPos;

            inputManager->SetCursorPos((glm::vec2)delta);
        }
        else
        {
            inputManager->SetCursorPos((glm::vec2)cPos);
        }

        m_lastCursorPos = cPos;

        UIControl::UpdateCursor((glm::vec2)cPos, (glm::vec2)winSize);
        
        bool leftDown = glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
        if (leftDown)
        {
            if (UIControl::SubmitClick((glm::vec2)cPos, (glm::vec2)winSize))
            {
                leftDown = false;
            }
        }
        else 
        {
            UIControl::SubmitRelease((glm::vec2)cPos, (glm::vec2)winSize);
        }

        inputManager->SetMouseButton(MouseButton_Left, leftDown);
        inputManager->SetMouseButton(MouseButton_Middle, glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS);
        inputManager->SetMouseButton(MouseButton_Right, glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS);

        for (unsigned int i = 0; i < KeyCode_Last; ++i)
        {
            inputManager->SetKeyboardKey((e_KeyCode)i, glfwGetKey(m_window, GLFWKeyTable[i]) == GLFW_PRESS);
        }

        if (inputManager->IsKeyDown(KeyCode_LeftAlt) || inputManager->IsKeyDown(KeyCode_RightAlt))
        {
            if (inputManager->IsKeyDown(KeyCode_F4))
            {
                // Alt+F4 is unreliable under XWayland 
                m_shouldClose = true;
            }
        }
    }  
}

glm::ivec2 GLFWAppWindow::GetSize() const
{
    glm::ivec2 winSize;
    glfwGetWindowSize(m_window, &winSize.x, &winSize.y);

    return winSize;
}
void GLFWAppWindow::Resize(uint32_t a_width, uint32_t a_height)
{
    glfwSetWindowSize(m_window, (int)a_width, (int)a_height);
}
void GLFWAppWindow::SetFullscreen(const AppMonitor& a_monitor, bool a_state, uint32_t a_width, uint32_t a_height)
{
    if (a_state)
    {
        glfwSetWindowMonitor(m_window, (GLFWmonitor*)a_monitor.Handle, 0, 0, a_width, a_height, GLFW_DONT_CARE);
    }
    else
    {
        glfwSetWindowMonitor(m_window, NULL, a_width / 2, a_height / 2, a_width, a_height, GLFW_DONT_CARE);
    }
}

AppMonitor* GLFWAppWindow::GetMonitors(int* a_count) const
{
    AppMonitor* monitors = nullptr;
    *a_count = 0;

    int monitorCount;
    GLFWmonitor** glfwMonitors = glfwGetMonitors(&monitorCount);
    if (monitorCount > 0 && glfwMonitors != nullptr)
    {
        monitors = new AppMonitor[monitorCount];
        *a_count = monitorCount;

        for (int i = 0; i < monitorCount; ++i)
        {
            GLFWmonitor* mon = glfwMonitors[i];

            monitors[i].Name = glfwGetMonitorName(mon);
            monitors[i].Width = 0;
            monitors[i].Height = 0;
            monitors[i].Handle = mon;
            
            int vidModeCount;
            const GLFWvidmode* vidModes = glfwGetVideoModes(mon, &vidModeCount);
            for (int j = 0; j < vidModeCount; ++j)
            {
                monitors[i].Width = glm::max(monitors[i].Width, (uint32_t)vidModes[j].width);
                monitors[i].Height = glm::max(monitors[i].Height, (uint32_t)vidModes[j].height);
            }
        }
    }

    return monitors;
}

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

vk::SurfaceKHR GLFWAppWindow::GetSurface(const vk::Instance& a_instance) 
{
    if (m_surface == vk::SurfaceKHR(nullptr))
    {
        VkSurfaceKHR tempSurf;
        glfwCreateWindowSurface(a_instance, m_window, nullptr, &tempSurf);
        m_surface = tempSurf;
    }
    
    return m_surface;
}
Array<const char*> GLFWAppWindow::GetRequiredVulkanExtenions() const
{
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    return Array<const char*>(glfwExtensions, glfwExtensionCount);
}

#endif

// MIT License
// 
// Copyright (c) 2024 River Govers
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.