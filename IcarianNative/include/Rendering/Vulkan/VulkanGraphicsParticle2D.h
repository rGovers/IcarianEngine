#pragma once

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN
#include "Rendering/Vulkan/VulkanConstants.h"

#include <cstdint>

class VulkanComputeEngine;
class VulkanGraphicsEngine;
class VulkanRenderEngineBackend;

#include "EngineMaterialInteropStructures.h"

class VulkanGraphicsParticle2D
{
private:
    VulkanRenderEngineBackend*     m_backend;
    VulkanComputeEngine*           m_cEngine;
    VulkanGraphicsEngine*          m_gEngine;

    std::vector<ShaderBufferInput> m_inputs;

    uint32_t                       m_computeBufferAddr;
    uint32_t                       m_renderProgramAddr;

protected:

public:
    VulkanGraphicsParticle2D(VulkanRenderEngineBackend* a_backend, VulkanComputeEngine* a_cEngine, VulkanGraphicsEngine* a_gEngine, uint32_t a_computeBufferAddr);
    ~VulkanGraphicsParticle2D();

    void Update(uint32_t a_index, uint32_t a_bufferIndex, vk::CommandBuffer a_commandBuffer, uint32_t a_renderTextureAddr);
};

#endif