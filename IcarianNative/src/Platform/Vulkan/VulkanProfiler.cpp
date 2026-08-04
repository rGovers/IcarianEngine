// Icarian Engine - C# Game Engine
//
// License at end of file.

#include "Rendering/Vulkan/VulkanProfiler.h"

#include "DataTypes/Allocators/StackAllocator.h"
#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"
#include "Profiler.h"

static VulkanProfiler* Instance = nullptr;

VulkanProfiler::VulkanProfiler(VulkanRenderEngineBackend* a_engine)
{
    m_engine = a_engine;

    m_queryIndex = 0;
    m_poolIndex = 0;

    m_init = false;

    ComplexAllocator* allocator = m_engine->GetAllocator();
    IVERIFY(allocator != nullptr);

    m_pools = allocator->ZTAllocate<Array<vk::QueryPool>>(VulkanFlightPoolSize);
    for (uint32_t i = 0; i < VulkanFlightPoolSize; ++i)
    {
        m_pools[i] = Array<vk::QueryPool>(allocator);
    }

    const vk::Device device = m_engine->GetLogicalDevice();

    const uint32_t graphicsQueueIndex = m_engine->GetGraphicsQueueIndex();

    const vk::CommandPoolCreateInfo commandPoolinfo = vk::CommandPoolCreateInfo
    (
        { },
        graphicsQueueIndex
    );

    VKRESERRMSG(device.createCommandPool(&commandPoolinfo, nullptr, &m_eteCommandPool), "Failed to create VulkanProfiler end to end command pool");

    constexpr uint32_t CommandBufferCount = VulkanFlightPoolSize * 2;

    vk::CommandBuffer buffers[CommandBufferCount];
    const vk::CommandBufferAllocateInfo commandBufferInfo = vk::CommandBufferAllocateInfo
    (
        m_eteCommandPool,
        vk::CommandBufferLevel::ePrimary,
        CommandBufferCount
    );

    constexpr vk::QueryPoolCreateInfo QueryInfo = vk::QueryPoolCreateInfo({ }, vk::QueryType::eTimestamp, 2);

    VKRESERRMSG(device.allocateCommandBuffers(&commandBufferInfo, buffers), "Failed to allocate VulkanProfiler end to end command buffers");
    for (uint32_t i = 0; i < VulkanFlightPoolSize; ++i)
    {
        const vk::CommandBuffer startBuffer = buffers[i * 2 + 0];
        const vk::CommandBuffer endBuffer = buffers[i * 2 + 1];

        m_eteStartCommandBuffer[i] = startBuffer;
        m_eteEndCommandBuffer[i] = endBuffer;

        VKRESERRMSG(device.createQueryPool(&QueryInfo, nullptr, &(m_etePools[i])), "Failed to create VulkanProfiler end to end query pool");

        constexpr vk::CommandBufferBeginInfo BeginInfo;

        startBuffer.begin(BeginInfo);
        IDEFER(startBuffer.end());

        // You do you Vulkan not here to argue
        // One set of queues it is fine resetting host side but another it flips its shit and has to be device side
        startBuffer.resetQueryPool(m_etePools[i], 0, 2);
        startBuffer.writeTimestamp(vk::PipelineStageFlagBits::eTopOfPipe, m_etePools[i], 0);

        endBuffer.begin(BeginInfo);
        IDEFER(endBuffer.end());

        endBuffer.writeTimestamp(vk::PipelineStageFlagBits::eBottomOfPipe, m_etePools[i], 1);
    }
}
VulkanProfiler::~VulkanProfiler()
{
    const vk::Device device = m_engine->GetLogicalDevice();

    ComplexAllocator* allocator = m_engine->GetAllocator();
    IVERIFY(allocator != nullptr);

    device.destroy(m_eteCommandPool);

    for (uint32_t i = 0; i < VulkanFlightPoolSize; ++i)
    {
        device.destroyQueryPool(m_etePools[i]);
    }

    for (uint32_t i = 0; i < VulkanFlightPoolSize; ++i)
    {
        for (vk::QueryPool& p : m_pools[i])
        {
            device.destroyQueryPool(p);
        }

        m_pools[i].~Array<vk::QueryPool>();
    }
    allocator->Free(m_pools);
}

