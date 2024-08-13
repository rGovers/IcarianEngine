// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/IcarianVulkanHeader.h"

#include "DataTypes/TArray.h"

class VulkanRenderEngineBackend;
class VulkanUniformBuffer;

struct VulkanPushPoolBuffer
{
    uint32_t Count;
    vk::DescriptorPool Pool;
    vk::DescriptorType Type;
};

class VulkanPushPool
{
private:
    static constexpr uint32_t MaxPoolSize = 128;

    VulkanRenderEngineBackend*   m_engine;

    TArray<VulkanPushPoolBuffer> m_buffers[VulkanFlightPoolSize];

    uint32_t                     m_ambientLightBufferIndex;
    TArray<VulkanUniformBuffer*> m_ambientLightBuffers;

    uint32_t                     m_directionalLightBufferIndex;
    TArray<VulkanUniformBuffer*> m_directionalLightBuffers;

    uint32_t                     m_pointLightBufferIndex;
    TArray<VulkanUniformBuffer*> m_pointLightBuffers;

    uint32_t                     m_spotLightBufferIndex;
    TArray<VulkanUniformBuffer*> m_spotLightBuffers;

    uint32_t                     m_shadowBufferIndex;
    TArray<VulkanUniformBuffer*> m_shadowBuffers;

    vk::DescriptorSet GenerateDescriptor(vk::DescriptorPool a_pool, const vk::DescriptorSetLayout* a_layout);
protected:

public:
    VulkanPushPool(VulkanRenderEngineBackend* a_engine);
    ~VulkanPushPool();

    vk::DescriptorSet AllocateDescriptor(uint32_t a_index, vk::DescriptorType a_type, const vk::DescriptorSetLayout* a_layout, uint32_t a_size = 1);
    void Reset(uint32_t a_index);

    VulkanUniformBuffer* AllocateAmbientLightUniformBuffer();
    VulkanUniformBuffer* AllocateDirectionalLightUniformBuffer();
    VulkanUniformBuffer* AllocatePointLightUniformBuffer();
    VulkanUniformBuffer* AllocateSpotLightUniformBuffer();

    VulkanUniformBuffer* AllocateShadowUniformBuffer();
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