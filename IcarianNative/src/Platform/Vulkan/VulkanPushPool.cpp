// Icarian Engine - C# Game Engine
// 
// License at end of file.

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/VulkanPushPool.h"

#include "Core/IcarianAssert.h"
#include "Core/ShaderBuffers.h"
#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"
#include "Rendering/Vulkan/VulkanUniformBuffer.h"
#include "Trace.h"

VulkanPushPool::VulkanPushPool(VulkanRenderEngineBackend* a_engine)
{
    m_engine = a_engine;
}
VulkanPushPool::~VulkanPushPool()
{
    const vk::Device device = m_engine->GetLogicalDevice();

    for (uint32_t i = 0; i < VulkanFlightPoolSize; ++i)
    {
        const uint32_t count = m_buffers[i].Size();
        for (uint32_t j = 0; j < count; ++j)
        {
            device.destroyDescriptorPool(m_buffers[i][j].Pool);
        }
    }

    for (uint32_t i = 0; i < m_ambientLightBuffers.Size(); ++i)
    {
        if (m_ambientLightBuffers[i] != nullptr)
        {
            delete m_ambientLightBuffers[i];
        }
    }
    for (uint32_t i = 0; i < m_directionalLightBuffers.Size(); ++i)
    {
        if (m_directionalLightBuffers[i] != nullptr)
        {
            delete m_directionalLightBuffers[i];
        }
    }
    for (uint32_t i = 0; i < m_pointLightBuffers.Size(); ++i)
    {
        if (m_pointLightBuffers[i] != nullptr)
        {
            delete m_pointLightBuffers[i];
        }
    }
    for (uint32_t i = 0; i < m_spotLightBuffers.Size(); ++i)
    {
        if (m_spotLightBuffers[i] != nullptr)
        {
            delete m_spotLightBuffers[i];
        }
    }
    
    for (uint32_t i = 0; i < m_shadowBuffers.Size(); ++i)
    {
        if (m_shadowBuffers[i] != nullptr)
        {
            delete m_shadowBuffers[i];
        }
    }
}

vk::DescriptorSet VulkanPushPool::GenerateDescriptor(vk::DescriptorPool a_pool, const vk::DescriptorSetLayout* a_layout)
{
    const vk::Device device = m_engine->GetLogicalDevice();

    const vk::DescriptorSetAllocateInfo descriptorSetInfo = vk::DescriptorSetAllocateInfo
    (
        a_pool,
        1,
        a_layout
    );

    vk::DescriptorSet descriptorSet;
    // Apparently the Vulkan validation layers have an issue with threading so may occasionaly get a crash here in Debug
    // Only recently popped up so suspect a regression in the Vulkan API, I imagine gonna be fixed if I update again
    // Have not had it occur in Release so not gonna worry about it
    VKRESERR(device.allocateDescriptorSets(&descriptorSetInfo, &descriptorSet));

    return descriptorSet;
}

vk::DescriptorSet VulkanPushPool::AllocateDescriptor(uint32_t a_index, vk::DescriptorType a_type, const vk::DescriptorSetLayout* a_layout, uint32_t a_size)
{
    TLockArray<VulkanPushPoolBuffer> buffers = m_buffers[a_index].ToLockArray();
    for (VulkanPushPoolBuffer& buffer : buffers)
    {
        if (buffer.Type == a_type && buffer.Count + a_size < MaxPoolSize)
        {
            buffer.Count += a_size;

            return GenerateDescriptor(buffer.Pool, a_layout);
        }
    }

    TRACE("Creating new descriptor push pool");
    const vk::Device device = m_engine->GetLogicalDevice();

    VulkanPushPoolBuffer buffer;
    buffer.Count = a_size;
    buffer.Type = a_type;
    
    const vk::DescriptorPoolSize poolSize = vk::DescriptorPoolSize
    (
        a_type,
        MaxPoolSize
    );

    const vk::DescriptorPoolCreateInfo poolInfo = vk::DescriptorPoolCreateInfo
    (
        { },
        MaxPoolSize,
        1,
        &poolSize
    );

    vk::DescriptorPool pool;
    ICARIAN_ASSERT_R(device.createDescriptorPool(&poolInfo, nullptr, &pool) == vk::Result::eSuccess);

    buffer.Pool = pool;

    // Have the lock here so can use UPush
    m_buffers[a_index].UPush(buffer);

    return GenerateDescriptor(buffer.Pool, a_layout);
}
void VulkanPushPool::Reset(uint32_t a_index)
{
    const vk::Device device = m_engine->GetLogicalDevice();

    TLockArray<VulkanPushPoolBuffer> buffers = m_buffers[a_index].ToLockArray();
    for (VulkanPushPoolBuffer& buffer : buffers)
    {
        // Unused so no need to reset
        if (buffer.Count > 0)
        {
            device.resetDescriptorPool(buffer.Pool);
            
            buffer.Count = 0;
        }
    }

    m_ambientLightBufferIndex = 0;
    m_directionalLightBufferIndex = 0;
    m_pointLightBufferIndex = 0;
    m_spotLightBufferIndex = 0;
    m_shadowBufferIndex = 0;
}

