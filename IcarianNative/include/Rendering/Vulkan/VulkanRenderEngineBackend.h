// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/IcarianVulkanHeader.h"

#include "DataTypes/TArray.h"
#include "DataTypes/TLockObj.h"
#include "DataTypes/SpinLock.h"
#include "Rendering/RenderEngineBackend.h"
#include "Rendering/Vulkan/VulkanCommandBuffer.h"

class AppWindow;
class LibVulkan;
class VulkanComputeEngine;
class VulkanGraphicsEngine;
class VulkanPushPool;
class VulkanSwapchain;

struct VulkanVideoDecodeCapabilities
{
    vk::VideoProfileInfoKHR VideoProfile;
    vk::VideoDecodeCapabilitiesKHR DecodeCapabilities;
    vk::VideoCapabilitiesKHR VideoCapabilities;
    vk::VideoDecodeH264CapabilitiesKHR DecodeH264Capabilities;
};

class VulkanDeletionObject
{
private:

protected:

public:
    virtual ~VulkanDeletionObject() { }

    virtual void Destroy() = 0;
};

class VulkanRenderEngineBackend : public RenderEngineBackend
{
private:
    constexpr static vk::VideoDecodeH264ProfileInfoKHR DecodeProfile = vk::VideoDecodeH264ProfileInfoKHR
    (
        STD_VIDEO_H264_PROFILE_IDC_HIGH,
        vk::VideoDecodeH264PictureLayoutFlagBitsKHR::eInterlacedInterleavedLines
    );

    LibVulkan*                    m_vulkanLib;

    VulkanComputeEngine*          m_computeEngine;
    VulkanGraphicsEngine*         m_graphicsEngine;
    VulkanSwapchain*              m_swapchain = nullptr;
    VulkanPushPool*               m_pushPool;

    Array<bool>                   m_optionalExtensionMask;
                
    VmaAllocator                  m_allocator;
                
    vk::Instance                  m_instance;
    vk::DebugUtilsMessengerEXT    m_messenger;

    vk::PhysicalDevice            m_pDevice;
    vk::Device                    m_lDevice;
                        
    vk::Queue                     m_computeQueue = nullptr;
    vk::Queue                     m_videoDecodeQueue = nullptr;
    vk::Queue                     m_graphicsQueue = nullptr;
    vk::Queue                     m_presentQueue = nullptr;

    TArray<VulkanDeletionObject*> m_deletionObjects[VulkanDeletionQueueSize];

    Array<vk::Semaphore>          m_interSemaphore[VulkanMaxFlightFrames];
    vk::Semaphore                 m_imageAvailable[VulkanMaxFlightFrames];
    vk::Fence                     m_inFlight[VulkanMaxFlightFrames];
            
    vk::CommandPool               m_commandPool;

    uint32_t                      m_imageIndex = -1;
    uint32_t                      m_currentFrame = 0;
    uint32_t                      m_currentFlightFrame = 0;
    uint32_t                      m_dQueueIndex = 0;

    uint32_t                      m_computeQueueIndex = -1;
    uint32_t                      m_videoDecodeQueueIndex = -1;
    uint32_t                      m_graphicsQueueIndex = -1;
    uint32_t                      m_presentQueueIndex = -1;

    VulkanVideoDecodeCapabilities m_videoDecodeCapabilities;

    SpinLock                      m_graphicsQueueLock;

protected:

public:
    VulkanRenderEngineBackend(RenderEngine* a_engine);
    virtual ~VulkanRenderEngineBackend();

    bool IsExtensionEnabled(const std::string_view& a_extension) const;

    virtual void Update(double a_delta, double a_time);

    TLockObj<vk::CommandBuffer, SpinLock>* CreateCommandBuffer(vk::CommandBufferLevel a_level);
    void DestroyCommandBuffer(TLockObj<vk::CommandBuffer, SpinLock>* a_buffer);

    TLockObj<vk::CommandBuffer, SpinLock>* BeginSingleCommand();
    void EndSingleCommand(TLockObj<vk::CommandBuffer, SpinLock>* a_buffer);

    virtual e_RenderDeviceType GetDeviceType() const;

    virtual uint64_t GetUsedDeviceMemory() const;
    virtual uint64_t GetTotalDeviceMemory() const;

    virtual uint32_t GenerateModel(const void* a_vertices, uint32_t a_vertexCount, uint16_t a_vertexStride, const uint32_t* a_indices, uint32_t a_indexCount, float a_radius);
    virtual void DestroyModel(uint32_t a_addr);

    virtual uint32_t GenerateTexture(uint32_t a_width, uint32_t a_height, e_TextureFormat a_format, const void* a_data);
    virtual uint32_t GenerateTextureMipMapped(uint32_t a_width, uint32_t a_height, uint32_t a_levels, uint64_t* a_offsets, e_TextureFormat a_format, const void* a_data, uint64_t a_dataSize);
    virtual void DestroyTexture(uint32_t a_addr);

    virtual uint32_t GenerateTextureSampler(uint32_t a_textureAddr, e_TextureMode a_textureMode, e_TextureFilter a_filterMode, e_TextureAddress a_addressMode, uint32_t a_slot = 0);
    virtual void DestroyTextureSampler(uint32_t a_addr);
    
    inline VulkanComputeEngine* GetComputeEngine() const
    {
        return m_computeEngine;
    }
    inline VulkanGraphicsEngine* GetGraphicsEngine() const
    {
        return m_graphicsEngine;
    }

    inline VulkanPushPool* GetPushPool() const
    {
        return m_pushPool;
    }

    inline const VulkanVideoDecodeCapabilities* GetVideoDecodeCapabilities() const
    {
        return &m_videoDecodeCapabilities;
    }

    void PushDeletionObject(VulkanDeletionObject* a_object);

    inline vk::Fence GetCurrentFlightFence() const
    {
        return m_inFlight[m_currentFlightFrame];
    }
    inline vk::Semaphore GetImageSemaphore(uint32_t a_index) const
    {
        return m_imageAvailable[a_index];
    }

    inline VmaAllocator GetAllocator() const
    {
        return m_allocator;
    }

    inline vk::Instance GetInstance() const
    {
        return m_instance;
    }

    inline vk::Device GetLogicalDevice() const
    {
        return m_lDevice;
    }
    inline vk::PhysicalDevice GetPhysicalDevice() const
    {
        return m_pDevice;
    }

    inline uint32_t GetPresentQueueIndex() const
    {
        return m_presentQueueIndex;
    }
    inline uint32_t GetComputeQueueIndex() const
    {
        return m_computeQueueIndex;
    }
    inline uint32_t GetVideoDecodeIndex() const
    {
        return m_videoDecodeQueueIndex;
    }
    inline uint32_t GetGraphicsQueueIndex() const
    {
        return m_graphicsQueueIndex;
    }

    inline vk::Queue GetPresentQueue() const
    {
        return m_presentQueue;
    }
    inline vk::Queue GetComputeQueue() const
    {
        return m_computeQueue;
    }
    inline vk::Queue GetVideoDecodeQueue() const
    {
        return m_videoDecodeQueue;
    }
    inline vk::Queue GetGraphicsQueue() const
    {
        return m_graphicsQueue;
    }

    inline uint32_t GetImageIndex() const
    {
        return m_imageIndex;
    }
    inline uint32_t GetCurrentFrame() const
    {
        return m_currentFrame;
    }
    inline uint32_t GetCurrentFlightFrame() const
    {
        return m_currentFlightFrame;
    }
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