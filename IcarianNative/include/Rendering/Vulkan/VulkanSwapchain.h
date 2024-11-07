// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include "Core/Bitfield.h"
#include "DataTypes/Array.h"
#include "Rendering/Vulkan/IcarianVulkanHeader.h"

class AppWindow;
class RuntimeFunction;
class RuntimeManager;
class VulkanRenderEngineBackend;
class VulkanRenderPass;

struct SwapChainSupportInfo
{
    vk::SurfaceCapabilitiesKHR Capabilites;
    Array<vk::SurfaceFormatKHR> Formats;
    Array<vk::PresentModeKHR> PresentModes;
};

struct VulkanSwapchainImage
{
    vk::Image Image;
    VmaAllocation Allocation;
    vk::ImageView View;
    vk::Framebuffer Framebuffer;
#ifdef ICARIANNATIVE_ENABLE_DMA
#ifndef WIN32
    int FD;
#endif
#endif
};

// You may be wondering how did you get DMA based swapchains the answer shamelessly copy code from GPU drivers and desktop compositors
// Vulkan is the more strigent of the 2 between OpenGL and Vulkan so it is better to initialize the DMA buffer on the Vulkan side
// Not even sure if you can create it on the OpenGL side
class VulkanSwapchain
{
private:
    constexpr static bool ForceHeadless = false;

    constexpr static uint32_t HeadlessBlockSize = 128 << 20;

    AppWindow*                  m_window;
    VulkanRenderEngineBackend*  m_engine;

    RuntimeFunction*            m_resizeFunc;

    Array<VulkanSwapchainImage> m_images;

    vk::Semaphore               m_startSemaphores[VulkanMaxFlightFrames];
    vk::Semaphore               m_endSemaphores[VulkanMaxFlightFrames];
    vk::Fence                   m_fences[VulkanMaxFlightFrames];

#ifdef ICARIANNATIVE_ENABLE_DMA
    int                         m_startSemaphoreFD[VulkanMaxFlightFrames];
    int                         m_endSemaphoreFD[VulkanMaxFlightFrames];

    VmaPool                     m_pool;
    // Needs to remain valid for the pool hence in the Swapchain
    VkExportMemoryAllocateInfo  m_exportInfo;
#else
    vk::Buffer                  m_buffer;
    VmaAllocation               m_allocBuffer;

    unsigned char               m_init;
#endif

    vk::SwapchainKHR            m_swapchain;
    vk::RenderPass              m_renderPass;
    vk::RenderPass              m_renderPassNoClear;
      
    vk::SurfaceFormatKHR        m_surfaceFormat;

    uint32_t                    m_width;
    uint32_t                    m_height;

    void Init(uint32_t a_width, uint32_t a_height);
    void InitHeadless(uint32_t a_width, uint32_t a_height);
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

    inline uint32_t GetWidth() const
    {
        return m_width;
    }
    inline uint32_t GetHeight() const
    {
        return m_height;
    }

    inline vk::RenderPass GetRenderPass() const
    {
        return m_renderPass;
    }
    inline vk::RenderPass GetRenderPassNoClear() const
    {
        return m_renderPassNoClear;
    }

    inline vk::Fence GetFence(uint32_t a_index) const
    {
        return m_fences[a_index];
    }
    inline vk::Semaphore GetEndSemaphore(uint32_t a_index) const
    {
        return m_endSemaphores[a_index];
    }

    inline vk::Framebuffer GetFramebuffer(uint32_t a_index) const
    {
        return m_images[a_index].Framebuffer;
    }
    inline vk::SwapchainKHR GetSwapchain() const
    {
        return m_swapchain;
    }

    vk::Image GetTexture() const;
    vk::ImageLayout GetImageLayout() const;

    inline bool IsInitialized(uint32_t a_index) const
    {
#ifdef ICARIANNATIVE_ENABLE_DMA
        return true;
#else
        return IISBITSET(m_init, a_index);
#endif
    }

    bool StartFrame(uint32_t* a_imageIndex, vk::Semaphore* a_semaphore, double a_delta, double a_time);
    void EndFrame(uint32_t a_imageIndex);
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