void VulkanProfiler::Init(VulkanRenderEngineBackend* a_engine)
{
    if (Instance == nullptr)
    {
        ComplexAllocator* allocator = a_engine->GetAllocator();

        Instance = allocator->Create<VulkanProfiler>(a_engine);
    }
}
void VulkanProfiler::Destroy()
{
    if (Instance != nullptr)
    {
        ComplexAllocator* allocator = Instance->m_engine->GetAllocator();

        allocator->Destroy(Instance);
    }
}

struct BufferData
{
    vk::CommandBuffer CommandBuffer;
    uint32_t LastIndex;
    uint32_t FrameIndex;
};
void VulkanProfiler::Update(uint32_t a_frame)
{
    IVERIFY(Instance != nullptr);

    IVERIFY(a_frame < VulkanFlightPoolSize);

    PROFILESTACK("GPU Profiler");

    Instance->m_poolIndex = 0;
    Instance->m_queryIndex = 0;

    if (!IISBITSET(Instance->m_init, a_frame))
    {
        ISETBIT(Instance->m_init, a_frame);

        return;
    }

    const float timestampPeriod = Instance->m_engine->GetTimestampPeriod();
    const double timestampScaleMs = (double)timestampPeriod / 1e+6;
    const double timestampScaleS = (double)timestampPeriod / 1e+9;

    const vk::Device device = Instance->m_engine->GetLogicalDevice();

    RENDERSCRATCHFRAME;

    StackAllocator* scratchAllocator = RenderScratchAlloc::GetAllocator();

    {
        PROFILESTACK("End to End");

        uint64_t times[2];

        VKRESERR(device.getQueryPoolResults
        (
            Instance->m_etePools[a_frame],
            0,
            2,
            2 * sizeof(uint64_t),
            times,
            sizeof(uint64_t),
            // This wait is mostly a semantics thing as it should have already wrote them by this point
            // We can be pretty certain about that as we just waited on a fence before this function and have never observed it
            vk::QueryResultFlagBits::e64 | vk::QueryResultFlagBits::eWait
        ));

        const float endToEndTime = (float)((times[1] - times[0]) * timestampScaleS);
        Profiler::PushGPUEndToEndTime(endToEndTime);
    }

    // TArray is async so the order may not be preserved so sort
    uint32_t size;
    ProfilerData* sortedData;
    {
        PROFILESTACK("Untangling");

        const TLockArray<ProfilerData> data = Instance->m_data[a_frame].ToLockArray();
        size = data.Size();

        sortedData = scratchAllocator->ZTAllocate<ProfilerData>(size);

        for (uint32_t i = 0; i < size; ++i)
        {
            const ProfilerData& d = data[i];

            for (uint32_t j = 0; j < i; ++j)
            {
                if (sortedData[j].PoolIndex < d.PoolIndex)
                {
                    continue;
                }

                if (sortedData[j].PoolIndex == d.PoolIndex && sortedData[j].QueryIndex < d.QueryIndex)
                {
                    continue;
                }

                for (int64_t k = (int64_t)i - 1; k >= j; --k)
                {
                    sortedData[k + 1] = sortedData[k];
                }

                sortedData[j] = d;

                goto NextData;
            }

            sortedData[i] = d;
NextData:;
        }

        Instance->m_data[a_frame].UClear();
    }

    {
        RENDERSCRATCHFRAME;

        const uint32_t poolCount = (size + (PoolSize - 1)) / PoolSize;

        uint64_t* times = scratchAllocator->ZTAllocate<uint64_t>(poolCount * PoolSize);
        {
            PROFILESTACK("Reading");

            const ThreadGuard g = ThreadGuard(Instance->m_poolLock);

            for (uint32_t i = 0; i < poolCount; ++i)
            {
                const uint32_t count = ILAMBDA(
                {
                    if (i == poolCount - 1)
                    {
                        ILRETURN size % PoolSize;
                    }

                    ILRETURN PoolSize;
                });

                VKRESERR(device.getQueryPoolResults
                (
                    Instance->m_pools[a_frame][i],
                    0,
                    count,
                    count * sizeof(uint64_t),
                    times + (i * PoolSize),
                    sizeof(uint64_t),
                    vk::QueryResultFlagBits::e64 | vk::QueryResultFlagBits::eWait
                ));
            }
        }

        Array<ProfilerGPUFrameData> frames = Array<ProfilerGPUFrameData>(MallocAllocator::Instance);
        {
            PROFILESTACK("Formatting");

            Array<BufferData> buffers = Array<BufferData>(MallocAllocator::Instance);
            for (uint32_t i = 0; i < size; ++i)
            {
                const ProfilerData& d = sortedData[i];

                const uint32_t frameCount = frames.Size();

                for (BufferData& b : buffers)
                {
                    if (b.CommandBuffer != d.CommandBuffer)
                    {
                        continue;
                    }

                    IDEFER(b.LastIndex = i);

                    const float duration = (float)((times[i] - times[b.LastIndex]) * timestampScaleMs);

                    Array<ProfilerGPUFrameItem>& items = frames[b.FrameIndex].Items;
                    for (ProfilerGPUFrameItem& it : items)
                    {
                        if (it.Name != d.String)
                        {
                            continue;
                        }

                        it.Duration += duration;

                        goto NextFrame;
                    }

                    items.Push(ProfilerGPUFrameItem
                    {
                        .Name = COWU8String(d.String, MallocAllocator::Instance),
                        .Duration = duration
                    });

                    goto NextFrame;
                }

                for (uint32_t j = 0; j < frameCount; ++j)
                {
                    if (frames[j].Name != d.String)
                    {
                        continue;
                    }

                    buffers.Push(BufferData
                    {
                        .CommandBuffer = d.CommandBuffer,
                        .LastIndex = i,
                        .FrameIndex = j,
                    });

                    goto NextFrame;
                }

                buffers.Push(BufferData
                {
                    .CommandBuffer = d.CommandBuffer,
                    .LastIndex = i,
                    .FrameIndex = frameCount,
                });

                // We have no past data so we use the 1st element as the label for the pass
                frames.Push(ProfilerGPUFrameData
                {
                    .Name = COWU8String(d.String, MallocAllocator::Instance),
                    .Items = Array<ProfilerGPUFrameItem>(MallocAllocator::Instance),
                });

    NextFrame:;
            }
        }

        PROFILESTACK("Submitting Info");
        Profiler::PushGPUData(frames.Data(), frames.Size());
    }
}

