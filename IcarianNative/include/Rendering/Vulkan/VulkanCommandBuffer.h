// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/IcarianVulkanHeader.h"

enum e_VulkanCommandBufferType
{
    VulkanCommandBufferType_Compute,
    VulkanCommandBufferType_VideoDecode,
    VulkanCommandBufferType_Graphics,
};

enum e_VulkanCommandBufferStage
{
    VulkanCommandBufferStage_Null,
    // This is a special stage that runs async
    VulkanCommandBufferStage_ComputePass,
    // Passes that are grouped run at the same time
    // (Shadow, Main) -> (Lighting, Forward) -> (Post) -> (UI)
    VulkanCommandBufferStage_ShadowPass,
    VulkanCommandBufferStage_DeferredPass,
    VulkanCommandBufferStage_LightingPass,
    VulkanCommandBufferStage_ForwardPass,
    VulkanCommandBufferStage_PostPass,
    VulkanCommandBufferStage_UIPass
    // So overall have the following going
    // | Compute                                         |
    // | Shadow   | ----->| Lighting | --->| Post | ->| UI |
    // | Deferred | _| |_ | Forward  | _|
    // Probably going to add a pre pass for occlusion so likely to become
    // Still need to fit Video in this somehow but urgh that has been a headache
    // | Video                                                            |
    // | Compute                                                          |
    // | Prepass | --->| Shadow  | ----->| Lighting | -->| Post | -> | UI |
    //              |_ | Deferred| _| |_ | Forward  | _|
    // If a hypothetical GPU come along can with multi graphics queues can split Shadow and Deffered between them and Lighting and Forward between them
    // However no current hardware exists that I am aware of and could only do it with mutli GPU which is its own headache for all 3 users in the world
    // It makes me sad that deferred can be pushed further but current GPUs do not allow it

    // I did think of something which is move the Lighting pass to pure compute and hijack the compute queue so we can do it at the same time as the forward pass
    // However that is a limitation that we would have to make the C# side aware of and also means would have to saw the async compute pipeline in 2 on most hardware
    // Benefits are questionable would need to do further testing
};

// TODO: Switch to this down the line
class VulkanCommandBuffer
{
private:
    vk::CommandBuffer          m_commandBuffer;
    e_VulkanCommandBufferType  m_type;
    e_VulkanCommandBufferStage m_stage;

protected:

public:
    VulkanCommandBuffer(const vk::CommandBuffer& a_buffer, e_VulkanCommandBufferType a_type, e_VulkanCommandBufferStage a_stage)
    {
        m_commandBuffer = a_buffer;
        m_type = a_type;
        m_stage = a_stage;
    }
    ~VulkanCommandBuffer() { }

    inline vk::CommandBuffer GetCommandBuffer() const
    {
        return m_commandBuffer;
    }
    inline void SetCommandBuffer(const vk::CommandBuffer& a_buffer)
    {
        m_commandBuffer = a_buffer;
    }

    inline e_VulkanCommandBufferStage GetBufferStage() const
    {
        return m_stage;
    }

    inline e_VulkanCommandBufferType GetBufferType() const
    {
        return m_type;
    }
};

#endif

// MIT License
// 
// Copyright (c) 2025 River Govers
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
