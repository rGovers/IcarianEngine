#pragma once

#include "Rendering/Vulkan/VulkanConstants.h"

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