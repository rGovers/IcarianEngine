#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/VulkanShaderData.h"

#include "Flare/IcarianAssert.h"
#include "Flare/IcarianDefer.h"
#include "ObjectManager.h"
#include "Rendering/ShaderBuffers.h"
#include "Rendering/UI/UIElement.h"
#include "Rendering/Vulkan/VulkanGraphicsEngine.h"
#include "Rendering/Vulkan/VulkanPushPool.h"
#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"
#include "Rendering/Vulkan/VulkanRenderTexture.h"
#include "Rendering/Vulkan/VulkanShaderStorageObject.h"
#include "Rendering/Vulkan/VulkanTexture.h"
#include "Rendering/Vulkan/VulkanTextureSampler.h"
#include "Rendering/Vulkan/VulkanUniformBuffer.h"
#include "Trace.h"

class VulkanShaderDataDeletionObject : public VulkanDeletionObject
{
private:
    VulkanRenderEngineBackend*        m_engine;
    
    vk::DescriptorSetLayout           m_staticLayout;
    vk::DescriptorPool                m_staticPool;
    
    vk::DescriptorSetLayout           m_shadowStaticLayout;
    vk::DescriptorPool                m_shadowStaticPool;

    std::vector<VulkanPushDescriptor> m_pushDescriptors;
    std::vector<VulkanPushDescriptor> m_shadowPushDescriptors;

    vk::PipelineLayout                m_layout;
    vk::PipelineLayout                m_shadowLayout;

protected:

public:
    VulkanShaderDataDeletionObject(VulkanRenderEngineBackend* a_engine, vk::DescriptorSetLayout a_staticLayout, vk::DescriptorPool a_staticPool, vk::DescriptorSetLayout a_shadowStaticLayout, vk::DescriptorPool a_shadowStaticPool, const std::vector<VulkanPushDescriptor>& a_pushDescriptors, const std::vector<VulkanPushDescriptor>& a_shadowPushDescriptors, vk::PipelineLayout a_layout, vk::PipelineLayout a_shadowLayout)
    {
        m_engine = a_engine;

        m_staticLayout = a_staticLayout;
        m_staticPool = a_staticPool;

        m_shadowStaticLayout = a_shadowStaticLayout;
        m_shadowStaticPool = a_shadowStaticPool;

        m_pushDescriptors = a_pushDescriptors;
        m_shadowPushDescriptors = a_shadowPushDescriptors;

        m_layout = a_layout;
        m_shadowLayout = a_shadowLayout;
    }
    virtual ~VulkanShaderDataDeletionObject()
    {
        
    }

    virtual void Destroy()
    {
        const vk::Device device = m_engine->GetLogicalDevice();

        if (m_staticLayout != vk::DescriptorSetLayout(nullptr))
        {
            device.destroyDescriptorSetLayout(m_staticLayout);
            device.destroyDescriptorPool(m_staticPool);
        }

        if (m_shadowStaticLayout != vk::DescriptorSetLayout(nullptr))
        {
            device.destroyDescriptorSetLayout(m_shadowStaticLayout);
            device.destroyDescriptorPool(m_shadowStaticPool);
        }

        const uint32_t pDescriptorCount = (uint32_t)m_pushDescriptors.size();
        for (uint32_t i = 0; i < pDescriptorCount; ++i)
        {
            device.destroyDescriptorSetLayout(m_pushDescriptors[i].DescriptorLayout);
        }

        const uint32_t sDescriptorCount = (uint32_t)m_shadowPushDescriptors.size();
        for (uint32_t i = 0; i < sDescriptorCount; ++i)
        {
            device.destroyDescriptorSetLayout(m_shadowPushDescriptors[i].DescriptorLayout);
        }

        device.destroyPipelineLayout(m_layout);
        if (m_shadowLayout != vk::PipelineLayout(nullptr))
        {
            device.destroyPipelineLayout(m_shadowLayout);
        }
    }
};