vk::CommandBuffer VulkanProfiler::GetStartCommandBuffer(uint32_t a_frame)
{
    IVERIFY(Instance != nullptr);

    IVERIFY(a_frame < VulkanFlightPoolSize);

    return Instance->m_eteStartCommandBuffer[a_frame];
}
vk::CommandBuffer VulkanProfiler::GetEndCommandBuffer(uint32_t a_frame)
{
    IVERIFY(Instance != nullptr);

    IVERIFY(a_frame < VulkanFlightPoolSize);

    return Instance->m_eteEndCommandBuffer[a_frame];
}

vk::QueryPool VulkanProfiler::GetPool(uint32_t a_frame, uint32_t* a_pool, uint32_t* a_index)
{
    IVERIFY(Instance != nullptr);

    IVERIFY(a_pool != nullptr);
    IVERIFY(a_index != nullptr);
    IVERIFY(a_frame < VulkanFlightPoolSize);

    *a_pool = uint32_t(-1);
    *a_index = uint32_t(-1);

    const vk::Device device = Instance->m_engine->GetLogicalDevice();

    const ThreadGuard g = ThreadGuard(Instance->m_poolLock);

    if (Instance->m_poolIndex >= Instance->m_pools[a_frame].Size())
    {
        TRACE("Creating Vulkan Profiler Query Pool");
        constexpr vk::QueryPoolCreateInfo Info = vk::QueryPoolCreateInfo({ }, vk::QueryType::eTimestamp, PoolSize);

        vk::QueryPool pool;
        VKRESERRMSG(device.createQueryPool(&Info, nullptr, &pool), "Failed to create Vulkan Profiler query pool");

        Instance->m_pools[a_frame].Push(pool);
    }

    *a_pool = Instance->m_poolIndex;

    vk::QueryPool pool = Instance->m_pools[a_frame][*a_pool];
    if (Instance->m_queryIndex == 0)
    {
        device.resetQueryPool(pool, 0, PoolSize);

        *a_index = 0;
        Instance->m_queryIndex = 1;

        return pool;
    }

    *a_index = Instance->m_queryIndex++;

    if (*a_index >= PoolSize)
    {
        *a_pool = ++Instance->m_poolIndex;
        *a_index = 0;
        Instance->m_queryIndex = 0;
    }

    return pool;
}

