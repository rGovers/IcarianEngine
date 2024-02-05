#pragma once

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN
#include "Rendering/Vulkan/VulkanConstants.h"

#include <cstdint>

class VulkanComputeEngine;

#include "EngineParticleSystemInteropStructures.h"

class VulkanComputeParticle2D
{
public:
    static constexpr uint32_t MaxParticleBuffers = 2;

private:
    VulkanComputeEngine*           m_engine;
        
    uint32_t                       m_computeShader;
    uint32_t                       m_computeLayout;
    uint32_t                       m_computePipeline;
    uint32_t                       m_particleBufferAddr;

    uint32_t                       m_bufferIndex;
    VmaAllocation                  m_allocations[MaxParticleBuffers];
    vk::Buffer                     m_particleBuffers[MaxParticleBuffers];

    void Clear();
    void Rebuild(ComputeParticleBuffer* a_buffer);

protected:

public:
    VulkanComputeParticle2D(VulkanComputeEngine* a_engine, uint32_t a_particleBufferAddr);
    ~VulkanComputeParticle2D();

    void Update(vk::CommandBuffer a_cmdBuffer, uint32_t a_index);
};

#endif