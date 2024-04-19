#pragma once

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN
#include "Rendering/Vulkan/IcarianVulkanHeader.h"

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include "EngineMaterialInteropStructures.h"
#include "EngineTextureSamplerInteropStructures.h"

class ObjectManager;
class UIElement;
class VulkanGraphicsEngine;
class VulkanRenderEngineBackend;
class VulkanShaderStorageObject;
class VulkanUniformBuffer;

struct VulkanPushDescriptor
{
    uint32_t Set;
    uint32_t Binding;
    uint32_t Count;
    vk::DescriptorSetLayout DescriptorLayout;
};

class VulkanShaderData
{
private:
    static constexpr uint32_t StaticIndex = 0;

    VulkanRenderEngineBackend*        m_engine;
    VulkanGraphicsEngine*             m_gEngine;

    VulkanUniformBuffer*              m_userUniformBuffer;

    vk::PipelineLayout                m_layout;
    vk::PipelineLayout                m_shadowLayout;

    std::vector<VulkanPushDescriptor> m_pushDescriptors;
    std::vector<VulkanPushDescriptor> m_shadowPushDescriptors;

    vk::DescriptorSetLayout           m_staticDesciptorLayout;
    vk::DescriptorPool                m_staticDescriptorPool;
    vk::DescriptorSet                 m_staticDescriptorSet;

    vk::DescriptorSetLayout           m_shadowStaticDesciptorLayout;
    vk::DescriptorPool                m_shadowStaticDescriptorPool;
    vk::DescriptorSet                 m_shadowStaticDescriptorSet;

    std::vector<ShaderBufferInput>    m_slotInputs;
    // Want quick access to these so they are stored separately
    ShaderBufferInput                 m_userBufferInput;  
    ShaderBufferInput                 m_transformBufferInput;
    ShaderBufferInput                 m_uiBufferInput;

    std::vector<ShaderBufferInput>    m_shadowSlotInputs;

protected:

public:
    VulkanShaderData(VulkanRenderEngineBackend* a_engine, VulkanGraphicsEngine* a_gEngine, const RenderProgram& a_program);
    ~VulkanShaderData();

    inline vk::PipelineLayout GetLayout() const
    {
        return m_layout;
    }
    inline vk::PipelineLayout GetShadowLayout() const
    {
        return m_shadowLayout;
    }

    bool GetShaderBufferInput(e_ShaderBufferType a_type, ShaderBufferInput* a_input) const;
    bool GetShadowShaderBufferInput(e_ShaderBufferType a_type, ShaderBufferInput* a_input) const;

    void SetTexture(uint32_t a_slot, const TextureSamplerBuffer& a_sampler) const;

    void PushTexture(vk::CommandBuffer a_commandBuffer, uint32_t a_set, const TextureSamplerBuffer& a_sampler, uint32_t a_index) const;
    void PushTextures(vk::CommandBuffer a_commandBuffer, uint32_t a_set, const TextureSamplerBuffer* a_samplers, uint32_t a_count, uint32_t a_index) const;
    void PushUniformBuffer(vk::CommandBuffer a_commandBuffer, uint32_t a_set, const VulkanUniformBuffer* a_buffer, uint32_t a_index) const;
    void PushShaderStorageObject(vk::CommandBuffer a_commandBuffer, uint32_t a_set, const VulkanShaderStorageObject* a_object, uint32_t a_index) const;
    void PushShaderStorageObject(vk::CommandBuffer a_commandBuffer, uint32_t a_set, vk::Buffer a_object, uint32_t a_index) const;

    void PushShadowTexture(vk::CommandBuffer a_commandBuffer, uint32_t a_set, const TextureSamplerBuffer& a_sampler, uint32_t a_index) const;
    void PushShadowUniformBuffer(vk::CommandBuffer a_commandBuffer, uint32_t a_set, const VulkanUniformBuffer* a_buffer, uint32_t a_index) const;
    void PushShadowShaderStorageObject(vk::CommandBuffer a_commandBuffer, uint32_t a_set, const VulkanShaderStorageObject* a_object, uint32_t a_index) const;

    void UpdateTransformBuffer(vk::CommandBuffer a_commandBuffer, const glm::mat4& a_transform) const;
    void UpdateShadowTransformBuffer(vk::CommandBuffer a_commandBuffer, const glm::mat4& a_transform) const;
    
    void UpdateUIBuffer(vk::CommandBuffer a_commandBuffer, const UIElement* a_element) const;

    void UpdateShadowLightBuffer(vk::CommandBuffer a_commandBuffer, const glm::mat4& a_lvp, float a_split) const;

    void Update(uint32_t a_index, const RenderProgram& a_program);
    void BindShadow(uint32_t a_index, vk::CommandBuffer a_commandBuffer) const;
    void Bind(uint32_t a_index, vk::CommandBuffer a_commandBuffer) const;
};
#endif