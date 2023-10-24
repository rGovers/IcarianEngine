#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/VulkanPushPool.h"

#include "Flare/IcarianAssert.h"
#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"

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

vk::DescriptorSet VulkanPushPool::AllocateDescriptor(uint32_t a_index, vk::DescriptorType a_type, const vk::DescriptorSetLayout* a_layout)
{
    {
        TLockArray<VulkanPushPoolBuffer> buffers = m_buffers[a_index].ToLockArray();
        for (VulkanPushPoolBuffer& buffer : buffers)
        {
            if (buffer.Type == a_type && buffer.Count < MaxPoolSize)
            {
                ++buffer.Count;

                return GenerateDescriptor(buffer.Pool, a_layout);
            }
        }
    }

    // Hypothetically possible to create multiple pools when multiple threads are trying to allocate at the same time
    // Not going to worry about that as all pools should still be in the array so should not leak just means a small amount of memory is wasted
    const vk::Device device = m_engine->GetLogicalDevice();

    VulkanPushPoolBuffer buffer;
    buffer.Count = 1;
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

    m_buffers[a_index].Push(buffer);

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
}

#endif