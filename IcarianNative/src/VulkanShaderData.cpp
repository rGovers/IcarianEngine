#include "Rendering/Vulkan/VulkanShaderData.h"

#include "Flare/IcarianAssert.h"
#include "ObjectManager.h"
#include "Rendering/ShaderBuffers.h"
#include "Rendering/UI/UIElement.h"
#include "Rendering/Vulkan/VulkanGraphicsEngine.h"
#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"
#include "Rendering/Vulkan/VulkanRenderTexture.h"
#include "Rendering/Vulkan/VulkanShaderStorageObject.h"
#include "Rendering/Vulkan/VulkanTexture.h"
#include "Rendering/Vulkan/VulkanTextureSampler.h"
#include "Rendering/Vulkan/VulkanUniformBuffer.h"
#include "Trace.h"

// Just changes to make the compiler happy
constexpr static vk::ShaderStageFlags GetShaderStage(FlareBase::e_ShaderSlot a_slot) 
{
    switch (a_slot)
    {
    case FlareBase::ShaderSlot_Vertex:
    {
        return vk::ShaderStageFlagBits::eVertex;
    }
    case FlareBase::ShaderSlot_Pixel:
    {
        return vk::ShaderStageFlagBits::eFragment;
    }
    case FlareBase::ShaderSlot_All:
    {
        return vk::ShaderStageFlagBits::eAllGraphics;
    }
    default:
    {
        ICARIAN_ASSERT_MSG(0, "Invalid shader slot");

        break;
    }
    }

    return vk::ShaderStageFlags();
}
constexpr static uint32_t GetBufferSize(FlareBase::e_ShaderBufferType a_type)
{
    switch (a_type)
    {
    case FlareBase::ShaderBufferType_CameraBuffer:
    {
        return sizeof(CameraShaderBuffer);
    }
    case FlareBase::ShaderBufferType_ModelBuffer:
    {
        return sizeof(ModelShaderBuffer);
    }
    case FlareBase::ShaderBufferType_UIBuffer:
    {
        return sizeof(UIShaderBuffer);
    }
    case FlareBase::ShaderBufferType_DirectionalLightBuffer:
    {
        return sizeof(DirectionalLightShaderBuffer);
    }
    case FlareBase::ShaderBufferType_PointLightBuffer:
    {
        return sizeof(PointLightShaderBuffer);
    }
    case FlareBase::ShaderBufferType_SpotLightBuffer:
    {
        return sizeof(SpotLightShaderBuffer);
    }
    default:
    {
        ICARIAN_ASSERT_MSG(0, "Invalid shader buffer type");

        break;
    }
    }

    return 0;
}
constexpr static vk::DescriptorType GetDescriptorType(FlareBase::e_ShaderBufferType a_bufferType)
{
    switch (a_bufferType)
    {
    case FlareBase::ShaderBufferType_Texture:
    case FlareBase::ShaderBufferType_PushTexture:
    {
        return vk::DescriptorType::eCombinedImageSampler;
    }
    case FlareBase::ShaderBufferType_SSModelBuffer:
    {
        return vk::DescriptorType::eStorageBuffer;
    }
    default:
    {
        return vk::DescriptorType::eUniformBuffer;
    }
    }

    ICARIAN_ASSERT(0);

    return vk::DescriptorType::eUniformBuffer;
}

struct Input
{
    uint32_t Slot;
    vk::DescriptorSetLayoutBinding Binding;
};

