#pragma once

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN
#include "Rendering/Vulkan/IcarianVulkanHeader.h"

#include <vector>

class VulkanRenderEngineBackend;

#include "EngineMaterialInteropStructures.h"

class VulkanComputeLayout
{
private:
    VulkanRenderEngineBackend* m_engine;

    vk::PipelineLayout         m_layout;

    uint32_t                   m_inputCount;
    vk::DescriptorSetLayout*   m_descLayouts;
    ShaderBufferInput*         m_slotInputs;

protected:

public:
    VulkanComputeLayout(VulkanRenderEngineBackend* a_engine, const ShaderBufferInput* a_inputs, uint32_t a_inputCount);
    ~VulkanComputeLayout();

    inline uint32_t GetInputCount() const
    {
        return m_inputCount;
    }

    inline const vk::DescriptorSetLayout* GetDescriptorLayouts() const
    {
        return m_descLayouts;
    }   
    inline const ShaderBufferInput* GetShaderInputs() const
    {
        return m_slotInputs;
    }

    inline vk::PipelineLayout GetLayout() const
    {
        return m_layout;
    }
};

#endif