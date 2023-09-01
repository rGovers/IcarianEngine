#pragma once

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN
#include <cstdint>

enum e_VulkanLightRenderTextureType : uint16_t
{
    VulkanLightRenderTextureType_Null = 0,
    VulkanLightRenderTextureType_RenderTexture = 1,
    VulkanLightRenderTextureType_DepthRenderTexture = 2,
};

struct VulkanLightRenderTexture
{
    uint32_t TextureAddr;
    e_VulkanLightRenderTextureType Type;
};

struct VulkanLightBuffer
{
    uint32_t LightRenderTextureCount;
    VulkanLightRenderTexture* LightRenderTextures;
};
#endif