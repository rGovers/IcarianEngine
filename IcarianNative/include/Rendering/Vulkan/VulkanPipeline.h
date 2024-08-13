// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/IcarianVulkanHeader.h"

class VulkanGraphicsEngine;
class VulkanRenderEngineBackend;
class VulkanRenderPass;
class VulkanShaderData;

enum e_VulkanPipelineType
{
    VulkanPipelineType_Graphics,
    VulkanPipelineType_Shadow
};

class VulkanPipeline
{
private:
    VulkanRenderEngineBackend* m_engine;
    VulkanGraphicsEngine*      m_gEngine;

    uint32_t                   m_programAddr;
    
    vk::Pipeline               m_pipeline;

    e_VulkanPipelineType       m_type;

    VulkanPipeline(vk::Pipeline a_pipeline, VulkanRenderEngineBackend* a_engine, VulkanGraphicsEngine* a_gEngine, uint32_t a_programAddr, e_VulkanPipelineType a_type);
    
protected:

public:
    ~VulkanPipeline();

    inline e_VulkanPipelineType GetType() const
    {
        return m_type;
    }

    inline vk::Pipeline GetPipeline() const
    {
        return m_pipeline;
    }

    VulkanShaderData* GetShaderData() const;

    void Bind(uint32_t a_index, vk::CommandBuffer a_commandBuffer) const;

    static VulkanPipeline* CreatePipeline(VulkanRenderEngineBackend* a_engine, VulkanGraphicsEngine* a_gEngine, const vk::RenderPass& a_renderPass, bool a_depth, uint32_t a_textureCount, uint32_t a_programAddr);
    static VulkanPipeline* CreateShadowPipeline(VulkanRenderEngineBackend* a_engine, VulkanGraphicsEngine* a_gEngine, const vk::RenderPass& a_renderPass, uint32_t a_programAddr);
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