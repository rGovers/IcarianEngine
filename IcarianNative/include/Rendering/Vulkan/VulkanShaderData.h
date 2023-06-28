#pragma once

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include "Rendering/Vulkan/VulkanConstants.h"

#include "Flare/ShaderBufferInput.h"
#include "Flare/TextureSampler.h"

class ObjectManager;
class UIElement;
class VulkanGraphicsEngine;
class VulkanRenderEngineBackend;
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
        vk::DescriptorPool DescriptorPool;
    };


    static constexpr uint32_t PushCount = 128;
    static constexpr uint32_t StaticIndex = 0;

    VulkanRenderEngineBackend*   m_engine;
    VulkanGraphicsEngine*        m_gEngine;
  
    uint32_t                     m_programAddr;
  
    vk::PipelineLayout           m_layout;
 
    std::vector<PushDescriptor>  m_pushDescriptors[VulkanFlightPoolSize];
 
    vk::DescriptorSetLayout      m_staticDesciptorLayout;
    vk::DescriptorPool           m_staticDescriptorPool;
    vk::DescriptorSet            m_staticDescriptorSet;

    FlareBase::ShaderBufferInput m_cameraBufferInput;
    FlareBase::ShaderBufferInput m_transformBufferInput;
    FlareBase::ShaderBufferInput m_uiBufferInput;
    FlareBase::ShaderBufferInput m_directionalLightBufferInput;
    FlareBase::ShaderBufferInput m_pointLightBufferInput;
    FlareBase::ShaderBufferInput m_spotLightBufferInput;

protected:

public:
    VulkanShaderData(VulkanRenderEngineBackend* a_engine, VulkanGraphicsEngine* a_gEngine, uint32_t a_programAddr);
    ~VulkanShaderData();

    inline vk::PipelineLayout GetLayout() const
    {
        return m_layout;
    }

    inline FlareBase::ShaderBufferInput GetCameraInput() const
    {
        return m_cameraBufferInput;
    }
    inline FlareBase::ShaderBufferInput GetDirectionalLightInput() const
    {
        return m_directionalLightBufferInput;
    }
    inline FlareBase::ShaderBufferInput GetPointLightInput() const
    {
        return m_pointLightBufferInput;
    }
    inline FlareBase::ShaderBufferInput GetSpotLightInput() const
    {
        return m_spotLightBufferInput;
    }

    void SetTexture(uint32_t a_slot, const FlareBase::TextureSampler& a_sampler) const;

    void PushTexture(vk::CommandBuffer a_commandBuffer, uint32_t a_slot, const FlareBase::TextureSampler& a_sampler, uint32_t a_index) const;
    void PushUniformBuffer(vk::CommandBuffer a_commandBuffer, uint32_t a_slot, VulkanUniformBuffer* a_buffer, uint32_t a_index) const;

    void UpdateTransformBuffer(vk::CommandBuffer a_commandBuffer, const glm::mat4& a_transform) const;
    void UpdateUIBuffer(vk::CommandBuffer a_commandBuffer, const UIElement* a_element) const;

    void Bind(uint32_t a_index, vk::CommandBuffer a_commandBuffer) const;
};