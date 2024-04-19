#pragma once

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN
#include "Rendering/Vulkan/IcarianVulkanHeader.h"

class VulkanRenderEngineBackend;

class VulkanDepthCubeRenderTexture
{
private:
    VulkanRenderEngineBackend* m_engine;

    uint32_t                   m_width;
    uint32_t                   m_height;

    vk::RenderPass             m_renderPass;
    vk::RenderPass             m_renderPassNoClear;
    vk::Framebuffer            m_frameBuffer[6];

    vk::Image                  m_texture;
    vk::ImageView              m_textureView;
    vk::ImageView              m_textureViewFramebuffer[6];
    VmaAllocation              m_textureAllocation;

    vk::ClearValue             m_clearValue;

    void Init(uint32_t a_width, uint32_t a_height);

protected:

public:
    VulkanDepthCubeRenderTexture(VulkanRenderEngineBackend* a_engine, uint32_t a_width, uint32_t a_height);
    ~VulkanDepthCubeRenderTexture();

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
    inline vk::Framebuffer GetFrameBuffer(uint32_t a_index) const
    {
        return m_frameBuffer[a_index];
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

#endif