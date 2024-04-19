#pragma once

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN
#include "Rendering/Vulkan/VulkanConstants.h"

#include "Rendering/TextureData.h"

class VulkanRenderEngineBackend;

class VulkanTexture
{
private:
    VulkanRenderEngineBackend* m_engine;
    
    vk::Format                 m_format;
    uint32_t                   m_channels;
    vk::Image                  m_image;
    vk::ImageView              m_imageView;
    VmaAllocation              m_allocation;

    uint32_t                   m_width;
    uint32_t                   m_height;

    void InitBase(const void* a_data, vk::Format a_format, uint32_t a_channels, uint64_t a_dataSize);
    void InitMipMapped(uint32_t a_levels, const uint64_t* a_offsets, const void* a_data, vk::Format a_format, uint32_t a_channels, uint64_t a_dataSize);

    VulkanTexture();

protected:

public:
    ~VulkanTexture();

    static VulkanTexture* CreateTexture(VulkanRenderEngineBackend* a_engine, uint32_t a_width, uint32_t a_height, e_TextureFormat a_format, const void* a_data, uint64_t a_dataSize = -1);
    static VulkanTexture* CreateTextureMipMapped(VulkanRenderEngineBackend* a_engine, uint32_t a_width, uint32_t a_height, uint32_t a_levels, const uint64_t* a_offsets, e_TextureFormat a_format, const void* a_data, uint64_t a_dataSize);

    void WriteData(const void* a_data, bool a_init = false);

    inline vk::Format GetFormat() const
    {
        return m_format;
    }

    inline vk::Image GetImage() const
    {
        return m_image;
    }
    inline vk::ImageView GetImageView() const
    {
        return m_imageView;
    }

    inline uint32_t GetWidth() const
    {
        return m_width;
    }
    inline uint32_t GetHeight() const
    {
        return m_height;
    }
};
#endif