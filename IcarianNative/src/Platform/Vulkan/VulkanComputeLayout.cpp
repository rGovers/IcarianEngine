// Icarian Engine - C# Game Engine
// 
// License at end of file.

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/VulkanComputeLayout.h"

#include "Core/IcarianAssert.h"
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
    VulkanComputeLayoutDeletionObject(VulkanRenderEngineBackend* a_engine, const vk::DescriptorSetLayout* a_descLayouts, uint32_t a_layoutCount, vk::PipelineLayout a_layout)
    {
        m_engine = a_engine;

        m_layoutCount = a_layoutCount;
        m_layout = a_layout;

        m_descLayouts = new vk::DescriptorSetLayout[m_layoutCount];
        for (uint32_t i = 0; i < m_layoutCount; ++i)
        {
            m_descLayouts[i] = a_descLayouts[i];
        }
    }
    virtual ~VulkanComputeLayoutDeletionObject()
    {
        delete[] m_descLayouts;
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

    ICARIAN_ASSERT(0);

    return vk::DescriptorType::eStorageBuffer;
}

VulkanComputeLayout::VulkanComputeLayout(VulkanRenderEngineBackend* a_engine, const ShaderBufferInput* a_inputs, uint32_t a_inputCount)
{
    m_engine = a_engine;

    m_inputCount = a_inputCount;

    const vk::Device device = m_engine->GetLogicalDevice();

    m_slotInputs = new ShaderBufferInput[m_inputCount];
    m_descLayouts = new vk::DescriptorSetLayout[m_inputCount];

    for (uint32_t i = 0; i < m_inputCount; ++i)
    {
        const ShaderBufferInput input = a_inputs[i];

        const vk::DescriptorSetLayoutBinding bind = vk::DescriptorSetLayoutBinding
        (
            input.Slot,
            GetDescriptorType(input.BufferType),
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

        ICARIAN_ASSERT_MSG_R(device.createDescriptorSetLayout(&layoutInfo, nullptr, &m_descLayouts[i]) == vk::Result::eSuccess, "Failed to create Compute Desctiptor Layout");
    }

    const vk::PipelineLayoutCreateInfo pipelineLayoutInfo = vk::PipelineLayoutCreateInfo
    (
        { },
        m_inputCount,
        m_descLayouts
    );

    TRACE("Creating Compute Pipeline Layout");
    ICARIAN_ASSERT_MSG_R(device.createPipelineLayout(&pipelineLayoutInfo, nullptr, &m_layout) == vk::Result::eSuccess, "Failed to create Compute Pipeline Layout");
}
VulkanComputeLayout::~VulkanComputeLayout()
{
    TRACE("Queueing Compute Pipeline Layout for deletion");
    m_engine->PushDeletionObject(new VulkanComputeLayoutDeletionObject(m_engine, m_descLayouts, m_inputCount, m_layout));

    delete[] m_descLayouts;
    delete[] m_slotInputs;
}

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