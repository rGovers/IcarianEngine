// Icarian Engine - C# Game Engine
// 
// License at end of file.

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