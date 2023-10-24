#pragma once

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN
#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include "Rendering/Vulkan/VulkanConstants.h"

#include "Flare/TextureSampler.h"

#include "EngineMaterialInteropStructures.h"

class ObjectManager;
class UIElement;
class VulkanGraphicsEngine;
class VulkanRenderEngineBackend;
class VulkanShaderStorageObject;
class VulkanUniformBuffer;

class VulkanShaderData
{
private:
    // Emulating push descriptors cause of AMD
    // Newer drivers have it but relying on new version
    struct PushDescriptor
    {
        uint32_t Set;
        uint32_t Binding;
        vk::DescriptorSetLayout DescriptorLayout;
    };

    static constexpr uint32_t StaticIndex = 0;

    VulkanRenderEngineBackend*     m_engine;
    VulkanGraphicsEngine*          m_gEngine;

    VulkanUniformBuffer*           m_userUniformBuffer;

    vk::PipelineLayout             m_layout;

    std::vector<PushDescriptor>    m_pushDescriptors;

    vk::DescriptorSetLayout        m_staticDesciptorLayout;
    vk::DescriptorPool             m_staticDescriptorPool;
    vk::DescriptorSet              m_staticDescriptorSet;

    std::vector<ShaderBufferInput> m_slotInputs;
    // Want quick access to these so they are stored separately
    ShaderBufferInput              m_userBufferInput;  
    ShaderBufferInput              m_transformBufferInput;
    ShaderBufferInput              m_uiBufferInput;

protected:

public:
    VulkanShaderData(VulkanRenderEngineBackend* a_engine, VulkanGraphicsEngine* a_gEngine, const RenderProgram& a_program);
    ~VulkanShaderData();

    inline vk::PipelineLayout GetLayout() const
    {
        return m_layout;
    }

    bool GetCameraInput(ShaderBufferInput* a_input) const;

    bool GetDirectionalLightInput(ShaderBufferInput* a_input) const;
    bool GetPointLightInput(ShaderBufferInput* a_input) const;
    bool GetSpotLightInput(ShaderBufferInput* a_input) const;

    bool GetBatchModelBufferInput(ShaderBufferInput* a_input) const;
    
    bool GetBoneBufferInput(ShaderBufferInput* a_input) const;

    void SetTexture(uint32_t a_slot, const FlareBase::TextureSampler& a_sampler) const;

    void PushTexture(vk::CommandBuffer a_commandBuffer, uint32_t a_set, const FlareBase::TextureSampler& a_sampler, uint32_t a_index) const;
    void PushUniformBuffer(vk::CommandBuffer a_commandBuffer, uint32_t a_set, const VulkanUniformBuffer* a_buffer, uint32_t a_index) const;
    void PushShaderStorageObject(vk::CommandBuffer a_commandBuffer, uint32_t a_set, const VulkanShaderStorageObject* a_object, uint32_t a_index) const;

    void UpdateTransformBuffer(vk::CommandBuffer a_commandBuffer, const glm::mat4& a_transform) const;
    void UpdateUIBuffer(vk::CommandBuffer a_commandBuffer, const UIElement* a_element) const;

    void Update(uint32_t a_index, const RenderProgram& a_program);
    void Bind(uint32_t a_index, vk::CommandBuffer a_commandBuffer) const;
};
#endif