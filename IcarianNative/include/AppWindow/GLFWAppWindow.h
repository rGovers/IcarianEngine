#pragma once

#include "AppWindow/AppWindow.h"

#include <vulkan/vulkan.h>
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

    vk::SurfaceKHR m_surface;
    
protected:

public:
    GLFWAppWindow(Application* a_app, Config* a_config);
    virtual ~GLFWAppWindow();

    virtual bool ShouldClose() const;

    virtual double GetDelta() const;
    virtual double GetTime() const;

    virtual void SetCursorState(FlareBase::e_CursorState a_state);

    virtual void Update();

    virtual glm::ivec2 GetSize() const;
    virtual void Resize(uint32_t a_width, uint32_t a_height);
    virtual void SetFullscreen(const AppMonitor& a_monitor, bool a_state, uint32_t a_width, uint32_t a_height);

    virtual bool IsHeadless() const
    {
        return false;
    }

    virtual AppMonitor* GetMonitors(int* a_count) const;

    virtual std::vector<const char*> GetRequiredVulkanExtenions() const;
    virtual vk::SurfaceKHR GetSurface(const vk::Instance& a_instance);
};