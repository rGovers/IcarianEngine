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

// TODO: Switch to this down the line
class VulkanCommandBuffer
{
private:
    vk::CommandBuffer         m_commandBuffer;
    e_VulkanCommandBufferType m_type;

protected:

public:
    VulkanCommandBuffer(const vk::CommandBuffer& a_buffer, e_VulkanCommandBufferType a_type)
    {
        m_commandBuffer = a_buffer;
        m_type = a_type;
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

    inline e_VulkanCommandBufferType GetBufferType() const
    {
        return m_type;
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