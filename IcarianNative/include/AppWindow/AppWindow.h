// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include <cstdint>
#include <string>

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN
#include "Rendering/Vulkan/IcarianVulkanHeader.h"
#endif

#include "DataTypes/Array.h"

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
    virtual Array<const char*> GetRequiredVulkanExtenions() const = 0;
    virtual vk::SurfaceKHR GetSurface(const vk::Instance& a_instance) = 0;
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