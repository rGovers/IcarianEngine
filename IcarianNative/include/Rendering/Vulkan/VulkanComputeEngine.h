#pragma once

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN
#include "Rendering/Vulkan/VulkanConstants.h"

#include "DataTypes/TNCArray.h"

class VulkanRenderEngineBackend;
class VulkanComputePipeline;
class VulkanComputeShader;

class VulkanComputeEngine
{
private:
    VulkanRenderEngineBackend*       m_engine;
  
    TNCArray<VulkanComputeShader*>   m_shaders;
    TNCArray<VulkanComputePipeline*> m_pipelines;

protected:

public:
    VulkanComputeEngine(VulkanRenderEngineBackend* a_engine);
    ~VulkanComputeEngine();

    vk::CommandBuffer Update();

    VulkanComputeShader* GetShader(uint32_t a_shaderAddr);
};

#endif