// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include "Rendering/Vulkan/IcarianVulkanHeader.h"

class AppWindow;
class RuntimeFunction;
class RuntimeManager;
class VulkanRenderEngineBackend;
class VulkanRenderPass;

struct SwapChainSupportInfo
{
    vk::SurfaceCapabilitiesKHR Capabilites;
    std::vector<vk::SurfaceFormatKHR> Formats;
    std::vector<vk::PresentModeKHR> PresentModes;
};

class VulkanSwapchain
{
private:
    AppWindow*                   m_window;
    VulkanRenderEngineBackend*   m_engine;

    RuntimeFunction*             m_resizeFunc;

    unsigned char                m_init;
    std::vector<vk::Image>       m_colorImage;
    VmaAllocation                m_colorAllocation[VulkanMaxFlightFrames];

    vk::Buffer                   m_buffer;
    VmaAllocation                m_allocBuffer;

    vk::SwapchainKHR             m_swapchain = nullptr;
    vk::RenderPass               m_renderPass = nullptr;
    vk::RenderPass               m_renderPassNoClear = nullptr;
    std::vector<vk::ImageView>   m_imageViews;
    std::vector<vk::Framebuffer> m_framebuffers;
      
    vk::SurfaceFormatKHR         m_surfaceFormat;

    glm::ivec2                   m_size;

    void Init(const glm::ivec2& a_size);
    void InitHeadless(const glm::ivec2& a_size);
    void Destroy();
    
protected:

public:
    VulkanSwapchain(VulkanRenderEngineBackend* a_engine, AppWindow* a_window);
    ~VulkanSwapchain();

    static SwapChainSupportInfo QuerySwapChainSupport(const vk::PhysicalDevice& a_device, const vk::SurfaceKHR& a_surface);

    inline vk::SurfaceFormatKHR GetSurfaceFormat() const
    {
        return m_surfaceFormat;
    }

    inline glm::ivec2 GetSize() const
    {
        return m_size;
    }

    inline vk::RenderPass GetRenderPass() const
    {
        return m_renderPass;
    }
    inline vk::RenderPass GetRenderPassNoClear() const
    {
        return m_renderPassNoClear;
    }

    inline vk::Framebuffer GetFramebuffer(uint32_t a_index) const
    {
        return m_framebuffers[a_index];
    }
    inline vk::SwapchainKHR GetSwapchain() const
    {
        return m_swapchain;
    }

    vk::Image GetTexture() const;
    vk::ImageLayout GetImageLayout() const;

    inline bool IsInitialized(uint32_t a_index) const
    {
        return (m_init & 0b1 << a_index) != 0;
    }

    bool StartFrame(uint32_t* a_imageIndex, double a_delta, double a_time);
    void EndFrame(const vk::Semaphore& a_semaphores, uint32_t a_imageIndex);
};

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