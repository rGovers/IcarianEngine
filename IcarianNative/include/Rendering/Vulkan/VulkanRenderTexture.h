#pragma once

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN
#include "Rendering/Vulkan/VulkanConstants.h"

class VulkanRenderEngineBackend;

class VulkanRenderTexture
{
private:
    static constexpr int HDRFlag = 0;
    static constexpr int DepthTextureFlag = 1;

    VulkanRenderEngineBackend* m_engine;

    uint32_t                   m_textureCount;
                    
    uint32_t                   m_width;
    uint32_t                   m_height;

    unsigned char              m_flags;

    vk::RenderPass             m_renderPass;
    vk::RenderPass             m_renderPassNoClear;
    vk::Framebuffer            m_frameBuffer;

    vk::Image*                 m_textures;
    vk::ImageView*             m_textureViews;
    VmaAllocation*             m_textureAllocations;

    vk::ClearValue*            m_clearValues;
    
    void Init(uint32_t a_width, uint32_t a_height);

protected:

public:
    VulkanRenderTexture(VulkanRenderEngineBackend* a_engine, uint32_t a_textureCount, uint32_t a_width, uint32_t a_height, bool a_depthTexture, bool a_hdr);
    ~VulkanRenderTexture();

    inline uint32_t GetWidth() const
    {
        return m_width;
    }
    inline uint32_t GetHeight() const
    {
        return m_height;
    }

    inline bool IsHDR() const
    {
        return m_flags & 0b1 << HDRFlag;
    }
    inline bool HasDepthTexture() const
    {
        return m_flags & 0b1 << DepthTextureFlag;
    }

    inline uint32_t GetTextureCount() const
    {
        return m_textureCount;
    }
    inline uint32_t GetTotalTextureCount() const
    {
        if (HasDepthTexture())
        {
            return m_textureCount + 1;
        }

        return m_textureCount;
    }

    inline vk::ImageView GetImageView(uint32_t a_index) const
    {
        return m_textureViews[a_index];
    }
    inline vk::ImageView GetDepthImageView() const
    {
        return m_textureViews[m_textureCount];
    }
    inline vk::ImageView* GetImageViews() const
    {
        return m_textureViews;
    }

    inline vk::Image GetTexture(uint32_t a_index) const
    {
        return m_textures[a_index];
    }
    inline vk::Image GetDepthTexture() const
    {
        return m_textures[m_textureCount];
    }
    inline vk::Image* GetTextures() const
    {
        return m_textures;
    }

    inline vk::RenderPass GetRenderPass() const
    {
        return m_renderPass;
    }
    inline vk::RenderPass GetRenderPassNoClear() const
    {
        return m_renderPassNoClear;
    }

    inline vk::Framebuffer GetFramebuffer() const
    {
        return m_frameBuffer;
    }

    inline vk::ClearValue* GetClearValues() const
    {
        return m_clearValues;
    }

    void Resize(uint32_t a_width, uint32_t a_height);
};
#endif