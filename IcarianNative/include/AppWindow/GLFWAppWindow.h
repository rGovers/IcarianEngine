// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#include "AppWindow/AppWindow.h"

#include <GLFW/glfw3.h>

class Config;

class GLFWAppWindow : public AppWindow
{
private:
    GLFWwindow*    m_window;
   
    bool           m_shouldClose;

    double         m_time;
    double         m_prevTime;
    double         m_startTime;

    glm::dvec2     m_lastCursorPos;

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN
    vk::SurfaceKHR m_surface;
#endif

protected:

public:
    GLFWAppWindow(Application* a_app, Config* a_config);
    virtual ~GLFWAppWindow();

    virtual bool ShouldClose() const;

    virtual double GetDelta() const;
    virtual double GetTime() const;

    virtual void SetCursorState(e_CursorState a_state);

    virtual void Update();

    virtual glm::ivec2 GetSize() const;
    virtual void Resize(uint32_t a_width, uint32_t a_height);
    virtual void SetFullscreen(const AppMonitor& a_monitor, bool a_state, uint32_t a_width, uint32_t a_height);

    virtual bool IsHeadless() const
    {
        return false;
    }

    virtual AppMonitor* GetMonitors(int* a_count) const;

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN
    virtual Array<const char*> GetRequiredVulkanExtenions() const;
    virtual vk::SurfaceKHR GetSurface(const vk::Instance& a_instance);
#endif
};

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