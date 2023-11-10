#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/VulkanPushPool.h"

#include "Flare/IcarianAssert.h"
#include "Flare/IcarianDefer.h"
#include "Rendering/ShaderBuffers.h"
#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"
#include "Rendering/Vulkan/VulkanUniformBuffer.h"

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
    ICARIAN_ASSERT_R(device.allocateDescriptorSets(&descriptorSetInfo, &descriptorSet) == vk::Result::eSuccess);

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
        buffer.Count = 0;

        device.resetDescriptorPool(buffer.Pool);
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
        VulkanUniformBuffer* buffer = new VulkanUniformBuffer(m_engine, sizeof(AmbientLightShaderBuffer));
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
        VulkanUniformBuffer* buffer = new VulkanUniformBuffer(m_engine, sizeof(DirectionalLightShaderBuffer));
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
        VulkanUniformBuffer* buffer = new VulkanUniformBuffer(m_engine, sizeof(PointLightShaderBuffer));
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
        VulkanUniformBuffer* buffer = new VulkanUniformBuffer(m_engine, sizeof(SpotLightShaderBuffer));
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
        VulkanUniformBuffer* buffer = new VulkanUniformBuffer(m_engine, sizeof(ShadowLightShaderBuffer));
        m_shadowBuffers.UPush(buffer);

        ++m_shadowBufferIndex;

        return buffer;
    }

    return a[m_shadowBufferIndex++];
}

#endif