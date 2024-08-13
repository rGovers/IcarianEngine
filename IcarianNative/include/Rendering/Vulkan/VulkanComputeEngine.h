// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN
#include "Rendering/Vulkan/IcarianVulkanHeader.h"

#include "DataTypes/TNCArray.h"
#include "Rendering/Vulkan/VulkanCommandBuffer.h"

class VulkanComputeEngineBindings;
class VulkanComputeLayout;
class VulkanComputeParticle2D;
class VulkanComputePipeline;
class VulkanComputeShader;
class VulkanRenderEngineBackend;
class VulkanUniformBuffer;

#include "EngineMaterialInteropStructures.h"
#include "EngineParticleSystemInteropStructures.h"

class VulkanComputeEngine
{
private:
    friend class VulkanComputeEngineBindings;

    VulkanRenderEngineBackend*         m_engine;
    VulkanComputeEngineBindings*       m_bindings;

    VulkanUniformBuffer*               m_timeUniform;

    vk::CommandPool                    m_pools[VulkanFlightPoolSize];
    vk::CommandBuffer                  m_buffers[VulkanFlightPoolSize];

    TNCArray<VulkanComputeShader*>     m_shaders;
    TNCArray<VulkanComputePipeline*>   m_pipelines;
    TNCArray<VulkanComputeLayout*>     m_layouts;
    
    TNCArray<ComputeParticleBuffer>    m_particleBuffers;

protected:

public:
    VulkanComputeEngine(VulkanRenderEngineBackend* a_engine);
    ~VulkanComputeEngine();

    inline VulkanRenderEngineBackend* GetRenderEngineBackend() const
    {
        return m_engine;
    }

    inline VulkanUniformBuffer* GetTimeUniformBuffer() const
    {
        return m_timeUniform;
    }

    VulkanCommandBuffer Update(double a_delta, double a_time, uint32_t a_index);

    ComputeParticleBuffer GetParticleBuffer(uint32_t a_addr);
    void SetParticleBuffer(uint32_t a_addr, const ComputeParticleBuffer& a_buffer);

    vk::Buffer GetParticleBufferData(uint32_t a_addr);

    uint32_t GenerateComputeFShader(const std::string_view& a_str);
    void DestroyComputeShader(uint32_t a_addr);
    VulkanComputeShader* GetComputeShader(uint32_t a_addr);

    uint32_t GenerateComputePipelineLayout(const ShaderBufferInput* a_inputs, uint32_t a_count);
    void DestroyComputePipelineLayout(uint32_t a_addr);
    VulkanComputeLayout* GetComputePipelineLayout(uint32_t a_addr);

    uint32_t GenerateComputePipeline(uint32_t a_shaderAddr, uint32_t a_layoutAddr);
    void DestroyComputePipeline(uint32_t a_addr);
    VulkanComputePipeline* GetComputePipeline(uint32_t a_addr);
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