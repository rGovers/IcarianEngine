// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/IcarianVulkanHeader.h"

#include "DataTypes/BlockAllocator.h"
#include "DataTypes/SpinLock.h"
#include "DataTypes/StackAllocator.h"
#include "DataTypes/TArray.h"
#include "DataTypes/TLockObj.h"
#include "Rendering/RenderEngineBackend.h"

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

// Wrapper to use scratch allocator with engine types
struct RenderScratchAlloc
{
    static void* Allocate(uint64_t a_value, uint64_t a_alignment);
    static void Free(void* a_ptr);

    template<typename T>
    static T* TAllocate()
    {
        return (T*)Allocate(sizeof(T), alignof(T));
    }
    template<typename T>
    static T* TAllocate(uint32_t a_count)
    {
        return (T*)Allocate(sizeof(T) * a_count, alignof(T));
    }

    static void PushFrame();
    static void PopFrame();
};

struct RenderBlockAlloc
{
    static void* Allocate(uint64_t a_value, uint32_t a_alignment);
    static void Free(void* a_ptr);
};

#define RENDERSCRATCHFRAME RenderScratchAlloc::PushFrame(); IDEFER(RenderScratchAlloc::PopFrame())

// Wrapper to use STL types with the scratch allocator
template<typename T>
struct STLRenderScratchAlloc
{
public:
    typedef uint64_t size_type;
    typedef uint64_t difference_type;
    typedef T* pointer;
    typedef const T* const_pointer;
    typedef T& reference;
    typedef const T& const_reference;
    typedef T value_type;

    pointer allocate(size_type a_n, const void* a_hint = 0)
    {
        return (pointer)RenderScratchAlloc::TAllocate<value_type>(a_n);
    }
    void deallocate(pointer a_p, size_type a_n)
    {
        
    }

    void construct(pointer a_p, const_reference a_val)
    {
        new (a_p) value_type(a_val);
    }
    template<typename U, typename ... Args>
    void construct(U* a_p, Args&&... a_args)
    {
        new (a_p) U(std::forward<Args>(a_args)...);
    }
    void destroy(pointer p)
    {
        p->~value_type();
    }

    STLRenderScratchAlloc()
    {

    }
    STLRenderScratchAlloc(const STLRenderScratchAlloc& a_other) noexcept
    {

    }
    template<typename U>
    STLRenderScratchAlloc(const STLRenderScratchAlloc<U>& a_other) noexcept
    {
        
    }
};

template<typename T>
struct STLRenderBlockAlloc
{
public:
    typedef uint64_t size_type;
    typedef uint64_t difference_type;
    typedef T* pointer;
    typedef const T* const_pointer;
    typedef T& reference;
    typedef const T& const_reference;
    typedef T value_type;

    pointer allocate(size_type a_n, const void* a_hint = 0)
    {
        return (pointer)RenderBlockAlloc::Allocate(a_n * sizeof(value_type), alignof(value_type));
    }
    void deallocate(pointer a_p, size_type a_n)
    {
        RenderBlockAlloc::Free(a_p);
    }

    void construct(pointer a_p, const_reference a_val)
    {
        new (a_p) value_type(a_val);
    }
    template<typename U, typename ... Args>
    void construct(U* a_p, Args&&... a_args)
    {
        new (a_p) U(std::forward<Args>(a_args)...);
    }
    void destroy(pointer p)
    {
        p->~value_type();
    }

    STLRenderBlockAlloc()
    {

    }
    STLRenderBlockAlloc(const STLRenderBlockAlloc& a_other) noexcept
    {

    }
    template<typename U>
    STLRenderBlockAlloc(const STLRenderBlockAlloc<U>& a_other) noexcept
    {
        
    }
};

struct RenderScratchAllocator
{
    volatile uint32_t Count;
    StackAllocator* Allocator;
};

class VulkanDeletionObject
{
private:

protected:

public:
    virtual ~VulkanDeletionObject() { }

    virtual void Destroy() = 0;
};

enum e_CommandIndex
{
    CommandIndex_Present,
    CommandIndex_Graphics,
    CommandIndex_Compute,
    CommandIndex_VideoDecode,
    CommandIndex_Last
};

