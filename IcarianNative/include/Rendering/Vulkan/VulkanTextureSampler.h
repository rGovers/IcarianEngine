#pragma once

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN
#include "Rendering/Vulkan/IcarianVulkanHeader.h"

#include "EngineTextureSamplerInteropStructures.h"

class VulkanGraphicsEngine;
class VulkanRenderEngineBackend;

class VulkanTextureSampler
{
private:
    VulkanRenderEngineBackend* m_engine;

    vk::Sampler                m_sampler;

    VulkanTextureSampler(VulkanRenderEngineBackend* a_engine);

protected:

public:
    ~VulkanTextureSampler();
    
    static VulkanTextureSampler* GenerateFromBuffer(VulkanRenderEngineBackend* a_engine, VulkanGraphicsEngine* a_gEngine, const TextureSamplerBuffer& a_sampler);

    inline vk::Sampler GetSampler() const
    {
        return m_sampler;
    }
};
#endif