static void GetLayoutInfo(const FlareBase::RenderProgram& a_program, std::vector<vk::PushConstantRange>& a_pushConstants, std::vector<Input>& a_bindings, std::vector<Input>& a_pushBindings)
{
    for (uint16_t i = 0; i < a_program.ShaderBufferInputCount; ++i)
    {
        const FlareBase::ShaderBufferInput& input = a_program.ShaderBufferInputs[i];
        
        switch (input.BufferType)
        {
        case FlareBase::ShaderBufferType_ModelBuffer:
        case FlareBase::ShaderBufferType_UIBuffer:
        {
            a_pushConstants.push_back(vk::PushConstantRange
            (
                GetShaderStage(input.ShaderSlot),
                0,
                GetBufferSize(input.BufferType)
            ));

            break;
        }
        case FlareBase::ShaderBufferType_CameraBuffer:
        case FlareBase::ShaderBufferType_DirectionalLightBuffer:
        case FlareBase::ShaderBufferType_PointLightBuffer:
        case FlareBase::ShaderBufferType_SpotLightBuffer:
        case FlareBase::ShaderBufferType_PushTexture:
        case FlareBase::ShaderBufferType_SSModelBuffer:
        {
            Input in;
            in.Slot = i;
            in.Binding = vk::DescriptorSetLayoutBinding
            (
                input.Slot,
                GetDescriptorType(input.BufferType),
                1,
                GetShaderStage(input.ShaderSlot)
            );

            a_pushBindings.push_back(in);

            break;
        }
        default:
        {
            Input in;
            in.Slot = i;
            in.Binding = vk::DescriptorSetLayoutBinding
            (
                input.Slot,
                GetDescriptorType(input.BufferType),
                1,
                GetShaderStage(input.ShaderSlot)
            );

            a_bindings.push_back(in);

            break;
        }
        }
    }
}

static vk::DescriptorImageInfo GetDescriptorImageInfo(const FlareBase::TextureSampler& a_baseSampler, const VulkanTextureSampler* a_sampler, VulkanGraphicsEngine* a_engine)
{
    vk::DescriptorImageInfo imageInfo = vk::DescriptorImageInfo(a_sampler->GetSampler());

    switch (a_baseSampler.TextureMode)
    {
    case FlareBase::TextureMode_Texture:
    {
        const VulkanTexture* texture = a_engine->GetTexture(a_baseSampler.Addr);

        imageInfo.imageView = texture->GetImageView();
        imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

        break;
    }
    case FlareBase::TextureMode_RenderTexture:
    {
        const VulkanRenderTexture* renderTexture = a_engine->GetRenderTexture(a_baseSampler.Addr);

        imageInfo.imageView = renderTexture->GetImageView(a_baseSampler.TSlot);
        imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

        break;
    }
    case FlareBase::TextureMode_RenderTextureDepth:
    {
        const VulkanRenderTexture* renderTexture = a_engine->GetRenderTexture(a_baseSampler.Addr);

        imageInfo.imageView = renderTexture->GetDepthImageView();
        imageInfo.imageLayout = vk::ImageLayout::eDepthStencilReadOnlyOptimal;

        break;
    }
    case FlareBase::TextureMode_DepthRenderTexture:
    {
        const VulkanDepthRenderTexture* renderTexture = a_engine->GetDepthRenderTexture(a_baseSampler.Addr);

        imageInfo.imageView = renderTexture->GetImageView();
        imageInfo.imageLayout = vk::ImageLayout::eDepthStencilReadOnlyOptimal;

        break;
    }
    default:
    {
        ICARIAN_ASSERT_MSG(0, "Invalid texture mode");

        return vk::DescriptorImageInfo();
    }
    }

    return imageInfo;
}

