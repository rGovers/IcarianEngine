// Icarian Engine - C# Game Engine
// 
// License at end of file.

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/VulkanComputeLayout.h"

#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"
#include "Trace.h"

class VulkanComputeLayoutDeletionObject : public VulkanDeletionObject
{
private:
    VulkanRenderEngineBackend* m_engine;

    uint32_t                   m_layoutCount;
    vk::DescriptorSetLayout*   m_descLayouts;
    vk::PipelineLayout         m_layout;

protected:

public:
    VulkanComputeLayoutDeletionObject
    (
        VulkanRenderEngineBackend* a_engine,
        const vk::DescriptorSetLayout* a_descLayouts,
        uint32_t a_layoutCount, 
        vk::PipelineLayout a_layout
    )
    {
        m_engine = a_engine;

        m_layoutCount = a_layoutCount;
        m_layout = a_layout;

        Allocator* allocator = m_engine->GetDeletionAllocator();

        m_descLayouts = allocator->TAllocate<vk::DescriptorSetLayout>(m_layoutCount);
        for (uint32_t i = 0; i < m_layoutCount; ++i)
        {
            m_descLayouts[i] = a_descLayouts[i];
        }
    }
    virtual ~VulkanComputeLayoutDeletionObject()
    {
        Allocator* allocator = m_engine->GetDeletionAllocator();

        allocator->Free(m_descLayouts);
    }

    virtual void Destroy()
    {
        const vk::Device device = m_engine->GetLogicalDevice();

        for (uint32_t i = 0; i < m_layoutCount; ++i)
        {
            device.destroyDescriptorSetLayout(m_descLayouts[i]);
        }

        device.destroyPipelineLayout(m_layout);
    }
};

constexpr static vk::DescriptorType GetDescriptorType(e_ShaderBufferType a_bufferType)
{
    switch (a_bufferType) 
    {
    case ShaderBufferType_TimeBuffer:
    {
        return vk::DescriptorType::eUniformBuffer;
    }
    default:
    {
        return vk::DescriptorType::eStorageBuffer;
    }
    }

    IERROR("Invalid ShaderBufferType");

    return vk::DescriptorType::eStorageBuffer;
}

VulkanComputeLayout::VulkanComputeLayout(VulkanRenderEngineBackend* a_engine, const ShaderBufferInput* a_inputs, uint32_t a_inputCount, Allocator* a_allocator)
{
    m_allocator = a_allocator;

    m_engine = a_engine;

    m_inputCount = a_inputCount;

    const vk::Device device = m_engine->GetLogicalDevice();

    m_slotInputs = m_allocator->TAllocate<ShaderBufferInput>(m_inputCount);
    m_descLayouts = m_allocator->TAllocate<vk::DescriptorSetLayout>(m_inputCount);

    for (uint32_t i = 0; i < m_inputCount; ++i)
    {
        const ShaderBufferInput& input = a_inputs[i];

        const vk::DescriptorType type = GetDescriptorType(input.BufferType);

        const vk::DescriptorSetLayoutBinding bind = vk::DescriptorSetLayoutBinding
        (
            input.RealSlot,
            type,
            1,
            vk::ShaderStageFlagBits::eCompute
        );

        m_slotInputs[i] = a_inputs[i];

        const vk::DescriptorSetLayoutCreateInfo layoutInfo = vk::DescriptorSetLayoutCreateInfo
        (
            { },
            1,
            &bind
        );

        VKRESERRMSG(device.createDescriptorSetLayout(&layoutInfo, nullptr, &m_descLayouts[i]), "Failed to create Compute Desctiptor Layout");
    }

    const vk::PipelineLayoutCreateInfo pipelineLayoutInfo = vk::PipelineLayoutCreateInfo
    (
        { },
        m_inputCount,
        m_descLayouts
    );

    TRACE("Creating Compute Pipeline Layout");
    VKRESERRMSG(device.createPipelineLayout(&pipelineLayoutInfo, nullptr, &m_layout), "Failed to create Compute Pipeline Layout");
}
VulkanComputeLayout::~VulkanComputeLayout()
{
    TRACE("Queueing Compute Pipeline Layout for deletion");
    m_engine->PushDeletionObject<VulkanComputeLayoutDeletionObject>(m_engine, m_descLayouts, m_inputCount, m_layout);

    m_allocator->Destroy(m_descLayouts);
    m_allocator->Destroy(m_slotInputs);
}

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