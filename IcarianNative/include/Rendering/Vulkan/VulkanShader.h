#pragma once

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN
#include "Rendering/Vulkan/IcarianVulkanHeader.h"

class VulkanRenderEngineBackend;

#include "EngineMaterialInteropStructures.h"

class VulkanShader
{
private:

protected:
    VulkanRenderEngineBackend*  m_engine;
    
    vk::ShaderModule            m_module;
    
    ShaderBufferInput*          m_inputs;
    uint32_t                    m_inputCount;

    VulkanShader(VulkanRenderEngineBackend* a_engine, const ShaderBufferInput* a_inputs, uint32_t a_inputCount);
    
public:
    VulkanShader() = delete;
    virtual ~VulkanShader();

    inline uint32_t GetShaderInputCount() const
    {
        return m_inputCount;
    }
    inline ShaderBufferInput GetShaderInput(uint32_t a_index) const
    {
        return m_inputs[a_index];
    }

    inline vk::ShaderModule GetShaderModule() const
    {
        return m_module;
    }
};
#endif