// Just changes to make the compiler happy
constexpr static vk::ShaderStageFlags GetShaderStage(e_ShaderSlot a_slot) 
{
    switch (a_slot)
    {
    case ShaderSlot_Vertex:
    {
        return vk::ShaderStageFlagBits::eVertex;
    }
    case ShaderSlot_Pixel:
    {
        return vk::ShaderStageFlagBits::eFragment;
    }
    case ShaderSlot_All:
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
constexpr static uint32_t GetBufferSize(e_ShaderBufferType a_type)
{
    switch (a_type)
    {
    case ShaderBufferType_PModelBuffer:
    {
        return sizeof(ModelShaderBuffer);
    }
    case ShaderBufferType_PUIBuffer:
    {
        return sizeof(UIShaderBuffer);
    }
    case ShaderBufferType_PShadowLightBuffer:
    {
        return sizeof(ShadowLightShaderBuffer);
    }
    case ShaderBufferType_CameraBuffer:
    {
        return sizeof(CameraShaderBuffer);
    }
    case ShaderBufferType_DirectionalLightBuffer:
    {
        return sizeof(DirectionalLightShaderBuffer);
    }
    case ShaderBufferType_PointLightBuffer:
    {
        return sizeof(PointLightShaderBuffer);
    }
    case ShaderBufferType_SpotLightBuffer:
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
constexpr static vk::DescriptorType GetDescriptorType(e_ShaderBufferType a_bufferType)
{
    switch (a_bufferType)
    {
    case ShaderBufferType_Texture:
    case ShaderBufferType_PushTexture:
    {
        return vk::DescriptorType::eCombinedImageSampler;
    }
    case ShaderBufferType_SSBoneBuffer:
    case ShaderBufferType_SSModelBuffer:
    case ShaderBufferType_SSDirectionalLightBuffer:
    case ShaderBufferType_SSPointLightBuffer:
    case ShaderBufferType_SSSpotLightBuffer:
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

static void GetLayoutInfo(const ShaderBufferInput* a_inputs, uint32_t a_inputCount, std::vector<vk::PushConstantRange>* a_pushConstants, std::vector<Input>* a_bindings, std::vector<Input>* a_pushBindings)
{
    for (uint16_t i = 0; i < a_inputCount; ++i)
    {
        const ShaderBufferInput& input = a_inputs[i];
        
        switch (input.BufferType)
        {
        case ShaderBufferType_PModelBuffer:
        case ShaderBufferType_PUIBuffer:
        case ShaderBufferType_PShadowLightBuffer:
        {
            a_pushConstants->push_back(vk::PushConstantRange
            (
                GetShaderStage(input.ShaderSlot),
                0,
                GetBufferSize(input.BufferType)
            ));

            break;
        }
        case ShaderBufferType_CameraBuffer:
        case ShaderBufferType_DirectionalLightBuffer:
        case ShaderBufferType_PointLightBuffer:
        case ShaderBufferType_SpotLightBuffer:
        case ShaderBufferType_PushTexture:
        case ShaderBufferType_SSModelBuffer:
        case ShaderBufferType_SSBoneBuffer:
        case ShaderBufferType_SSDirectionalLightBuffer:
        case ShaderBufferType_SSPointLightBuffer:
        case ShaderBufferType_SSSpotLightBuffer:
        case ShaderBufferType_UserUBO:
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

            a_pushBindings->push_back(in);

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

            a_bindings->push_back(in);

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

static void GenerateStaticBindings(const std::vector<Input>& a_bindings, const vk::Device a_device, std::vector<vk::DescriptorSetLayout>* a_layouts, vk::DescriptorSetLayout* a_staticDesciptorLayout, vk::DescriptorSet* a_staticDescriptorSet, vk::DescriptorPool* a_staticDescriptorPool)
{
    if (!a_bindings.empty())
    {
        const uint32_t bindingCount = (uint32_t)a_bindings.size();

        vk::DescriptorSetLayoutBinding* layoutBindings = new vk::DescriptorSetLayoutBinding[bindingCount];
        IDEFER(delete[] layoutBindings);
        for (uint32_t i = 0; i < bindingCount; ++i)
        {
            layoutBindings[i] = a_bindings[i].Binding;
        }

        const vk::DescriptorSetLayoutCreateInfo staticDescriptorLayout = vk::DescriptorSetLayoutCreateInfo
        (
            vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPoolEXT,
            bindingCount,
            layoutBindings
        );
        ICARIAN_ASSERT_MSG_R(a_device.createDescriptorSetLayout(&staticDescriptorLayout, nullptr, a_staticDesciptorLayout) == vk::Result::eSuccess, "Failed to create Static Descriptor Layout");

        vk::DescriptorPoolSize* poolSizes = new vk::DescriptorPoolSize[bindingCount];
        IDEFER(delete[] poolSizes);
        for (uint32_t i = 0; i < bindingCount; ++i)
        {
            poolSizes[i].type = a_bindings[i].Binding.descriptorType;
            poolSizes[i].descriptorCount = a_bindings[i].Binding.descriptorCount;
        }

        const vk::DescriptorPoolCreateInfo poolInfo = vk::DescriptorPoolCreateInfo
        (
            vk::DescriptorPoolCreateFlagBits::eUpdateAfterBindEXT, 
            1, 
            bindingCount, 
            poolSizes
        );
        ICARIAN_ASSERT_MSG_R(a_device.createDescriptorPool(&poolInfo, nullptr, a_staticDescriptorPool) == vk::Result::eSuccess, "Failed to create Static Descriptor Pool");

        const vk::DescriptorSetAllocateInfo descriptorSetAllocInfo = vk::DescriptorSetAllocateInfo
        (
            *a_staticDescriptorPool,
            1,
            a_staticDesciptorLayout
        );
        ICARIAN_ASSERT_MSG_R(a_device.allocateDescriptorSets(&descriptorSetAllocInfo, a_staticDescriptorSet) == vk::Result::eSuccess, "Failed to create Static Descriptor Sets");

        a_layouts->emplace_back(*a_staticDesciptorLayout);
    }
}
static void GeneratePushBindings(const std::vector<Input>& a_bindings, const ShaderBufferInput* a_inputs, uint32_t a_inputCount, const vk::Device a_device, std::vector<vk::DescriptorSetLayout>* a_layouts, std::vector<VulkanPushDescriptor>* a_pushDescriptors)
{
    if (!a_bindings.empty())
    {
        const uint32_t bindingCount = (uint32_t)a_bindings.size();

        for (uint32_t i = 0; i < bindingCount; ++i)
        {
            const Input binding = a_bindings[i];
            const vk::DescriptorSetLayoutCreateInfo descriptorLayoutInfo = vk::DescriptorSetLayoutCreateInfo
            (
                { },
                1,
                &binding.Binding
            );

            vk::DescriptorSetLayout layout;
            ICARIAN_ASSERT_MSG_R(a_device.createDescriptorSetLayout(&descriptorLayoutInfo, nullptr, &layout) == vk::Result::eSuccess, "Failed to create Push Descriptor Layout");
            a_layouts->emplace_back(layout);

            VulkanPushDescriptor d;
            d.Set = a_inputs[binding.Slot].Set;
            d.Binding = a_inputs[binding.Slot].Slot;
            d.DescriptorLayout = layout;
                
            a_pushDescriptors->emplace_back(d);
        }
    }
}

VulkanShaderData::VulkanShaderData(VulkanRenderEngineBackend* a_engine, VulkanGraphicsEngine* a_gEngine, const RenderProgram& a_program)
{
    m_engine = a_engine;
    m_gEngine = a_gEngine;

    m_staticDesciptorLayout = nullptr;
    m_staticDescriptorSet = nullptr;

    m_shadowStaticDesciptorLayout = nullptr;
    m_shadowStaticDescriptorSet = nullptr;

    m_shadowLayout = nullptr;

    m_userUniformBuffer = nullptr;

    m_userBufferInput.ShaderSlot = ShaderSlot_Null;
    m_transformBufferInput.ShaderSlot = ShaderSlot_Null;
    m_uiBufferInput.ShaderSlot = ShaderSlot_Null;

    TRACE("Creating Shader Data");
    const vk::Device device = m_engine->GetLogicalDevice();

    for (uint16_t i = 0; i < a_program.ShaderBufferInputCount; ++i)
    {
        switch (a_program.ShaderBufferInputs[i].BufferType)
        {
        case ShaderBufferType_UserUBO:
        {
            m_userBufferInput = a_program.ShaderBufferInputs[i];

            break;
        }
        case ShaderBufferType_PModelBuffer:
        {
            m_transformBufferInput = a_program.ShaderBufferInputs[i];

            break;
        }
        case ShaderBufferType_PUIBuffer:
        {
            m_uiBufferInput = a_program.ShaderBufferInputs[i];

            break;
        }
        default:
        {
            m_slotInputs.emplace_back(a_program.ShaderBufferInputs[i]);

            break;
        }
        }
    }

    std::vector<vk::PushConstantRange> pushConstants;
    std::vector<Input> bindings;
    std::vector<Input> pushBindings;
    GetLayoutInfo(a_program.ShaderBufferInputs, a_program.ShaderBufferInputCount, &pushConstants, &bindings, &pushBindings);

    std::vector<vk::DescriptorSetLayout> layouts;

    GenerateStaticBindings(bindings, device, &layouts, &m_staticDesciptorLayout, &m_staticDescriptorSet, &m_staticDescriptorPool);
    GeneratePushBindings(pushBindings, a_program.ShaderBufferInputs, a_program.ShaderBufferInputCount, device, &layouts, &m_pushDescriptors);    

    const vk::PipelineLayoutCreateInfo pipelineLayoutInfo = vk::PipelineLayoutCreateInfo
    (
        { },
        (uint32_t)layouts.size(),
        layouts.data(),
        (uint32_t)pushConstants.size(),
        pushConstants.data()
    );

    if (a_program.UBOData != NULL && a_program.UBODataSize > 0)
    {
        m_userUniformBuffer = new VulkanUniformBuffer(m_engine, a_program.UBODataSize);
    }

    TRACE("Creating Pipeline Layout");
    ICARIAN_ASSERT_MSG_R(device.createPipelineLayout(&pipelineLayoutInfo, nullptr, &m_layout) == vk::Result::eSuccess, "Failed to create Pipeline Layout");

    if (a_program.ShadowVertexShader != -1)
    {
        for (uint16_t i = 0; i < a_program.ShadowShaderBufferInputCount; ++i)
        {
            m_shadowSlotInputs.emplace_back(a_program.ShadowShaderBufferInputs[i]);
        }

        std::vector<vk::PushConstantRange> shadowPushConstants;
        std::vector<Input> shadowBindings;
        std::vector<Input> shadowPushBindings;
        GetLayoutInfo(a_program.ShadowShaderBufferInputs, a_program.ShadowShaderBufferInputCount, &shadowPushConstants, &shadowBindings, &shadowPushBindings);

        std::vector<vk::DescriptorSetLayout> shadowLayouts;

        GenerateStaticBindings(shadowBindings, device, &shadowLayouts, &m_shadowStaticDesciptorLayout, &m_shadowStaticDescriptorSet, &m_shadowStaticDescriptorPool);
        GeneratePushBindings(shadowPushBindings, a_program.ShadowShaderBufferInputs, a_program.ShadowShaderBufferInputCount, device, &shadowLayouts, &m_shadowPushDescriptors);

        const vk::PipelineLayoutCreateInfo shadowPipelineLayoutInfo = vk::PipelineLayoutCreateInfo
        (
            { },
            (uint32_t)shadowLayouts.size(),
            shadowLayouts.data(),
            (uint32_t)shadowPushConstants.size(),
            shadowPushConstants.data()
        );

        TRACE("Creating Shadow Pipeline Layout");
        ICARIAN_ASSERT_MSG_R(device.createPipelineLayout(&shadowPipelineLayoutInfo, nullptr, &m_shadowLayout) == vk::Result::eSuccess, "Failed to create Shadow Pipeline Layout");
    }
}
VulkanShaderData::~VulkanShaderData()
{
    TRACE("Queueing Shader Data for deletion");
    m_engine->PushDeletionObject(new VulkanShaderDataDeletionObject(m_engine, m_staticDesciptorLayout, m_staticDescriptorPool, m_shadowStaticDesciptorLayout, m_shadowStaticDescriptorPool, m_pushDescriptors, m_shadowPushDescriptors, m_layout, m_shadowLayout));    

    if (m_userUniformBuffer != nullptr)
    {
        delete m_userUniformBuffer;
    }
}

void VulkanShaderData::SetTexture(uint32_t a_slot, const FlareBase::TextureSampler& a_sampler) const
{
    vk::Device device = m_engine->GetLogicalDevice();

    ICARIAN_ASSERT(a_sampler.Data != nullptr);
    const VulkanTextureSampler* vSampler = (VulkanTextureSampler*)a_sampler.Data;

    vk::DescriptorImageInfo imageInfo = GetDescriptorImageInfo(a_sampler, vSampler, m_gEngine);

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

void VulkanShaderData::PushTexture(vk::CommandBuffer a_commandBuffer, uint32_t a_set, const FlareBase::TextureSampler& a_sampler, uint32_t a_index) const
{   
    const vk::Device device = m_engine->GetLogicalDevice();

    const VulkanTextureSampler* vSampler = (VulkanTextureSampler*)a_sampler.Data;
    ICARIAN_ASSERT(vSampler != nullptr);

    for (const VulkanPushDescriptor& d : m_pushDescriptors)
    {
        if (d.Set == a_set)
        {
            VulkanPushPool* pool = m_gEngine->GetPushPool();

            vk::DescriptorSet descriptorSet = pool->AllocateDescriptor(a_index, vk::DescriptorType::eCombinedImageSampler, &d.DescriptorLayout);

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
void VulkanShaderData::PushUniformBuffer(vk::CommandBuffer a_commandBuffer, uint32_t a_set, const VulkanUniformBuffer* a_buffer, uint32_t a_index) const
{
    const vk::Device device = m_engine->GetLogicalDevice();

    for (const VulkanPushDescriptor& d : m_pushDescriptors)
    {
        if (d.Set == a_set)
        {
            VulkanPushPool* pool = m_gEngine->GetPushPool();
            
            vk::DescriptorSet descriptorSet = pool->AllocateDescriptor(a_index, vk::DescriptorType::eUniformBuffer, &d.DescriptorLayout);

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
void VulkanShaderData::PushShaderStorageObject(vk::CommandBuffer a_commandBuffer, uint32_t a_set, const VulkanShaderStorageObject* a_object, uint32_t a_index) const
{
    const vk::Device device = m_engine->GetLogicalDevice();

    for (const VulkanPushDescriptor& d : m_pushDescriptors)
    {
        if (d.Set == a_set)
        {
            VulkanPushPool* pool = m_gEngine->GetPushPool();

            vk::DescriptorSet descriptorSet = pool->AllocateDescriptor(a_index, vk::DescriptorType::eStorageBuffer, &d.DescriptorLayout);

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

void VulkanShaderData::PushShadowTexture(vk::CommandBuffer a_commandBuffer, uint32_t a_set, const FlareBase::TextureSampler& a_sampler, uint32_t a_index) const
{
    const vk::Device device = m_engine->GetLogicalDevice();

    const VulkanTextureSampler* vSampler = (VulkanTextureSampler*)a_sampler.Data;
    ICARIAN_ASSERT(vSampler != nullptr);

    for (const VulkanPushDescriptor& d : m_shadowPushDescriptors)
    {
        if (d.Set == a_set)
        {
            VulkanPushPool* pool = m_gEngine->GetPushPool();

            vk::DescriptorSet descriptorSet = pool->AllocateDescriptor(a_index, vk::DescriptorType::eCombinedImageSampler, &d.DescriptorLayout);

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

            a_commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_shadowLayout, d.Set, 1, &descriptorSet, 0, nullptr);

            return;
        }
    }

    ICARIAN_ASSERT_MSG(0, "PushShadowTexture binding not found");
}
void VulkanShaderData::PushShadowUniformBuffer(vk::CommandBuffer a_commandBuffer, uint32_t a_set, const VulkanUniformBuffer* a_buffer, uint32_t a_index) const
{
    const vk::Device device = m_engine->GetLogicalDevice();

    for (const VulkanPushDescriptor& d : m_shadowPushDescriptors)
    {
        if (d.Set == a_set)
        {
            VulkanPushPool* pool = m_gEngine->GetPushPool();

            vk::DescriptorSet descriptorSet = pool->AllocateDescriptor(a_index, vk::DescriptorType::eUniformBuffer, &d.DescriptorLayout);

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

            a_commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_shadowLayout, d.Set, 1, &descriptorSet, 0, nullptr);

            return;
        }
    }

    ICARIAN_ASSERT_MSG(0, "PushShadowUniformBuffer binding not found");
}
void VulkanShaderData::PushShadowShaderStorageObject(vk::CommandBuffer a_commandBuffer, uint32_t a_set, const VulkanShaderStorageObject* a_object, uint32_t a_index) const
{
    const vk::Device device = m_engine->GetLogicalDevice();

    for (const VulkanPushDescriptor& d : m_shadowPushDescriptors)
    {
        if (d.Set == a_set)
        {
            VulkanPushPool* pool = m_gEngine->GetPushPool();

            vk::DescriptorSet descriptorSet = pool->AllocateDescriptor(a_index, vk::DescriptorType::eStorageBuffer, &d.DescriptorLayout);

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

            a_commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_shadowLayout, d.Set, 1, &descriptorSet, 0, nullptr);

            return;
        }
    }

    ICARIAN_ASSERT_MSG(0, "PushShadowShaderStorageObject binding not found");
}

void VulkanShaderData::UpdateTransformBuffer(vk::CommandBuffer a_commandBuffer, const glm::mat4& a_transform) const
{
    if (m_transformBufferInput.ShaderSlot != ShaderSlot_Null)
    {
        ModelShaderBuffer buffer;
        buffer.Model = a_transform;
        buffer.InvModel = glm::inverse(a_transform);

        a_commandBuffer.pushConstants(m_layout, GetShaderStage(m_transformBufferInput.ShaderSlot), 0, sizeof(ModelShaderBuffer), &buffer);
    }
}
void VulkanShaderData::UpdateShadowTransformBuffer(vk::CommandBuffer a_commandBuffer, const glm::mat4& a_transform) const
{
    for (const ShaderBufferInput& input : m_shadowSlotInputs)
    {
        if (input.BufferType == ShaderBufferType_PModelBuffer)
        {
            ModelShaderBuffer buffer;
            buffer.Model = a_transform;
            buffer.InvModel = glm::inverse(a_transform);

            a_commandBuffer.pushConstants(m_shadowLayout, GetShaderStage(input.ShaderSlot), 0, sizeof(ModelShaderBuffer), &buffer);

            return;
        }
    }
}

void VulkanShaderData::UpdateUIBuffer(vk::CommandBuffer a_commandBuffer, const UIElement* a_element) const
{
    if (m_uiBufferInput.ShaderSlot != ShaderSlot_Null)
    {
        UIShaderBuffer buffer;
        buffer.Color = a_element->GetColor();

        a_commandBuffer.pushConstants(m_layout, GetShaderStage(m_uiBufferInput.ShaderSlot), 0, sizeof(UIShaderBuffer), &buffer);
    }
}

void VulkanShaderData::UpdateShadowLightBuffer(vk::CommandBuffer a_commandBuffer, const glm::mat4& a_lvp) const
{
    for (const ShaderBufferInput& input : m_shadowSlotInputs)
    {
        if (input.BufferType == ShaderBufferType_PShadowLightBuffer)
        {
            ShadowLightShaderBuffer buffer;
            buffer.LVP = a_lvp;

            a_commandBuffer.pushConstants(m_shadowLayout, GetShaderStage(input.ShaderSlot), 0, sizeof(ShadowLightShaderBuffer), &buffer);

            return;
        }
    }
}

void VulkanShaderData::Update(uint32_t a_index, const RenderProgram& a_program)
{
    // Want the user to be able to update the uniform buffer whenever they want
    // However updating mid render can cause issues
    if (m_userBufferInput.ShaderSlot != ShaderSlot_Null && m_userUniformBuffer != nullptr)
    {
        m_userUniformBuffer->SetData(a_index, a_program.UBOData);
    }
}
void VulkanShaderData::BindShadow(uint32_t a_index, vk::CommandBuffer a_commandBuffer) const
{
    if (m_shadowStaticDescriptorSet != vk::DescriptorSet(nullptr))
    {
        a_commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_shadowLayout, StaticIndex, 1, &m_shadowStaticDescriptorSet, 0, nullptr);
    }

    for (const ShaderBufferInput& input : m_shadowSlotInputs)
    {
        if (input.ShaderSlot != ShaderSlot_Null && input.BufferType == ShaderBufferType_UserUBO && m_userUniformBuffer != nullptr)
        {
            PushUniformBuffer(a_commandBuffer, input.Set, m_userUniformBuffer, a_index);
        }
    }
}
void VulkanShaderData::Bind(uint32_t a_index, vk::CommandBuffer a_commandBuffer) const
{
    if (m_staticDescriptorSet != vk::DescriptorSet(nullptr))
    {
        a_commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_layout, StaticIndex, 1, &m_staticDescriptorSet, 0, nullptr);
    }    

    if (m_userBufferInput.ShaderSlot != ShaderSlot_Null && m_userUniformBuffer != nullptr)
    {
        PushUniformBuffer(a_commandBuffer, m_userBufferInput.Set, m_userUniformBuffer, a_index);
    }
}

bool VulkanShaderData::GetCameraInput(ShaderBufferInput* a_input) const
{
    for (const ShaderBufferInput& input : m_slotInputs)
    {
        if (input.BufferType == ShaderBufferType_CameraBuffer)
        {
            *a_input = input;

            return true;
        }
    }

    return false;
}

bool VulkanShaderData::GetDirectionalLightInput(ShaderBufferInput* a_input) const
{
    for (const ShaderBufferInput& input : m_slotInputs)
    {
        if (input.BufferType == ShaderBufferType_DirectionalLightBuffer)
        {
            *a_input = input;

            return true;
        }
    }

    return false;
}
bool VulkanShaderData::GetPointLightInput(ShaderBufferInput* a_input) const
{
    for (const ShaderBufferInput& input : m_slotInputs)
    {
        if (input.BufferType == ShaderBufferType_PointLightBuffer)
        {
            *a_input = input;

            return true;
        }
    }

    return false;
}
bool VulkanShaderData::GetSpotLightInput(ShaderBufferInput* a_input) const
{
    for (const ShaderBufferInput& input : m_slotInputs)
    {
        if (input.BufferType == ShaderBufferType_SpotLightBuffer)
        {
            *a_input = input;

            return true;
        }
    }

    return false;
}

bool VulkanShaderData::GetBatchDirectionalLightInput(ShaderBufferInput* a_input) const
{
    for (const ShaderBufferInput& input : m_slotInputs)
    {
        if (input.BufferType == ShaderBufferType_SSDirectionalLightBuffer)
        {
            *a_input = input;

            return true;
        }
    }

    return false;
}
bool VulkanShaderData::GetBatchPointLightInput(ShaderBufferInput* a_input) const
{
    for (const ShaderBufferInput& input : m_slotInputs)
    {
        if (input.BufferType == ShaderBufferType_SSPointLightBuffer)
        {
            *a_input = input;

            return true;
        }
    }

    return false;
}
bool VulkanShaderData::GetBatchSpotLightInput(ShaderBufferInput* a_input) const
{
    for (const ShaderBufferInput& input : m_slotInputs)
    {
        if (input.BufferType == ShaderBufferType_SSSpotLightBuffer)
        {
            *a_input = input;

            return true;
        }
    }

    return false;
}

bool VulkanShaderData::GetBatchModelBufferInput(ShaderBufferInput* a_input) const
{
    for (const ShaderBufferInput& input : m_slotInputs)
    {
        if (input.BufferType == ShaderBufferType_SSModelBuffer)
        {
            *a_input = input;

            return true;
        }
    }

    return false;
}
bool VulkanShaderData::GetShadowBatchModelBufferInput(ShaderBufferInput* a_input) const
{
    for (const ShaderBufferInput& input : m_shadowSlotInputs)
    {
        if (input.BufferType == ShaderBufferType_SSModelBuffer)
        {
            *a_input = input;

            return true;
        }
    }

    return false;
}

bool VulkanShaderData::GetBoneBufferInput(ShaderBufferInput* a_input) const
{
    for (const ShaderBufferInput& input : m_slotInputs)
    {
        if (input.BufferType == ShaderBufferType_SSBoneBuffer)
        {
            *a_input = input;

            return true;
        }
    }

    return false;
}
#endif