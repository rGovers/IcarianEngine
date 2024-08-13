// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN
#include "Rendering/Vulkan/IcarianVulkanHeader.h"

#include <cstdint>

class VulkanComputeEngine;

#include "EngineParticleSystemInteropStructures.h"

class VulkanComputeParticle
{
public:
    static constexpr uint32_t MaxParticleBuffers = 2;

private:
    VulkanComputeEngine* m_engine;
        
    uint32_t             m_computeShader;
    uint32_t             m_computeLayout;
    uint32_t             m_computePipeline;
    uint32_t             m_particleBufferAddr;

    uint32_t             m_bufferIndex;
    VmaAllocation        m_allocations[MaxParticleBuffers];
    vk::Buffer           m_particleBuffers[MaxParticleBuffers];

    void Clear();
    void Rebuild(ComputeParticleBuffer* a_buffer);

protected:

public:
    VulkanComputeParticle(VulkanComputeEngine* a_engine, uint32_t a_particleBufferAddr);
    ~VulkanComputeParticle();

    void Update(vk::CommandBuffer a_cmdBuffer, uint32_t a_index);

    inline vk::Buffer GetComputeBuffer() const
    {
        return m_particleBuffers[m_bufferIndex];
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