VulkanShaderData::VulkanShaderData(VulkanRenderEngineBackend* a_engine, VulkanGraphicsEngine* a_gEngine, uint32_t a_programAddr)
{
    m_engine = a_engine;
    m_gEngine = a_gEngine;

    m_programAddr = a_programAddr;

    m_staticDesciptorLayout = nullptr;
    m_staticDescriptorSet = nullptr;

    TRACE("Creating Shader Data");
    const vk::Device device = m_engine->GetLogicalDevice();
    const FlareBase::RenderProgram program = m_gEngine->GetRenderProgram(m_programAddr);

    for (uint16_t i = 0; i < program.ShaderBufferInputCount; ++i)
    {
        switch (program.ShaderBufferInputs[i].BufferType)
        {
        case FlareBase::ShaderBufferType_CameraBuffer:
        case FlareBase::ShaderBufferType_DirectionalLightBuffer:
        case FlareBase::ShaderBufferType_PointLightBuffer:
        case FlareBase::ShaderBufferType_SpotLightBuffer:
        case FlareBase::ShaderBufferType_SSModelBuffer:
        {
            ShaderSlotInput input;
            input.Input = program.ShaderBufferInputs[i];
            input.Data = nullptr;

            m_slotInputs.emplace_back(input);

            break;
        }
        case FlareBase::ShaderBufferType_ModelBuffer:
        {
            m_transformBufferInput = program.ShaderBufferInputs[i];

            break;
        }
        case FlareBase::ShaderBufferType_UIBuffer:
        {
            m_uiBufferInput = program.ShaderBufferInputs[i];

            break;
        }
        default:
        {
            break;
        }
        }
    }

    std::vector<vk::PushConstantRange> pushConstants;
    std::vector<Input> bindings;
    std::vector<Input> pushBindings;
    GetLayoutInfo(program, pushConstants, bindings, pushBindings);

    std::vector<vk::DescriptorSetLayout> layouts;

    if (!bindings.empty())
    {
        const uint32_t bindingCount = (uint32_t)bindings.size();

        std::vector<vk::DescriptorSetLayoutBinding> layoutBindings;
        layoutBindings.reserve(bindingCount);
        for (const Input& in : bindings)
        {
            layoutBindings.emplace_back(in.Binding);
        }

        TRACE("Creating Pipeline Static Descriptor Layout");
        const vk::DescriptorSetLayoutCreateInfo staticDescriptorLayout = vk::DescriptorSetLayoutCreateInfo
        (
            vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPoolEXT,
            bindingCount,
            layoutBindings.data()
        );
        ICARIAN_ASSERT_MSG_R(device.createDescriptorSetLayout(&staticDescriptorLayout, nullptr, &m_staticDesciptorLayout) == vk::Result::eSuccess, "Failed to create Static Descriptor Layout");

        TRACE("Creating Pipeline Static Descriptor Pool");
        std::vector<vk::DescriptorPoolSize> poolSizes = std::vector<vk::DescriptorPoolSize>(bindingCount);
        std::vector<vk::DescriptorBindingFlagsEXT> bindingFlags = std::vector<vk::DescriptorBindingFlagsEXT>(bindingCount);
        for (uint32_t i = 0; i < bindingCount; ++i)
        {
            poolSizes[i].type = bindings[i].Binding.descriptorType;
            poolSizes[i].descriptorCount = bindings[i].Binding.descriptorCount;
            bindingFlags[i] = vk::DescriptorBindingFlagBitsEXT::eUpdateAfterBind;
        }

        const vk::DescriptorPoolCreateInfo poolInfo = vk::DescriptorPoolCreateInfo
        (
            vk::DescriptorPoolCreateFlagBits::eUpdateAfterBindEXT, 
            1, 
            bindingCount, 
            poolSizes.data()
        );
        ICARIAN_ASSERT_MSG_R(device.createDescriptorPool(&poolInfo, nullptr, &m_staticDescriptorPool) == vk::Result::eSuccess, "Failed to create Static Descriptor Pool");

        TRACE("Creating Pipeline Static Descriptor Set");
        const vk::DescriptorSetAllocateInfo descriptorSetAllocInfo = vk::DescriptorSetAllocateInfo
        (
            m_staticDescriptorPool,
            1,
            &m_staticDesciptorLayout
        );
        ICARIAN_ASSERT_MSG_R(device.allocateDescriptorSets(&descriptorSetAllocInfo, &m_staticDescriptorSet) == vk::Result::eSuccess, "Failed to create Static Descriptor Sets");

        layouts.emplace_back(m_staticDesciptorLayout);
    }

    if (!pushBindings.empty())
    {
        TRACE("Creating Pipeline Push Descriptor Layout");
        const uint32_t bindingCount = (uint32_t)pushBindings.size();

        for (uint32_t i = 0; i < bindingCount; ++i)
        {
            const Input binding = pushBindings[i];
            const vk::DescriptorSetLayoutCreateInfo descriptorLayoutInfo = vk::DescriptorSetLayoutCreateInfo
            (
                { },
                1,
                &binding.Binding
            );

            vk::DescriptorSetLayout layout;
            ICARIAN_ASSERT_MSG_R(device.createDescriptorSetLayout(&descriptorLayoutInfo, nullptr, &layout) == vk::Result::eSuccess, "Failed to create Push Descriptor Layout");
            layouts.emplace_back(layout);

            const vk::DescriptorPoolSize poolSize = vk::DescriptorPoolSize(binding.Binding.descriptorType, 1);
            const vk::DescriptorPoolCreateInfo poolInfo = vk::DescriptorPoolCreateInfo
            (
                { },
                PushCount,
                1,
                &poolSize
            );

            for (uint32_t j = 0; j < VulkanFlightPoolSize; ++j)
            {
                PushDescriptor d;
                d.Set = program.ShaderBufferInputs[binding.Slot].Set;
                d.Binding = program.ShaderBufferInputs[binding.Slot].Slot;
                d.DescriptorLayout = layout;
                ICARIAN_ASSERT_MSG_R(device.createDescriptorPool(&poolInfo, nullptr, &d.DescriptorPool) == vk::Result::eSuccess, "Failed to create Push Descriptor Pool");

                m_pushDescriptors[j].emplace_back(d);
            }
        }
    }

    const vk::PipelineLayoutCreateInfo pipelineLayoutInfo = vk::PipelineLayoutCreateInfo
    (
        { },
        (uint32_t)layouts.size(),
        layouts.data(),
        (uint32_t)pushConstants.size(),
        pushConstants.data()
    );

    TRACE("Creating Pipeline Layout");
    ICARIAN_ASSERT_MSG_R(device.createPipelineLayout(&pipelineLayoutInfo, nullptr, &m_layout) == vk::Result::eSuccess, "Failed to create Pipeline Layout");
}
VulkanShaderData::~VulkanShaderData()
{
    TRACE("Destroying Shader Data");
    const vk::Device device = m_engine->GetLogicalDevice();

    for (const ShaderSlotInput& input : m_slotInputs)
    {
        switch (input.Input.BufferType)
        {
        case FlareBase::ShaderBufferType_SSModelBuffer:
        {
            delete (VulkanShaderStorageObject*)input.Data;

            break;
        }
        default:
        {
            break;
        }
        }
    }

    if (m_staticDesciptorLayout != vk::DescriptorSetLayout(nullptr))
    {
        device.destroyDescriptorSetLayout(m_staticDesciptorLayout);
        device.destroyDescriptorPool(m_staticDescriptorPool);
    }

    const uint32_t pDescriptorCount = (uint32_t)m_pushDescriptors[0].size();
    for (uint32_t i = 0; i < pDescriptorCount; ++i)
    {
        device.destroyDescriptorSetLayout(m_pushDescriptors[0][i].DescriptorLayout);

        for (uint32_t j = 0; j < VulkanFlightPoolSize; ++j)
        {
            device.destroyDescriptorPool(m_pushDescriptors[j][i].DescriptorPool);
        }
    }

    device.destroyPipelineLayout(m_layout);
}

