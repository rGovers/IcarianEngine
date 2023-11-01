#pragma once

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN
#include "Rendering/Vulkan/VulkanConstants.h"

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

    uint32_t                     m_directionalLightBufferIndex;
    TArray<VulkanUniformBuffer*> m_directionalLightBuffers;

    uint32_t                     m_pointLightBufferIndex;
    TArray<VulkanUniformBuffer*> m_pointLightBuffers;

    uint32_t                     m_spotLightBufferIndex;
    TArray<VulkanUniformBuffer*> m_spotLightBuffers;

    vk::DescriptorSet GenerateDescriptor(vk::DescriptorPool a_pool, const vk::DescriptorSetLayout* a_layout);
protected:

public:
    VulkanPushPool(VulkanRenderEngineBackend* a_engine);
    ~VulkanPushPool();

    vk::DescriptorSet AllocateDescriptor(uint32_t a_index, vk::DescriptorType a_type, const vk::DescriptorSetLayout* a_layout);
    void Reset(uint32_t a_index);

    VulkanUniformBuffer* AllocateDirectionalLightUniformBuffer();
    VulkanUniformBuffer* AllocatePointLightUniformBuffer();
    VulkanUniformBuffer* AllocateSpotLightUniformBuffer();
};

#endif