#pragma once

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN
#include "Rendering/Vulkan/IcarianVulkanHeader.h"
#endif

#include "EngineInputInteropStructures.h"

class Application;

struct AppMonitor
{
    std::string Name;
    uint32_t Width;
    uint32_t Height;
    void* Handle;
};

class AppWindow
{
private:
    Application* m_app;

protected:

public:
    AppWindow(Application* a_app)
    {
        m_app = a_app;
    }
    virtual ~AppWindow() { }

    virtual bool ShouldClose() const = 0;

    virtual double GetDelta() const = 0;
    virtual double GetTime() const = 0;
    
    virtual void SetCursorState(e_CursorState a_state) = 0;

    virtual void Update() = 0;

    virtual void Resize(uint32_t a_width, uint32_t a_height) { }
    virtual void SetFullscreen(const AppMonitor& a_monitor, bool a_state, uint32_t a_width, uint32_t a_height) { }

    inline Application* GetApplication() const
    {
        return m_app;
    }

    virtual glm::ivec2 GetSize() const = 0;

    virtual bool IsHeadless() const = 0;

    virtual AppMonitor* GetMonitors(int* a_count) const { *a_count = 0; return nullptr; }

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN
    virtual std::vector<const char*> GetRequiredVulkanExtenions() const = 0;
    virtual vk::SurfaceKHR GetSurface(const vk::Instance& a_instance) = 0;
#endif
};