void VulkanShaderData::SetTexture(uint32_t a_slot, const FlareBase::TextureSampler& a_sampler) const
{
    const vk::Device device = m_engine->GetLogicalDevice();

    const VulkanTextureSampler* vSampler = (VulkanTextureSampler*)a_sampler.Data;
    ICARIAN_ASSERT(vSampler != nullptr);

    const vk::DescriptorImageInfo imageInfo = GetDescriptorImageInfo(a_sampler, vSampler, m_gEngine);

    TRACE("Setting material texture");
    const vk::WriteDescriptorSet descriptorWrite = vk::WriteDescriptorSet
    (
        m_staticDescriptorSet,
        a_slot,
        0,
        1,
        vk::DescriptorType::eCombinedImageSampler,
        &imageInfo
    );

    device.updateDescriptorSets(1, &descriptorWrite, 0, nullptr);
}

void VulkanShaderData::PushTexture(vk::CommandBuffer a_commandBuffer, uint32_t a_slot, const FlareBase::TextureSampler& a_sampler, uint32_t a_index) const
{   
    const vk::Device device = m_engine->GetLogicalDevice();

    const VulkanTextureSampler* vSampler = (VulkanTextureSampler*)a_sampler.Data;
    ICARIAN_ASSERT(vSampler != nullptr);

    for (const PushDescriptor& d : m_pushDescriptors[a_index])
    {
        if (d.Set == a_slot)
        {
            const vk::DescriptorSetAllocateInfo descriptorSetInfo = vk::DescriptorSetAllocateInfo
            (
                d.DescriptorPool,
                1,
                &d.DescriptorLayout
            );

            vk::DescriptorSet descriptorSet;
            ICARIAN_ASSERT_R(device.allocateDescriptorSets(&descriptorSetInfo, &descriptorSet) == vk::Result::eSuccess);

            const vk::DescriptorImageInfo imageInfo = GetDescriptorImageInfo(a_sampler, vSampler, m_gEngine);
            
            const vk::WriteDescriptorSet descriptorWrite = vk::WriteDescriptorSet
            (
                descriptorSet,
                d.Binding,
                0,
                1,
                vk::DescriptorType::eCombinedImageSampler,
                &imageInfo
            );

            device.updateDescriptorSets(1, &descriptorWrite, 0, nullptr);
            
            a_commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_layout, d.Set, 1, &descriptorSet, 0, nullptr);

            return;
        }
    }

    ICARIAN_ASSERT_MSG(0, "PushTexture binding not found");
}
void VulkanShaderData::PushUniformBuffer(vk::CommandBuffer a_commandBuffer, uint32_t a_slot, const VulkanUniformBuffer* a_buffer, uint32_t a_index) const
{
    const vk::Device device = m_engine->GetLogicalDevice();

    for (const PushDescriptor& d : m_pushDescriptors[a_index])
    {
        if (d.Set == a_slot)
        {
            const vk::DescriptorSetAllocateInfo descriptorSetInfo = vk::DescriptorSetAllocateInfo
            (
                d.DescriptorPool,
                1,
                &d.DescriptorLayout
            );

            vk::DescriptorSet descriptorSet;
            ICARIAN_ASSERT_R(device.allocateDescriptorSets(&descriptorSetInfo, &descriptorSet) == vk::Result::eSuccess);

            const vk::DescriptorBufferInfo bufferInfo = vk::DescriptorBufferInfo
            (
                a_buffer->GetBuffer(a_index), 
                0, 
                VK_WHOLE_SIZE
            );

            const vk::WriteDescriptorSet descriptorWrite = vk::WriteDescriptorSet
            (
                descriptorSet,
                d.Binding,
                0,
                1,
                vk::DescriptorType::eUniformBuffer,
                nullptr,
                &bufferInfo
            );

            device.updateDescriptorSets(1, &descriptorWrite, 0, nullptr);

            a_commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_layout, d.Set, 1, &descriptorSet, 0, nullptr);

            return;
        }
    }

    ICARIAN_ASSERT_MSG(0, "PushUniformBuffer binding not found");
}
void VulkanShaderData::PushShaderStorageObject(vk::CommandBuffer a_commandBuffer, uint32_t a_slot, const VulkanShaderStorageObject* a_object, uint32_t a_index) const
{
    const vk::Device device = m_engine->GetLogicalDevice();

    for (const PushDescriptor& d : m_pushDescriptors[a_index])
    {
        if (d.Set == a_slot)
        {
            const vk::DescriptorSetAllocateInfo descriptorSetInfo = vk::DescriptorSetAllocateInfo
            (
                d.DescriptorPool,
                1,
                &d.DescriptorLayout
            );

            vk::DescriptorSet descriptorSet;
            ICARIAN_ASSERT_R(device.allocateDescriptorSets(&descriptorSetInfo, &descriptorSet) == vk::Result::eSuccess);

            const vk::DescriptorBufferInfo bufferInfo = vk::DescriptorBufferInfo
            (
                a_object->GetBuffer(),
                0,
                VK_WHOLE_SIZE
            );

            const vk::WriteDescriptorSet descriptorWrite = vk::WriteDescriptorSet
            (
                descriptorSet,
                d.Binding,
                0,
                1,
                vk::DescriptorType::eStorageBuffer,
                nullptr,
                &bufferInfo
            );

            device.updateDescriptorSets(1, &descriptorWrite, 0, nullptr);

            a_commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_layout, d.Set, 1, &descriptorSet, 0, nullptr);

            return;
        }
    }

    ICARIAN_ASSERT_MSG(0, "PushShaderStorageObject binding not found");
}

