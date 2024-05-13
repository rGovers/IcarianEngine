#pragma once

#include "DataTypes/Array.h"
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
    uint32_t Slot;
    uint32_t Count;
    vk::DescriptorSetLayout DescriptorLayout;
};

struct VulkanShaderInput
{
    uint32_t Slot;
    uint32_t Count;
    e_ShaderBufferType BufferType;
    vk::ShaderStageFlags StageFlags;
};

struct VulkanTextureBinding
{
    uint32_t Slot;
    uint32_t SamplerAddr;
};

class VulkanShaderData
{
private:
    static constexpr uint32_t StaticIndex = 0;

    VulkanRenderEngineBackend*  m_engine;
    VulkanGraphicsEngine*       m_gEngine;
 
    VulkanUniformBuffer*        m_userUniformBuffer;
 
    vk::PipelineLayout          m_layout;
    vk::PipelineLayout          m_shadowLayout;

    Array<VulkanPushDescriptor> m_pushDescriptors;
    Array<VulkanPushDescriptor> m_shadowPushDescriptors;

    Array<VulkanShaderInput>    m_slotInputs;
    // Want quick access to these so they are stored separately
    VulkanShaderInput           m_userBufferInput;  
    VulkanShaderInput           m_transformBufferInput;
    VulkanShaderInput           m_uiBufferInput;

    Array<VulkanShaderInput>    m_shadowSlotInputs;

    Array<VulkanTextureBinding> m_textures;

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

    void SetTexture(uint32_t a_slot, uint32_t a_sampleAddr);

    void PushTexture(vk::CommandBuffer a_commandBuffer, uint32_t a_slot, const TextureSamplerBuffer& a_sampler, uint32_t a_index) const;
    void PushTextures(vk::CommandBuffer a_commandBuffer, uint32_t a_slot, const TextureSamplerBuffer* a_samplers, uint32_t a_count, uint32_t a_index) const;
    void PushUniformBuffer(vk::CommandBuffer a_commandBuffer, uint32_t a_slot, const VulkanUniformBuffer* a_buffer, uint32_t a_index) const;
    void PushShaderStorageObject(vk::CommandBuffer a_commandBuffer, uint32_t a_slot, const VulkanShaderStorageObject* a_object, uint32_t a_index) const;
    void PushShaderStorageObject(vk::CommandBuffer a_commandBuffer, uint32_t a_slot, vk::Buffer a_object, uint32_t a_index) const;

    void PushShadowTexture(vk::CommandBuffer a_commandBuffer, uint32_t a_slot, const TextureSamplerBuffer& a_sampler, uint32_t a_index) const;
    void PushShadowUniformBuffer(vk::CommandBuffer a_commandBuffer, uint32_t a_slot, const VulkanUniformBuffer* a_buffer, uint32_t a_index) const;
    void PushShadowShaderStorageObject(vk::CommandBuffer a_commandBuffer, uint32_t a_slot, const VulkanShaderStorageObject* a_object, uint32_t a_index) const;

    void UpdateTransformBuffer(vk::CommandBuffer a_commandBuffer, const glm::mat4& a_transform) const;
    void UpdateShadowTransformBuffer(vk::CommandBuffer a_commandBuffer, const glm::mat4& a_transform) const;
    
    void UpdateUIBuffer(vk::CommandBuffer a_commandBuffer, const UIElement* a_element) const;

    void UpdateShadowLightBuffer(vk::CommandBuffer a_commandBuffer, const glm::mat4& a_lvp, float a_split) const;

    void Update(uint32_t a_index, const RenderProgram& a_program);
    void BindShadow(uint32_t a_index, vk::CommandBuffer a_commandBuffer) const;
    void Bind(uint32_t a_index, vk::CommandBuffer a_commandBuffer) const;
};
#endif