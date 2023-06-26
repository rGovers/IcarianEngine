#pragma once

#include "Rendering/Vulkan/VulkanConstants.h"

class VulkanRenderEngineBackend;

class VulkanDepthRenderTexture 
{
private:
    VulkanRenderEngineBackend* m_engine;

    uint32_t                   m_width;
    uint32_t                   m_height;

    vk::RenderPass             m_renderPass;
    vk::RenderPass             m_renderPassNoClear;
    vk::Framebuffer            m_frameBuffer;

    vk::Image                  m_texture;
    vk::ImageView              m_textureView;
    VmaAllocation              m_textureAllocation;

    vk::ClearValue             m_clearValue;

    void Init(uint32_t a_width, uint32_t a_height);
    void Destroy();

protected:

public:
    VulkanDepthRenderTexture(VulkanRenderEngineBackend* a_engine, uint32_t a_width, uint32_t a_height);
    ~VulkanDepthRenderTexture();

    inline uint32_t GetWidth() const
    {
        return m_width;
    }
    inline uint32_t GetHeight() const
    {
        return m_height;
    }

    inline vk::RenderPass GetRenderPass() const
    {
        return m_renderPass;
    }
    inline vk::RenderPass GetRenderPassNoClear() const
    {
        return m_renderPassNoClear;
    }
    
    inline vk::Framebuffer GetFrameBuffer() const
    {
        return m_frameBuffer;
    }
    inline vk::ClearValue GetClearValue() const
    {
        return m_clearValue;
    }

    inline vk::ImageView GetImageView() const
    {
        return m_textureView;
    }

    void Resize(uint32_t a_width, uint32_t a_height);
};