// Icarian Engine - C# Game Engine
//
// License at end of file.

#pragma once

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/RenderEngineBackend.h"

#include "Rendering/Vulkan/IcarianVulkanHeader.h"

#include "DataTypes/Allocators/BlockAllocator.h"
#include "DataTypes/SpinLock.h"
#include "DataTypes/TArray.h"
#include "DataTypes/TLockObj.h"

class Allocator;
class AppWindow;
class LibVulkan;
class StackAllocator;
class TrackerAllocator;
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
    static StackAllocator* GetAllocator();

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
    constexpr static uint32_t SmallAllocatorSize = 8 << 10;
    constexpr static uint32_t LargeAllocatorSize = 8 << 20;
    constexpr static uint32_t DeletionAllocatorSize = 2 << 10;
    constexpr static uint64_t ScratchAllocatorSize = 1 << 20;

    constexpr static vk::VideoDecodeH264ProfileInfoKHR DecodeProfile = vk::VideoDecodeH264ProfileInfoKHR
    (
        STD_VIDEO_H264_PROFILE_IDC_HIGH,
        vk::VideoDecodeH264PictureLayoutFlagBitsKHR::eInterlacedInterleavedLines
    );

    // Need to wrap the data in another struct as it depend on an allocator that we create in the constructor
    // All this is to prevent RAII related crashes while still allowing RAII and preventing the use of pointers for manual management
    // Just custom memory allocator things as RAII forces an initializtion order based on the members
    struct ClassData
    {
        LibVulkan*                     VulkanLib;

        VulkanComputeEngine*           ComputeEngine;
        VulkanGraphicsEngine*          GraphicsEngine;
        VulkanSwapchain*               Swapchain;
        VulkanPushPool*                PushPool;

        TArray<RenderScratchAllocator> ScratchAllocators;

        // Was bugging me taking up 8x the memory needed so.... uint8_t bitmask it is
        uint8_t*                       OptionalExtensionMask;

        VmaAllocator                   VMAAllocator;

        vk::Instance                   Instance;
        vk::DebugUtilsMessengerEXT     Messenger;

        vk::PhysicalDevice             PhysicalDevice;
        vk::Device                     LogicalDevice;

        vk::Queue                      ComputeQueue;
        vk::Queue                      VideoDecodeQueue;
        vk::Queue                      GraphicsQueue;
        vk::Queue                      PresentQueue;

        TArray<VulkanDeletionObject*>  DeletionObjects[VulkanDeletionQueueSize];

        Array<vk::Semaphore>           InterSemaphore[VulkanMaxFlightFrames];

        vk::CommandPool                CommandPools[CommandIndex_Last];

        uint32_t                       ScratchIndex;
        uint32_t                       ImageIndex;
        uint32_t                       CurrentFrame;
        uint32_t                       CurrentFlightFrame;
        uint32_t                       DeletionQueueIndex;

        uint32_t                       ComputeQueueIndex;
        uint32_t                       VideoDecodeQueueIndex;
        uint32_t                       GraphicsQueueIndex;
        uint32_t                       PresentQueueIndex;

        VulkanVideoDecodeCapabilities  VideoDecodeCapabilities;
    };

    BlockAllocator*    m_smallAllocator;
    BlockAllocator*    m_largeAllocator;

    TrackerAllocator*  m_trackerAllocator;

    ComplexAllocator*  m_allocator;
    Array<Allocator*>* m_allocatorChain;

    ComplexAllocator*  m_deletionAllocator;

    ClassData*         m_data;

    SharedSpinLock     m_scratchLock;
    SpinLock           m_graphicsQueueLock;

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

    virtual void DMASignal();

    virtual uint32_t GenerateMesh
    (
        const void* a_vertices,
        uint32_t a_vertexCount,
        uint16_t a_vertexStride,
        const uint32_t* a_meshletVertices,
        uint32_t a_meshletVertexCount,
        const uint8_t* a_meshletTriangles,
        uint32_t a_meshletTriangleCount,
        const IcarianCore::ShaderMeshletBuffer* a_meshlets,
        uint32_t a_meshletCount,
        float a_radius
    );
    virtual void DestroyMesh(uint32_t a_addr);

    virtual uint32_t GenerateModel(const void* a_vertices, uint32_t a_vertexCount, uint16_t a_vertexStride, const uint32_t* a_indices, uint32_t a_indexCount, float a_radius);
    virtual void DestroyModel(uint32_t a_addr);

    virtual uint32_t GenerateTexture(uint32_t a_width, uint32_t a_height, e_TextureFormat a_format, const void* a_data);
    virtual uint32_t GenerateTextureMipMapped
    (
        uint32_t a_width,
        uint32_t a_height,
        uint32_t a_levels,
        uint64_t* a_offsets,
        e_TextureFormat a_format,
        const void* a_data,
        uint64_t a_dataSize
    );
    virtual void DestroyTexture(uint32_t a_addr);

    virtual uint32_t GenerateTextureSampler(uint32_t a_textureAddr, e_TextureMode a_textureMode, e_TextureFilter a_filterMode, e_TextureAddress a_addressMode, uint32_t a_slot = 0);
    virtual void DestroyTextureSampler(uint32_t a_addr);

    inline ComplexAllocator* GetAllocator() const
    {
        return m_allocator;
    }
    inline ComplexAllocator* GetDeletionAllocator() const
    {
        return m_deletionAllocator;
    }

    inline VulkanComputeEngine* GetComputeEngine() const
    {
        return m_data->ComputeEngine;
    }
    inline VulkanGraphicsEngine* GetGraphicsEngine() const
    {
        return m_data->GraphicsEngine;
    }

    inline VulkanPushPool* GetPushPool() const
    {
        return m_data->PushPool;
    }

    inline const VulkanVideoDecodeCapabilities* GetVideoDecodeCapabilities() const
    {
        return &m_data->VideoDecodeCapabilities;
    }

    void IncrementScratchFrame(uint32_t a_index);
    void DecrementScratchFrame(uint32_t a_index);

    StackAllocator* GetStackAllocator(uint32_t* a_index = nullptr);

    template<typename T, typename ... Args>
    void PushDeletionObject(Args&&... a_args)
    {
        VulkanDeletionObject* deletionObject = m_deletionAllocator->Create<T>(a_args...);

        InternalPushDeletionObject(deletionObject);
    }

    inline VmaAllocator GetVMAAllocator() const
    {
        return m_data->VMAAllocator;
    }

    inline vk::Instance GetInstance() const
    {
        return m_data->Instance;
    }

    inline vk::Device GetLogicalDevice() const
    {
        return m_data->LogicalDevice;
    }
    inline vk::PhysicalDevice GetPhysicalDevice() const
    {
        return m_data->PhysicalDevice;
    }

    inline uint32_t GetPresentQueueIndex() const
    {
        return m_data->PresentQueueIndex;
    }
    inline uint32_t GetComputeQueueIndex() const
    {
        return m_data->ComputeQueueIndex;
    }
    inline uint32_t GetVideoDecodeIndex() const
    {
        return m_data->VideoDecodeQueueIndex;
    }
    inline uint32_t GetGraphicsQueueIndex() const
    {
        return m_data->GraphicsQueueIndex;
    }

    inline vk::Queue GetPresentQueue() const
    {
        return m_data->PresentQueue;
    }
    inline vk::Queue GetComputeQueue() const
    {
        return m_data->ComputeQueue;
    }
    inline vk::Queue GetVideoDecodeQueue() const
    {
        return m_data->VideoDecodeQueue;
    }
    inline vk::Queue GetGraphicsQueue() const
    {
        return m_data->GraphicsQueue;
    }

    inline uint32_t GetImageIndex() const
    {
        return m_data->ImageIndex;
    }
    inline uint32_t GetCurrentFrame() const
    {
        return m_data->CurrentFrame;
    }
    inline uint32_t GetCurrentFlightFrame() const
    {
        return m_data->CurrentFlightFrame;
    }

    inline bool IsMeshEnabled() const
    {
        return !VulkanForceMeshEmulation && IsExtensionEnabled(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    }

    inline bool IsVideoEnabled() const
    {
        return IsExtensionEnabled(VK_KHR_VIDEO_DECODE_H264_EXTENSION_NAME) && IsExtensionEnabled(VK_KHR_VIDEO_MAINTENANCE_1_EXTENSION_NAME);
    }
};

#endif

// MIT License
//
// Copyright (c) 2026 River Govers
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
