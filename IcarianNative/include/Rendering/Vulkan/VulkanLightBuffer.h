#pragma once

#include <cstdint>

enum e_VulkanLightRenderTextureType : uint16_t
{
    VulkanRenderTextureType_Null = 0,
    VulkanRenderTextureType_RenderTexture = 1,
    VulkanRenderTextureType_DepthRenderTexture = 2,
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