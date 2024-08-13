// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/IcarianVulkanHeader.h"

class VulkanRenderEngineBackend;

class VulkanUniformBuffer
{
private:
    VulkanRenderEngineBackend* m_engine;

    uint32_t                   m_uniformSize;
    vk::Buffer                 m_buffers[VulkanFlightPoolSize];
    VmaAllocation              m_allocations[VulkanFlightPoolSize];
protected:

public:
    VulkanUniformBuffer(VulkanRenderEngineBackend* a_engine, uint32_t a_uniformSize);
    ~VulkanUniformBuffer();

    void SetData(uint32_t a_index, const void* a_data);

    inline vk::Buffer GetBuffer(uint32_t a_index) const
    {
        return m_buffers[a_index];
    }
    inline uint32_t GetSize() const
    {
        return m_uniformSize;
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