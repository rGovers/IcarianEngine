#pragma once

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN
#include "Rendering/Vulkan/IcarianVulkanHeader.h"

#include <cstdint>

class VulkanComputeEngine;
class VulkanGraphicsEngine;
class VulkanRenderEngineBackend;

#include "DataTypes/Array.h"

#include "EngineMaterialInteropStructures.h"

class VulkanGraphicsParticle2D
{
private:
    VulkanRenderEngineBackend* m_backend;
    VulkanComputeEngine*       m_cEngine;
    VulkanGraphicsEngine*      m_gEngine;

    Array<ShaderBufferInput>   m_inputs;

    uint32_t                   m_computeBufferAddr;
    uint32_t                   m_renderProgramAddr;

    void Build();
    void Destroy();

protected:

public:
    VulkanGraphicsParticle2D(VulkanRenderEngineBackend* a_backend, VulkanComputeEngine* a_cEngine, VulkanGraphicsEngine* a_gEngine, uint32_t a_computeBufferAddr);
    ~VulkanGraphicsParticle2D();

    void Update(uint32_t a_index, uint32_t a_bufferIndex, vk::CommandBuffer a_commandBuffer, uint32_t a_renderTextureAddr);
};

#endif