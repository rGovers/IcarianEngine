// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/IcarianVulkanHeader.h"

class VulkanRenderEngineBackend;

class VulkanModel
{
private:
    VulkanRenderEngineBackend* m_engine;

    float                      m_radius;

    VmaAllocation              m_allocation;
    vk::Buffer                 m_buffer;

    uint32_t                   m_offset;
    uint32_t                   m_indexCount;

protected:

public:
    VulkanModel(VulkanRenderEngineBackend* a_engine, uint32_t a_vertexCount, const void* a_vertices, uint16_t a_vertexSize, uint32_t a_indexCount, const uint32_t* a_indices, float a_radius);
    ~VulkanModel();

    inline uint32_t GetIndexCount() const
    {
        return m_indexCount;
    }
    inline float GetRadius() const
    {
        return m_radius;
    }

    void Bind(const vk::CommandBuffer& a_cmdBuffer) const;
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