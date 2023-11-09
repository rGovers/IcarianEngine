#pragma once

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN
#include <cstdint>

struct VulkanLightBuffer
{
    uint32_t LightRenderTextureCount;
    uint32_t* LightRenderTextures;
};
#endif