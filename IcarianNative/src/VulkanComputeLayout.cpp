#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/VulkanComputeLayout.h"

#include "Flare/IcarianAssert.h"
#include "Flare/IcarianDefer.h"
#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"
#include "Trace.h"

class VulkanComputeLayoutDeletionObject : public VulkanDeletionObject
{
private:
    VulkanRenderEngineBackend* m_engine;

    vk::DescriptorSetLayout    m_descLayout;
    vk::PipelineLayout         m_layout;

protected:

public:
    VulkanComputeLayoutDeletionObject(VulkanRenderEngineBackend* a_engine, vk::DescriptorSetLayout a_descLayout, vk::PipelineLayout a_layout)
    {
        m_engine = a_engine;

        m_descLayout = a_descLayout;
        m_layout = a_layout;
    }
    virtual ~VulkanComputeLayoutDeletionObject()
    {

    }

    virtual void Destroy()
    {
        const vk::Device device = m_engine->GetLogicalDevice();

        device.destroyDescriptorSetLayout(m_descLayout);
        device.destroyPipelineLayout(m_layout);
    }
};

constexpr static vk::DescriptorType GetDescriptorType(e_ShaderBufferType a_bufferType)
{
    switch (a_bufferType) 
    {
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
    // m_engine->PushDeletionObject(new VulkanComputeLayoutDeletionObject(m_engine, m_descLayout, m_layout));

    delete[] m_descLayouts;
    delete[] m_slotInputs;
}

#endif