VulkanUniformBuffer* VulkanPushPool::AllocateAmbientLightUniformBuffer()
{
    TLockArray<VulkanUniformBuffer*> a = m_ambientLightBuffers.ToLockArray();
    if (m_ambientLightBufferIndex >= a.Size())
    {
        TRACE("Creating new ambient light uniform buffer");
        VulkanUniformBuffer* buffer = new VulkanUniformBuffer(m_engine, sizeof(IcarianCore::ShaderAmbientLightBuffer));
        m_ambientLightBuffers.UPush(buffer);

        ++m_ambientLightBufferIndex;

        return buffer;
    }

    return a[m_ambientLightBufferIndex++];
}
VulkanUniformBuffer* VulkanPushPool::AllocateDirectionalLightUniformBuffer()
{
    TLockArray<VulkanUniformBuffer*> a = m_directionalLightBuffers.ToLockArray();
    if (m_directionalLightBufferIndex >= a.Size())
    {
        TRACE("Creating new directional light uniform buffer");
        VulkanUniformBuffer* buffer = new VulkanUniformBuffer(m_engine, sizeof(IcarianCore::ShaderDirectionalLightBuffer));
        m_directionalLightBuffers.UPush(buffer);

        ++m_directionalLightBufferIndex;

        return buffer;
    }

    return a[m_directionalLightBufferIndex++];
}
VulkanUniformBuffer* VulkanPushPool::AllocatePointLightUniformBuffer()
{
    TLockArray<VulkanUniformBuffer*> a = m_pointLightBuffers.ToLockArray();
    if (m_pointLightBufferIndex >= a.Size())
    {
        TRACE("Creating new point light uniform buffer");
        VulkanUniformBuffer* buffer = new VulkanUniformBuffer(m_engine, sizeof(IcarianCore::ShaderPointLightBuffer));
        m_pointLightBuffers.UPush(buffer);

        ++m_pointLightBufferIndex;

        return buffer;
    }

    return a[m_pointLightBufferIndex++];
}
VulkanUniformBuffer* VulkanPushPool::AllocateSpotLightUniformBuffer()
{
    TLockArray<VulkanUniformBuffer*> a = m_spotLightBuffers.ToLockArray();
    if (m_spotLightBufferIndex >= a.Size())
    {
        TRACE("Creating new spot light uniform buffer");
        VulkanUniformBuffer* buffer = new VulkanUniformBuffer(m_engine, sizeof(IcarianCore::ShaderSpotLightBuffer));
        m_spotLightBuffers.UPush(buffer);

        ++m_spotLightBufferIndex;

        return buffer;
    }

    return a[m_spotLightBufferIndex++];
}

VulkanUniformBuffer* VulkanPushPool::AllocateShadowUniformBuffer()
{
    TLockArray<VulkanUniformBuffer*> a = m_shadowBuffers.ToLockArray();
    if (m_shadowBufferIndex >= a.Size())
    {
        TRACE("Creating new shadow uniform buffer");
        VulkanUniformBuffer* buffer = new VulkanUniformBuffer(m_engine, sizeof(IcarianCore::ShaderShadowLightBuffer));
        m_shadowBuffers.UPush(buffer);

        ++m_shadowBufferIndex;

        return buffer;
    }

    return a[m_shadowBufferIndex++];
}

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