void VulkanShaderData::UpdateTransformBuffer(vk::CommandBuffer a_commandBuffer, const glm::mat4& a_transform) const
{
    if (m_transformBufferInput.ShaderSlot != FlareBase::ShaderSlot_Null)
    {
        ModelShaderBuffer buffer;
        buffer.Model = a_transform;
        buffer.InvModel = glm::inverse(a_transform);

        a_commandBuffer.pushConstants(m_layout, GetShaderStage(m_transformBufferInput.ShaderSlot), 0, sizeof(ModelShaderBuffer), &buffer);
    }
}
void VulkanShaderData::UpdateUIBuffer(vk::CommandBuffer a_commandBuffer, const UIElement* a_element) const
{
    if (m_uiBufferInput.ShaderSlot != FlareBase::ShaderSlot_Null)
    {
        UIShaderBuffer buffer;
        buffer.Color = a_element->GetColor();

        a_commandBuffer.pushConstants(m_layout, GetShaderStage(m_uiBufferInput.ShaderSlot), 0, sizeof(UIShaderBuffer), &buffer);
    }
}

void VulkanShaderData::Bind(uint32_t a_index, vk::CommandBuffer a_commandBuffer) const
{
    const vk::Device device = m_engine->GetLogicalDevice();
    for (const PushDescriptor& d : m_pushDescriptors[a_index])
    {
        device.resetDescriptorPool(d.DescriptorPool);
    }

    if (m_staticDescriptorSet != vk::DescriptorSet(nullptr))
    {
        a_commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_layout, StaticIndex, 1, &m_staticDescriptorSet, 0, nullptr);
    }
}

