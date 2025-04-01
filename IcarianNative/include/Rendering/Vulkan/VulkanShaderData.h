// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/IcarianVulkanHeader.h"

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include "DataTypes/Array.h"
#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"

#include "EngineMaterialInteropStructures.h"
#include "EngineTextureSamplerInteropStructures.h"

class Allocator;
class UIElement;
class VulkanDecalShader;
class VulkanGraphicsEngine;
class VulkanRenderTexture;
class VulkanShaderStorageObject;
class VulkanUniformBuffer;

struct VulkanPushDescriptor
{
    uint32_t Slot;
    uint32_t Count;
    vk::DescriptorSetLayout DescriptorLayout;
};

struct VulkanShaderInput
{
    uint32_t Slot;
    uint32_t Count;
    e_ShaderBufferType BufferType;
    vk::ShaderStageFlags StageFlags;
};

struct VulkanTextureBinding
{
    uint32_t Slot;
    uint32_t SamplerAddr;
};

class VulkanShaderData
{
private:
    static constexpr uint32_t StaticIndex = 0;

    Allocator*                                    m_allocator;

    VulkanRenderEngineBackend*                    m_engine;
    VulkanGraphicsEngine*                         m_gEngine;
 
    VulkanUniformBuffer*                          m_userUniformBuffer;
 
    vk::PipelineLayout                            m_layout;
    vk::PipelineLayout                            m_shadowLayout;

    Array<VulkanPushDescriptor, RenderBlockAlloc> m_pushDescriptors;
    Array<VulkanPushDescriptor, RenderBlockAlloc> m_shadowPushDescriptors;

    e_MaterialMode                                m_materialMode;

    uint32_t                                      m_slotInputCount;
    uint32_t                                      m_shadowSlotInputCount;
    VulkanShaderInput*                            m_slotInputs;
    VulkanShaderInput*                            m_shadowSlotInputs;

    Array<VulkanTextureBinding, RenderBlockAlloc> m_textures;

protected:

public:
    VulkanShaderData(VulkanRenderEngineBackend* a_engine, VulkanGraphicsEngine* a_gEngine, const RenderProgram& a_program, Allocator* a_allocator);
    ~VulkanShaderData();

    inline vk::PipelineLayout GetLayout() const
    {
        return m_layout;
    }
    inline vk::PipelineLayout GetShadowLayout() const
    {
        return m_shadowLayout;
    }

    inline e_MaterialMode GetMaterialMode() const
    {
        return m_materialMode;
    }

    Array<ShaderBufferInput, RenderScratchAlloc> GetShaderBufferInputs(e_ShaderBufferType a_type) const;

    bool GetShaderBufferInput(e_ShaderBufferType a_type, ShaderBufferInput* a_input) const;
    bool GetShadowShaderBufferInput(e_ShaderBufferType a_type, ShaderBufferInput* a_input) const;

    void SetTexture(uint32_t a_slot, uint32_t a_sampleAddr);

    void PushComputeBufferTexture(vk::CommandBuffer a_commandBuffer, uint32_t a_slot, const VulkanRenderTexture* a_renderTexture, uint32_t a_index) const;

    void PushTexture(vk::CommandBuffer a_commandBuffer, uint32_t a_slot, const TextureSamplerBuffer& a_sampler, uint32_t a_index) const;
    void PushTextures(vk::CommandBuffer a_commandBuffer, uint32_t a_slot, const TextureSamplerBuffer* a_samplers, uint32_t a_count, uint32_t a_index) const;
    void PushUniformBuffer(vk::CommandBuffer a_commandBuffer, uint32_t a_slot, const VulkanUniformBuffer* a_buffer, uint32_t a_index) const;
    void PushShaderStorageObject(vk::CommandBuffer a_commandBuffer, uint32_t a_slot, const VulkanShaderStorageObject* a_object, uint32_t a_index) const;
    void PushShaderStorageObject(vk::CommandBuffer a_commandBuffer, uint32_t a_slot, vk::Buffer a_object, uint32_t a_index) const;

    void PushShadowTexture(vk::CommandBuffer a_commandBuffer, uint32_t a_slot, const TextureSamplerBuffer& a_sampler, uint32_t a_index) const;
    void PushShadowUniformBuffer(vk::CommandBuffer a_commandBuffer, uint32_t a_slot, const VulkanUniformBuffer* a_buffer, uint32_t a_index) const;
    void PushShadowShaderStorageObject(vk::CommandBuffer a_commandBuffer, uint32_t a_slot, const VulkanShaderStorageObject* a_object, uint32_t a_index) const;

    void UpdateTransformBuffer(vk::CommandBuffer a_commandBuffer, const glm::mat4& a_transform) const;
    void UpdateShadowTransformBuffer(vk::CommandBuffer a_commandBuffer, const glm::mat4& a_transform) const;
    
    void UpdateUIBuffer(vk::CommandBuffer a_commandBuffer, const UIElement* a_element) const;

    void UpdateShadowLightBuffer(vk::CommandBuffer a_commandBuffer, const glm::mat4& a_lvp, float a_split) const;

    void Update(uint32_t a_index, const RenderProgram& a_program);
    void Bind(uint32_t a_index, vk::CommandBuffer a_commandBuffer) const;
    void BindShadow(uint32_t a_index, vk::CommandBuffer a_commandBuffer) const;
    void BindCompute(uint32_t a_index, vk::CommandBuffer a_commandBuffer) const;
    void UnbindCompute(uint32_t a_index, vk::CommandBuffer a_commandBuffer) const;
};

#endif

// MIT License
// 
// Copyright (c) 2025 River Govers
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