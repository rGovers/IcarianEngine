#pragma once

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN
#include "Rendering/Vulkan/VulkanConstants.h"

#include <cstdint>

class VulkanGraphicsParticle2D
{
private:
    uint32_t m_computeBufferAddr;

protected:

public:
    VulkanGraphicsParticle2D(uint32_t a_computeBufferAddr);
    ~VulkanGraphicsParticle2D();
};

#endif