class VulkanRenderEngineBackend : public RenderEngineBackend
{
private:
    // Doing 1MB need to investigate later
    constexpr static uint64_t ScratchAllocatorSize = 1 << 20;

    constexpr static vk::VideoDecodeH264ProfileInfoKHR DecodeProfile = vk::VideoDecodeH264ProfileInfoKHR
    (
        STD_VIDEO_H264_PROFILE_IDC_HIGH,
        vk::VideoDecodeH264PictureLayoutFlagBitsKHR::eInterlacedInterleavedLines
    );

    LibVulkan*                    m_vulkanLib;

    BlockAllocator*               m_blockAllocator;
    BlockAllocator*               m_deletionAllocator;

    VulkanComputeEngine*          m_computeEngine;
    VulkanGraphicsEngine*         m_graphicsEngine;
    VulkanSwapchain*              m_swapchain = nullptr;
    VulkanPushPool*               m_pushPool;

    uint32_t                      m_scratchIndex;
    Array<RenderScratchAllocator> m_scratchAllocators;

    // Was bugging me taking up 8x the memory needed so.... uint8_t bitmask it is
    Array<uint8_t>                m_optionalExtensionMask;
                
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
            
    vk::CommandPool               m_commandPools[CommandIndex_Last];

    uint32_t                      m_imageIndex = -1;
    uint32_t                      m_currentFrame = 0;
    uint32_t                      m_currentFlightFrame = 0;
    uint32_t                      m_dQueueIndex = 0;

    uint32_t                      m_computeQueueIndex = -1;
    uint32_t                      m_videoDecodeQueueIndex = -1;
    uint32_t                      m_graphicsQueueIndex = -1;
    uint32_t                      m_presentQueueIndex = -1;

    VulkanVideoDecodeCapabilities m_videoDecodeCapabilities;

    SharedSpinLock                m_scratchLock;
    SpinLock                      m_graphicsQueueLock;

    void InternalPushDeletionObject(VulkanDeletionObject* a_object);

protected:

public:
    VulkanRenderEngineBackend(RenderEngine* a_engine);
    virtual ~VulkanRenderEngineBackend();

    bool IsExtensionEnabled(const std::string_view& a_extension) const;

    virtual void Update(double a_delta, double a_time);

    TLockObj<vk::CommandBuffer, SpinLock>* CreateCommandBuffer(vk::CommandBufferLevel a_level, e_CommandIndex a_index = CommandIndex_Graphics);
    void DestroyCommandBuffer(TLockObj<vk::CommandBuffer, SpinLock>* a_buffer, e_CommandIndex a_index = CommandIndex_Graphics);

    TLockObj<vk::CommandBuffer, SpinLock>* BeginSingleCommand(e_CommandIndex a_index = CommandIndex_Graphics);
    void EndSingleCommand(TLockObj<vk::CommandBuffer, SpinLock>* a_buffer, e_CommandIndex a_index = CommandIndex_Graphics);

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

    inline BlockAllocator* GetBlockAllocator() const
    {
        return m_blockAllocator;
    }
    inline BlockAllocator* GetDeletionAllocator() const
    {
        return m_deletionAllocator;
    }

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

    inline void IncrementScratchFrame(uint32_t a_index)
    {
        if (m_scratchAllocators[a_index].Count == 0)
        {
            const SharedThreadGuard g = SharedThreadGuard(m_scratchLock);

            ++m_scratchAllocators[a_index].Count;

            return;
        }

        ++m_scratchAllocators[a_index].Count;
    }
    inline void DecrementScratchFrame(uint32_t a_index)
    {
        --m_scratchAllocators[a_index].Count;
    }

    StackAllocator* GetStackAllocator(uint32_t* a_index = nullptr);

    template<typename T, typename ... Args>
    void PushDeletionObject(Args&&... a_args)
    {
        VulkanDeletionObject* deletionObject = m_deletionAllocator->Create<T>(a_args...);

        InternalPushDeletionObject(deletionObject);
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

    inline bool IsVideoEnabled() const
    {
        return IsExtensionEnabled(VK_KHR_VIDEO_DECODE_H264_EXTENSION_NAME) && IsExtensionEnabled(VK_KHR_VIDEO_MAINTENANCE_1_EXTENSION_NAME);
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