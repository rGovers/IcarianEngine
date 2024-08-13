// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN
#include "Rendering/Vulkan/IcarianVulkanHeader.h"

#include "Core/Bitfield.h"

class VulkanGraphicsEngine;
class VulkanRenderEngineBackend;

class VulkanRenderTexture
{
private:
    static constexpr uint32_t HDRFlag = 0;
    static constexpr uint32_t OwnsDepthTextureFlag = 1;

    VulkanRenderEngineBackend* m_engine;
    VulkanGraphicsEngine*      m_gEngine;

    uint32_t                   m_textureCount;
    uint32_t                   m_channelCount;

    uint32_t                   m_width;
    uint32_t                   m_height;

    uint32_t                   m_depthHandle;

    vk::RenderPass             m_renderPass;
    vk::RenderPass             m_renderPassColorClear;
    vk::RenderPass             m_renderPassNoClear;
    vk::Framebuffer            m_frameBuffer;

    vk::Image*                 m_textures;
    vk::ImageView*             m_textureViews;
    VmaAllocation*             m_textureAllocations;

    vk::ClearValue*            m_clearValues;

    uint8_t                    m_flags;
    
    void Setup();
    void Init(uint32_t a_width, uint32_t a_height);

protected:

public:
    VulkanRenderTexture(VulkanRenderEngineBackend* a_engine, VulkanGraphicsEngine* a_gEngine, uint32_t a_textureCount, uint32_t a_width, uint32_t a_height, bool a_depthTexture, bool a_hdr, uint32_t a_channelCount);
    VulkanRenderTexture(VulkanRenderEngineBackend* a_engine, VulkanGraphicsEngine* a_gEngine, uint32_t a_textureCount, uint32_t a_width, uint32_t a_height, uint32_t a_depthHandle, bool a_hdr, uint32_t a_channelCount);
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
        return IISBITSET(m_flags, HDRFlag);
    }
    inline bool HasDepthTexture() const
    {
        return m_depthHandle != -1;
    }

    inline uint32_t GetTextureCount() const
    {
        return m_textureCount;
    }
    inline uint32_t GetTotalTextureCount() const
    {
        return m_textureCount + HasDepthTexture();
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
    inline vk::RenderPass GetRenderPassColorClear() const
    {
        return m_renderPassColorClear;
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

// MIT License
// 
// Copyright (c) 2024 River Govers
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.