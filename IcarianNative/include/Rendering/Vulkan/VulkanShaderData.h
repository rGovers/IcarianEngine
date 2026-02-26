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
class VulkanGraphicsEngine;
class VulkanMesh;
class VulkanRenderTexture;
class VulkanShader;
class VulkanShaderStorageObject;
class VulkanUniformBuffer;

struct VulkanPushDescriptor
{
    vk::DescriptorSetLayout DescriptorLayout;
    uint32_t Count;
    uint16_t RealSlot;
    uint16_t UserSlot;
};

struct VulkanShaderInput
{
    vk::ShaderStageFlags StageFlags;
    e_ShaderBufferType BufferType;
    uint16_t Count;
    uint16_t RealSlot;
    uint16_t UserSlot;
};

struct VulkanTextureBinding
{
    uint32_t Slot;
    uint32_t SamplerAddr;
};

struct VulkanBaseShaderDataBuilder
{
    VulkanRenderEngineBackend* Engine;
    VulkanGraphicsEngine* GraphicsEngine;

    RenderProgram Program;
};

struct VulkanShadowShaderDataBuilder
{
    VulkanRenderEngineBackend* Engine;
    VulkanGraphicsEngine* GraphicsEngine;

    RenderProgram Program;
};

struct VulkanComputeMeshShaderDataBuilder
{
    VulkanRenderEngineBackend* Engine;
    VulkanGraphicsEngine* GraphicsEngine;

    RenderProgram Program;
};

class VulkanShaderData
{
private:
    static constexpr uint32_t StaticIndex = 0;

    Allocator*                  m_allocator;

    VulkanRenderEngineBackend*  m_engine;
    VulkanGraphicsEngine*       m_gEngine;

    VulkanUniformBuffer*        m_userUniformBuffer;
    VulkanShaderStorageObject*  m_userArray;
    VulkanShader**              m_shaders;
    VulkanShaderInput*          m_slotInputs;
    VulkanPushDescriptor*       m_pushDescriptors;
    VertexInputAttribute*       m_attributes;

    vk::PipelineLayout          m_layout;

    Array<VulkanTextureBinding> m_textures;

    uint32_t                    m_shaderCount;
    uint32_t                    m_slotInputCount;
    uint32_t                    m_pushDesciptorCount;
    uint32_t                    m_attributeCount;

    e_MaterialMode              m_materialMode;

    VulkanShaderData(Allocator* a_allocator);

protected:

public:
    ~VulkanShaderData();

    static void CreateBaseShaderData(VulkanShaderData* a_data, const VulkanBaseShaderDataBuilder& a_builder, Allocator* a_allocator, Allocator* a_tempAllocator);
    static void CreateComputeMeshShaderData
    (
        VulkanShaderData* a_data,
        const VulkanComputeMeshShaderDataBuilder& a_builder,
        Allocator* a_allocator,
        Allocator* a_tempAllocator
    );
    static void CreateShadowShaderData(VulkanShaderData* a_data, const VulkanShadowShaderDataBuilder& a_builder, Allocator* a_allocator, Allocator* a_tempAllocator);

    inline vk::PipelineLayout GetLayout() const
    {
        return m_layout;
    }

    inline e_MaterialMode GetMaterialMode() const
    {
        return m_materialMode;
    }

    uint16_t UserSlotToReal(uint16_t a_userSlot) const;

    uint32_t GetAttributeCount() const
    {
        return m_attributeCount;
    }
    VertexInputAttribute GetAttribute(uint32_t a_index) const;

    uint32_t GetShaderCount() const
    {
        return m_shaderCount;
    }
    VulkanShader* GetShader(uint32_t a_index) const;

    Array<ShaderBufferInput> GetShaderBufferInputs(e_ShaderBufferType a_type, Allocator* a_allocator) const;

    bool GetShaderBufferInput(e_ShaderBufferType a_type, ShaderBufferInput* a_input) const;

    void SetTexture(uint16_t a_slot, uint32_t a_sampleAddr);

    bool PushComputeBufferTexture(vk::CommandBuffer a_commandBuffer, uint16_t a_slot, const VulkanRenderTexture* a_renderTexture, uint32_t a_index) const;

    bool PushTexture(vk::CommandBuffer a_commandBuffer, uint16_t a_slot, const TextureSamplerBuffer& a_sampler, uint32_t a_index) const;
    bool PushTextures
    (
        vk::CommandBuffer a_commandBuffer,
        uint16_t a_slot,
        const TextureSamplerBuffer* a_samplers,
        uint16_t a_count,
        uint32_t a_index,
        Allocator* a_tempAllocator
    ) const;
    bool PushUniformBuffer(vk::CommandBuffer a_commandBuffer, uint16_t a_slot, const VulkanUniformBuffer* a_buffer, uint32_t a_index) const;
    bool PushShaderStorageObject(vk::CommandBuffer a_commandBuffer, uint16_t a_slot, const VulkanShaderStorageObject* a_object, uint32_t a_index) const;
    bool PushShaderStorageObject(vk::CommandBuffer a_commandBuffer, uint16_t a_slot, vk::Buffer a_object, vk::DeviceSize a_offset, uint32_t a_index) const;

    void PushMeshBuffers(vk::CommandBuffer a_commandBuffer, const VulkanMesh* a_mesh, uint32_t a_index) const;

    void UpdateTransformBuffer(vk::CommandBuffer a_commandBuffer, const glm::mat4& a_transform) const;

    void UpdateUIBuffer(vk::CommandBuffer a_commandBuffer, const UIElement* a_element) const;

    void UpdateShadowLightBuffer(vk::CommandBuffer a_commandBuffer, const glm::mat4& a_lvp, float a_split) const;

    void Update(uint32_t a_index, const RenderProgram& a_program);

    bool Bind(uint32_t a_index, vk::CommandBuffer a_commandBuffer) const;
    void Unbind(uint32_t a_index, vk::CommandBuffer a_commandBuffer) const;
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