void VulkanProfiler::PushPoint(const VulkanCommandBuffer& a_buffer, const COWU8String& a_str, vk::PipelineStageFlagBits a_stage, uint32_t a_frame)
{
    IVERIFY(Instance != nullptr);

    IVERIFY(a_frame < VulkanFlightPoolSize);

    const e_VulkanCommandBufferType type = a_buffer.GetBufferType();
    if (!Instance->m_engine->IsQueueTimingEnabled(type))
    {
        return;
    }

    uint32_t poolIndex;
    uint32_t index;
    const vk::QueryPool pool = GetPool(a_frame, &poolIndex, &index);
    const vk::CommandBuffer cmd = a_buffer.GetCommandBuffer();
    cmd.writeTimestamp(a_stage, pool, index);

    ComplexAllocator* allocator = Instance->m_engine->GetAllocator();
    IVERIFY(allocator != nullptr);

    const ProfilerData data =
    {
        .CommandBuffer = cmd,
        .String = COWU8String(a_str, allocator),
        .PoolIndex = poolIndex,
        .QueryIndex = index,
    };
    Instance->m_data[a_frame].Push(data);
}

void VulkanProfiler::StartTimingPoint(const VulkanCommandBuffer& a_buffer, const char* a_name)
{
#ifdef ICARIANNATIVE_ENABLE_PROFILER
    IVERIFY(a_name != nullptr);

    IVERIFY(Instance != nullptr);

    ComplexAllocator* allocator = Instance->m_engine->GetAllocator();
    IVERIFY(allocator != nullptr);

    const COWU8String str = COWU8String(a_name, allocator);
    StartTimingPoint(a_buffer, str);
#endif
}
void VulkanProfiler::StartTimingPoint(const VulkanCommandBuffer& a_buffer, const COWU8String& a_name)
{
#ifdef ICARIANNATIVE_ENABLE_PROFILER
    IVERIFY(Instance != nullptr);

    const uint32_t frame = Instance->m_engine->GetCurrentFrame();

    PushPoint(a_buffer, a_name, vk::PipelineStageFlagBits::eTopOfPipe, frame);
#endif
}

void VulkanProfiler::PushTimingPoint(const VulkanCommandBuffer& a_buffer, const char* a_name)
{
#ifdef ICARIANNATIVE_ENABLE_PROFILER
    IVERIFY(a_name != nullptr);

    IVERIFY(Instance != nullptr);

    ComplexAllocator* allocator = Instance->m_engine->GetAllocator();
    IVERIFY(allocator != nullptr);

    const COWU8String str = COWU8String(a_name, allocator);
    PushTimingPoint(a_buffer, str);
#endif
}
void VulkanProfiler::PushTimingPoint(const VulkanCommandBuffer& a_buffer, const COWU8String& a_name)
{
#ifdef ICARIANNATIVE_ENABLE_PROFILER
    IVERIFY(Instance != nullptr);

    const uint32_t frame = Instance->m_engine->GetCurrentFrame();

    PushPoint(a_buffer, a_name, vk::PipelineStageFlagBits::eBottomOfPipe, frame);
#endif
}

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