bool VulkanShaderData::GetCameraInput(FlareBase::ShaderBufferInput* a_input) const
{
    for (const ShaderSlotInput& input : m_slotInputs)
    {
        if (input.Input.BufferType == FlareBase::ShaderBufferType_CameraBuffer)
        {
            *a_input = input.Input;

            return true;
        }
    }

    return false;
}

bool VulkanShaderData::GetDirectionalLightInput(FlareBase::ShaderBufferInput* a_input) const
{
    for (const ShaderSlotInput& input : m_slotInputs)
    {
        if (input.Input.BufferType == FlareBase::ShaderBufferType_DirectionalLightBuffer)
        {
            *a_input = input.Input;

            return true;
        }
    }

    return false;
}
bool VulkanShaderData::GetPointLightInput(FlareBase::ShaderBufferInput* a_input) const
{
    for (const ShaderSlotInput& input : m_slotInputs)
    {
        if (input.Input.BufferType == FlareBase::ShaderBufferType_PointLightBuffer)
        {
            *a_input = input.Input;

            return true;
        }
    }

    return false;
}
bool VulkanShaderData::GetSpotLightInput(FlareBase::ShaderBufferInput* a_input) const
{
    for (const ShaderSlotInput& input : m_slotInputs)
    {
        if (input.Input.BufferType == FlareBase::ShaderBufferType_SpotLightBuffer)
        {
            *a_input = input.Input;

            return true;
        }
    }

    return false;
}

bool VulkanShaderData::GetBatchModelBufferInput(FlareBase::ShaderBufferInput* a_input) const
{
    for (const ShaderSlotInput& input : m_slotInputs)
    {
        if (input.Input.BufferType == FlareBase::ShaderBufferType_SSModelBuffer)
        {
            *a_input = input.Input;

            return true;
        }
    }

    return false;
}
