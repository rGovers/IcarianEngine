#pragma once

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include <cstdint>

class VulkanComputeEngine;

#include "EngineParticleSystemInteropStructures.h"

class VulkanComputeEngineBindings
{
private:
    VulkanComputeEngine* m_engine;

protected:

public:
    VulkanComputeEngineBindings(VulkanComputeEngine* a_engine);
    ~VulkanComputeEngineBindings();

    uint32_t GenerateParticleSystemBuffer() const;
    void DestroyParticleSystemBuffer(uint32_t a_addr) const;
    ComputeParticleBuffer GetParticleSystemBuffer(uint32_t a_addr) const;
    void SetParticleSystemBuffer(uint32_t a_addr, const ComputeParticleBuffer& a_buffer) const;

    uint32_t GenerateParticleSystem(uint32_t a_particleBufferAddr) const;
    void DestroyParticleSystem(uint32_t a_addr) const;
};

#endif