#pragma once

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN
#include "Rendering/Vulkan/IcarianVulkanHeader.h"

class VulkanRenderEngineBackend;

class VulkanShader
{
private:

protected:
    VulkanRenderEngineBackend* m_engine = nullptr;

    vk::ShaderModule           m_module = nullptr;
    
    VulkanShader(VulkanRenderEngineBackend* a_engine) 
    { 
        m_engine = a_engine;
    }
public:
    VulkanShader() = delete;
    virtual ~VulkanShader() { }

    inline vk::ShaderModule GetShaderModule() const
    {
        return m_module;
    }
};
#endif