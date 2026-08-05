// Icarian Engine - C# Game Engine
//
// License at end of file.

#pragma once

#include "Core/DataTypes/Array.h"
#include "Core/DataTypes/COWString.h"
#include "Core/DataTypes/SpinLock.h"
#include "DataTypes/TArray.h"
#include "Rendering/Vulkan/IcarianVulkanHeader.h"
#include "Rendering/Vulkan/VulkanCommandBuffer.h"

class VulkanRenderEngineBackend;

class VulkanProfiler
{
private:
    // This is a preface to explain why the profiler is made the way it is
    // So GPUs are a bitch and highly async so we need to take the values with a grain of salt
    // Because of that we basically need 2 timers the end to end and the per segment breakdown
    // We do per segment breakdown mostly because scheduling can mean that a lot of work can get overlapped
    // Because a lot of work can get overlapped even per segment should be taken with a grain of salt but gives a better view then per job
    // NEVER use per item breakdowns in a job as they will be horribly inaccurate because of the async nature
    // It should be noted there is also no guarantee of order between sync points only the order of sync points on modern GPUs soo..... have fun
    // Yes the fact that drivers will go fuck you and reorder has cause a many a bug in raster work dont ask
    // We have to do a little fuckery because Vulkan only allows measuring time in a command buffer
    // To get around that we have to insert a command buffer at the very start and end of the frame that is purely to measure end to end
    // Yes I have gone down a whole GPU profiling rabbit hole and gone mad consensus seems to be give up and track something
    // Having a timing to action is better then nothing sooo... This is an answer until I decide I want to go even more mad

    constexpr static uint32_t PoolSize = 128;

    VulkanRenderEngineBackend* m_engine;

    struct ProfilerData
    {
        vk::CommandBuffer CommandBuffer;
        IcarianCore::COWU8String String;
        uint32_t PoolIndex;
        uint32_t QueryIndex;
        e_VulkanCommandBufferType Type;
    };

    IcarianCore::Array<vk::QueryPool>* m_pools;
    TArray<ProfilerData>               m_data[VulkanFlightPoolSize];

    vk::CommandPool                    m_eteCommandPool;
    vk::CommandBuffer                  m_eteStartCommandBuffer[VulkanFlightPoolSize];
    vk::CommandBuffer                  m_eteEndCommandBuffer[VulkanFlightPoolSize];
    vk::QueryPool                      m_etePools[VulkanFlightPoolSize];

    uint32_t                           m_queryIndex;
    uint32_t                           m_poolIndex;

    IcarianCore::SpinLock              m_poolLock;
    uint8_t                            m_init;

    static vk::QueryPool GetPool(uint32_t a_frame, uint32_t* a_pool, uint32_t* a_index);
    static void PushPoint(const VulkanCommandBuffer& a_buffer, const IcarianCore::COWU8String& a_str, vk::PipelineStageFlagBits a_stage, uint32_t a_frame);

protected:

public:
    VulkanProfiler(VulkanRenderEngineBackend* a_engine);
    ~VulkanProfiler();

    static void Init(VulkanRenderEngineBackend* a_engine);
    static void Destroy();
    static void Update(uint32_t a_frame);

    static vk::CommandBuffer GetStartCommandBuffer(uint32_t a_frame);
    static vk::CommandBuffer GetEndCommandBuffer(uint32_t a_frame);

    static void StartTimingPoint(const VulkanCommandBuffer& a_buffer, const char* a_name);
    static void StartTimingPoint(const VulkanCommandBuffer& a_buffer, const IcarianCore::COWU8String& a_name);

    static void PushTimingPoint(const VulkanCommandBuffer& a_buffer, const char* a_name);
    static void PushTimingPoint(const VulkanCommandBuffer& a_buffer, const IcarianCore::COWU8String& a_name);
};

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
