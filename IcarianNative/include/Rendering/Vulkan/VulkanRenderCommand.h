// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/IcarianVulkanHeader.h"

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include "Core/Bitfield.h"

#include "EngineLightInteropStructures.h"
#include "EngineRenderCommandInteropStructures.h"
#include "EngineTextureSamplerInteropStructures.h"

class VulkanGraphicsEngine;
class VulkanPipeline;
class VulkanRenderEngineBackend;
class VulkanRenderTexture;
class VulkanSwapchain;

class VulkanRenderCommand
{
private:
    constexpr static uint32_t FlushedBit = 0;

    VulkanRenderEngineBackend* m_engine;
    VulkanGraphicsEngine*      m_gEngine;
    VulkanSwapchain*           m_swapchain;

    uint32_t                   m_bufferIndex;

    uint32_t                   m_renderTexAddr;
    uint32_t                   m_materialAddr;
    uint32_t                   m_cameraAddr;

    vk::CommandBuffer          m_commandBuffer;

    uint8_t                    m_flags;

    void SetFlushedState(bool a_value);

protected:

public:
    VulkanRenderCommand(VulkanRenderEngineBackend* a_engine, VulkanGraphicsEngine* a_gEngine, VulkanSwapchain* a_swapchain, vk::CommandBuffer a_buffer, uint32_t a_camAddr, uint32_t a_bufferIndex);
    ~VulkanRenderCommand();

    inline bool IsFlushed() const
    {
        return IISBITSET(m_flags, FlushedBit);
    }

    void Flush();

    inline uint32_t GetRenderTexutreAddr() const
    {
        return m_renderTexAddr;
    }
    VulkanRenderTexture* GetRenderTexture() const;

    inline uint32_t GetMaterialAddr() const
    {
        return m_materialAddr;
    }
    VulkanPipeline* GetPipeline() const;

    inline vk::CommandBuffer GetCommandBuffer() const
    {
        return m_commandBuffer;
    }

    VulkanPipeline* BindMaterial(uint32_t a_materialAddr);

    void PushTexture(uint32_t a_slot, const TextureSamplerBuffer& a_sampler) const;
    void PushLight(uint32_t a_slot, e_LightType a_lightType, uint32_t a_lightAddr) const;
    void PushLightSplits(uint32_t a_slot, const LightShadowSplit* a_splits, uint32_t a_splitCount) const;
    void PushShadowTextureArray(uint32_t a_slot, uint32_t a_dirLightAddr) const;

    void BindRenderTexture(uint32_t a_renderTexAddr, e_RenderTextureBindMode a_bindMode);
    
    void Blit(const VulkanRenderTexture* a_src, const VulkanRenderTexture* a_dst);
    void Blit(const VulkanRenderTexture* a_src, uint32_t a_index, const VulkanRenderTexture* a_dst);

    void DrawMaterial();
    void DrawModel(const glm::mat4& a_transform, uint32_t a_addr);

    void MarkerStart(const std::string_view& a_name);
    void MarkerEnd();
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