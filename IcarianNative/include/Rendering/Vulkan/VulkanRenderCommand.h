// Icarian Engine - C# Game Engine
//
// License at end of file.

#pragma once

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/IcarianVulkanHeader.h"

#define GLM_FORCE_SWIZZLE
#include <glm/glm.hpp>

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
    constexpr static uint32_t RenderTextureBoundBit = 0;
    constexpr static uint32_t RenderTextureFirstBindBit = 1;
    constexpr static uint32_t MaterialBoundBit = 2;
    constexpr static uint32_t ComputeLayoutBit = 3;

    VulkanRenderEngineBackend* m_engine;
    VulkanGraphicsEngine*      m_gEngine;
    VulkanSwapchain*           m_swapchain;

    uint32_t                   m_bufferIndex;

    uint32_t                   m_renderTexAddr;
    uint32_t                   m_materialAddr;
    uint32_t                   m_cameraAddr;

    vk::CommandBuffer          m_commandBuffer;

    e_RenderTextureBindMode    m_renderBindMode;

    uint8_t                    m_flags;

    void SetRenderTextureCompute();
    void ClearRenderTextureCompute();

    void BindRenderTexturePass();

    bool BindResources(IcarianCore::Allocator* a_tempAllocator);

protected:

public:
    VulkanRenderCommand(VulkanRenderEngineBackend* a_engine, VulkanGraphicsEngine* a_gEngine, VulkanSwapchain* a_swapchain, vk::CommandBuffer a_buffer, uint32_t a_camAddr, uint32_t a_bufferIndex);
    ~VulkanRenderCommand();

    inline bool IsFlushed() const
    {
        return m_renderTexAddr == uint32_t(-1) && m_materialAddr == uint32_t(-1);
    }

    vk::RenderPassBeginInfo GetRenderPassInfo() const;

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

    VulkanPipeline* BindMaterial(uint32_t a_materialAddr, bool a_immediate, IcarianCore::Allocator* a_tempAllocator);

    void PushTexture(uint32_t a_slot, const TextureSamplerBuffer& a_sampler, IcarianCore::Allocator* a_tempAllocator);
    void PushLight(uint32_t a_slot, e_LightType a_lightType, uint32_t a_lightAddr, IcarianCore::Allocator* a_tempAllocator);
    void PushLightSplits(uint32_t a_slot, const LightShadowSplit* a_splits, uint32_t a_splitCount, IcarianCore::Allocator* a_tempAllocator);
    void PushShadowTextureArray(uint32_t a_slot, uint32_t a_dirLightAddr, IcarianCore::Allocator* a_tempAllocator);

    void PushUserTexture(uint32_t a_slot, const TextureSamplerBuffer& a_sampler, IcarianCore::Allocator* a_tempAllocator);
    void PushUserLight(uint32_t a_slot, e_LightType a_lightType, uint32_t a_lightAddr, IcarianCore::Allocator* a_tempAllocator);
    void PushUserLightSplits(uint32_t a_slot, const LightShadowSplit* a_splits, uint32_t a_splitCount, IcarianCore::Allocator* a_tempAllocator);
    void PushUserShadowTextureArray(uint32_t a_slot, uint32_t a_dirLightAddr, IcarianCore::Allocator* a_tempAllocator);

    void BindRenderTexture(uint32_t a_renderTexAddr, e_RenderTextureBindMode a_bindMode);

    void Blit(const VulkanRenderTexture* a_src, const VulkanRenderTexture* a_dst);
    void Blit(const VulkanRenderTexture* a_src, uint32_t a_index, const VulkanRenderTexture* a_dst);

    void DrawMaterial(IcarianCore::Allocator* a_tempAllocator);
    void DrawModel(const glm::mat4& a_transform, uint32_t a_modelAddr, IcarianCore::Allocator* a_tempAllocator);
    void DrawMesh(const glm::mat4& a_transform, uint32_t a_meshAddr, uint32_t a_indexCount, IcarianCore::Allocator* a_tempAllocator);

    void MarkerStart(const IcarianCore::COWU8String& a_name);
    void MarkerEnd();
};

#endif

// MIT License
//
// Copyright (c) 2026 River Govers
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
