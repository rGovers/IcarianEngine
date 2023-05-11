#pragma once

#include "Rendering/Vulkan/VulkanConstants.h"

#include "Flare/TextureSampler.h"

class VulkanRenderEngineBackend;

class VulkanTextureSampler
{
private:
    VulkanRenderEngineBackend* m_engine;

    vk::Sampler                m_sampler;

protected:

public:
    VulkanTextureSampler(VulkanRenderEngineBackend* a_engine, const FlareBase::TextureSampler& a_sampler);
    ~VulkanTextureSampler();
    
    inline vk::Sampler GetSampler() const
    {
        return m_sampler;
    }
};