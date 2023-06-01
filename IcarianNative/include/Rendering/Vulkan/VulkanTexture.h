#pragma once

#include "Rendering/Vulkan/VulkanConstants.h"

class VulkanRenderEngineBackend;

class VulkanTexture
{
private:
    VulkanRenderEngineBackend* m_engine;
    
    vk::Image                  m_image;
    vk::ImageView              m_view;
    VmaAllocation              m_allocation;

    uint32_t                   m_width;
    uint32_t                   m_height;

    void Init(VulkanRenderEngineBackend* a_engine, uint32_t a_width, uint32_t a_height, const void* a_data, vk::Format a_format, uint32_t a_channels);

    VulkanTexture();
protected:

public:
    ~VulkanTexture();

    static VulkanTexture* CreateRGBA(VulkanRenderEngineBackend* a_engine, uint32_t a_width, uint32_t a_height, const void* a_data);
    static VulkanTexture* CreateAlpha(VulkanRenderEngineBackend* a_engine, uint32_t a_width, uint32_t a_height, const void* a_data);

    inline uint32_t GetWidth() const
    {
        return m_width;
    }
    inline uint32_t GetHeight() const
    {
        return m_height;
    }

    inline vk::ImageView GetImageView() const
    {
        return m_view;
    }
};