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

protected:

public:
    VulkanTexture(VulkanRenderEngineBackend* a_engine, uint32_t a_width, uint32_t a_height, const void* a_data);
    ~VulkanTexture();

    inline vk::ImageView GetImageView() const
    {
        return m_view;
    }
};