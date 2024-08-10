#pragma once

#include "DataTypes/SpinLock.h"
#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/IcarianVulkanHeader.h"

#include <cstdint>

class VulkanComputeEngine;
class VulkanGraphicsEngine;
class VulkanRenderEngineBackend;

#include "DataTypes/Array.h"

#include "EngineMaterialInteropStructures.h"
#include "EngineParticleSystemInteropStructures.h"

class VulkanGraphicsParticle2D
{
private:
    VulkanRenderEngineBackend* m_backend;
    VulkanComputeEngine*       m_cEngine;
    VulkanGraphicsEngine*      m_gEngine;

    Array<ShaderBufferInput>   m_inputs;

    uint32_t                   m_computeBufferAddr;
    uint32_t                   m_renderProgramAddr;

    SpinLock                   m_lock;

    void Build(const ComputeParticleBuffer& a_buffer);
    void Destroy();

protected:

public:
    VulkanGraphicsParticle2D(VulkanRenderEngineBackend* a_backend, VulkanComputeEngine* a_cEngine, VulkanGraphicsEngine* a_gEngine, uint32_t a_computeBufferAddr);
    ~VulkanGraphicsParticle2D();

    void Update(uint32_t a_index, uint32_t a_bufferIndex, uint32_t a_renderLayer, vk::CommandBuffer a_commandBuffer, uint32_t a_renderTextureAddr);
};

#endif