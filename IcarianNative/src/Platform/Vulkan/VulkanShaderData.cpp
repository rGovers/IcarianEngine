// Icarian Engine - C# Game Engine
// 
// License at end of file.

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/VulkanShaderData.h"

#include "Core/IcarianDefer.h"
#include "Core/ShaderBuffers.h"
#include "DataTypes/Allocator.h"
#include "Rendering/UI/UIElement.h"
#include "Rendering/Vulkan/Shaders/VulkanMeshShader.h"
#include "Rendering/Vulkan/Shaders/VulkanPixelShader.h"
#include "Rendering/Vulkan/Shaders/VulkanTaskShader.h"
#include "Rendering/Vulkan/Shaders/VulkanVertexShader.h"
#include "Rendering/Vulkan/VulkanDepthCubeRenderTexture.h"
#include "Rendering/Vulkan/VulkanDepthRenderTexture.h"
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
    VulkanRenderEngineBackend*  m_engine;

    Array<VulkanPushDescriptor> m_pushDescriptors;
    Array<VulkanPushDescriptor> m_shadowPushDescriptors;

    vk::PipelineLayout          m_layout;
    vk::PipelineLayout          m_shadowLayout;

protected:

public:
    VulkanShaderDataDeletionObject(VulkanRenderEngineBackend* a_engine, const Array<VulkanPushDescriptor>& a_pushDescriptors, const Array<VulkanPushDescriptor>& a_shadowPushDescriptors, vk::PipelineLayout a_layout, vk::PipelineLayout a_shadowLayout)
    {
        m_engine = a_engine;

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

        for (const VulkanPushDescriptor& d : m_pushDescriptors)
        {
            device.destroyDescriptorSetLayout(d.DescriptorLayout);
        }

        for (const VulkanPushDescriptor& d : m_shadowPushDescriptors)
        {
            device.destroyDescriptorSetLayout(d.DescriptorLayout);
        }

        device.destroyPipelineLayout(m_layout);
        if (m_shadowLayout != vk::PipelineLayout(nullptr))
        {
            device.destroyPipelineLayout(m_shadowLayout);
        }
    }
};

constexpr static uint32_t GetBufferSize(e_ShaderBufferType a_type)
{
    switch (a_type)
    {
    case ShaderBufferType_PModelBuffer:
    {
        return sizeof(IcarianCore::ShaderModelBuffer);
    }
    case ShaderBufferType_PUIBuffer:
    {
        return sizeof(IcarianCore::ShaderUIBuffer);
    }
    case ShaderBufferType_PShadowLightBuffer:
    {
        return sizeof(IcarianCore::ShaderShadowLightBuffer);
    }
    case ShaderBufferType_CameraBuffer:
    {
        return sizeof(IcarianCore::ShaderCameraBuffer);
    }
    case ShaderBufferType_AmbientLightBuffer:
    {
        return sizeof(IcarianCore::ShaderAmbientLightBuffer);
    }
    case ShaderBufferType_DirectionalLightBuffer:
    {
        return sizeof(IcarianCore::ShaderDirectionalLightBuffer);
    }
    case ShaderBufferType_PointLightBuffer:
    {
        return sizeof(IcarianCore::ShaderPointLightBuffer);
    }
    case ShaderBufferType_SpotLightBuffer:
    {
        return sizeof(IcarianCore::ShaderSpotLightBuffer);
    }
    case ShaderBufferType_ShadowLightBuffer:
    {
        return sizeof(IcarianCore::ShaderShadowLightBuffer);
    }
    default:
    {
        IERROR("Invalid shader buffer type");

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
    case ShaderBufferType_ShadowTexture2D:
    case ShaderBufferType_ShadowTextureCube:
    case ShaderBufferType_AShadowTexture2D:
    {
        return vk::DescriptorType::eCombinedImageSampler;
    }
    case ShaderBufferType_SSParticleBuffer:
    case ShaderBufferType_SSBoneBuffer:
    case ShaderBufferType_SSModelBuffer:
    case ShaderBufferType_SSAmbientLightBuffer:
    case ShaderBufferType_SSDirectionalLightBuffer:
    case ShaderBufferType_SSPointLightBuffer:
    case ShaderBufferType_SSSpotLightBuffer:
    case ShaderBufferType_SSShadowLightBuffer:
    {
        return vk::DescriptorType::eStorageBuffer;
    }
    default:
    {
        return vk::DescriptorType::eUniformBuffer;
    }
    }

    IERROR("Invalid Vulkan descriptor type");

    return vk::DescriptorType::eUniformBuffer;
}

struct Input
{
    uint32_t Slot;
    vk::DescriptorSetLayoutBinding Binding;
};

static void GetLayoutInfo(const Array<VulkanShaderInput, RenderScratchAlloc>& a_inputs, Array<vk::PushConstantRange, RenderScratchAlloc>* a_pushConstants, Array<Input, RenderScratchAlloc>* a_pushBindings)
{
    const uint32_t inputCount = a_inputs.Size();

    for (uint32_t i = 0; i < inputCount; ++i)
    {
        const VulkanShaderInput& input = a_inputs[i];
        
        switch (input.BufferType)
        {
        case ShaderBufferType_PModelBuffer:
        case ShaderBufferType_PUIBuffer:
        case ShaderBufferType_PShadowLightBuffer:
        {
            const vk::PushConstantRange pushConstant = vk::PushConstantRange
            (
                input.StageFlags,
                0,
                GetBufferSize(input.BufferType)
            );

            a_pushConstants->Push(pushConstant);

            break;
        }
        case ShaderBufferType_CameraBuffer:
        case ShaderBufferType_AmbientLightBuffer:
        case ShaderBufferType_DirectionalLightBuffer:
        case ShaderBufferType_PointLightBuffer:
        case ShaderBufferType_SpotLightBuffer:
        case ShaderBufferType_ShadowLightBuffer:
        case ShaderBufferType_Texture:
        case ShaderBufferType_TimeBuffer:
        case ShaderBufferType_PushTexture:
        case ShaderBufferType_ShadowTexture2D:
        case ShaderBufferType_ShadowTextureCube:
        case ShaderBufferType_SSParticleBuffer:
        case ShaderBufferType_SSModelBuffer:
        case ShaderBufferType_SSBoneBuffer:
        case ShaderBufferType_SSAmbientLightBuffer:
        case ShaderBufferType_SSDirectionalLightBuffer:
        case ShaderBufferType_SSPointLightBuffer:
        case ShaderBufferType_SSSpotLightBuffer:
        case ShaderBufferType_SSShadowLightBuffer:
        case ShaderBufferType_UserUBO:
        {
            const vk::DescriptorSetLayoutBinding binding = vk::DescriptorSetLayoutBinding
            (
                input.Slot,
                GetDescriptorType(input.BufferType),
                1,
                input.StageFlags
            );

            const Input in = 
            {
                .Slot = i,
                .Binding = binding
            };

            a_pushBindings->Push(in);

            break;
        }
        case ShaderBufferType_AShadowTexture2D:
        {
            const vk::DescriptorSetLayoutBinding binding = vk::DescriptorSetLayoutBinding
            (
                input.Slot, 
                GetDescriptorType(input.BufferType), 
                input.Count,
                input.StageFlags
            );

            const Input in = 
            {
                .Slot = i, 
                .Binding = binding
            };

            a_pushBindings->Push(in);

            break;
        }
        default:
        {
            IERROR("Invalid Vulkan shader layout info");

            break;
        }
        }
    }
}

static vk::DescriptorImageInfo GetDescriptorImageInfo(const TextureSamplerBuffer& a_baseSampler, const VulkanTextureSampler* a_sampler, VulkanGraphicsEngine* a_engine)
{
    vk::DescriptorImageInfo imageInfo;
    imageInfo.sampler = a_sampler->GetSampler();
    
    switch (a_baseSampler.TextureMode)
    {
    case TextureMode_Texture:
    {
        const VulkanTexture* texture = a_engine->GetTexture(a_baseSampler.Addr);

        imageInfo.imageView = texture->GetImageView();
        imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

        break;
    }
    case TextureMode_RenderTexture:
    {
        const VulkanRenderTexture* renderTexture = a_engine->GetRenderTexture(a_baseSampler.Addr);

        imageInfo.imageView = renderTexture->GetImageView(a_baseSampler.Slot);
        imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

        break;
    }
    case TextureMode_RenderTextureDepth:
    {
        const VulkanRenderTexture* renderTexture = a_engine->GetRenderTexture(a_baseSampler.Addr);

        imageInfo.imageView = renderTexture->GetDepthImageView();
        imageInfo.imageLayout = vk::ImageLayout::eDepthStencilReadOnlyOptimal;

        break;
    }
    case TextureMode_DepthRenderTexture:
    {
        const VulkanDepthRenderTexture* renderTexture = a_engine->GetDepthRenderTexture(a_baseSampler.Addr);

        imageInfo.imageView = renderTexture->GetImageView();
        imageInfo.imageLayout = vk::ImageLayout::eDepthStencilReadOnlyOptimal;

        break;
    }
    case TextureMode_DepthCubeRenderTexture:
    {
        const VulkanDepthCubeRenderTexture* renderTexture = a_engine->GetDepthCubeRenderTexture(a_baseSampler.Addr);

        imageInfo.imageView = renderTexture->GetImageView();
        imageInfo.imageLayout = vk::ImageLayout::eDepthStencilReadOnlyOptimal;

        break;
    }
    default:
    {
        IERROR("Invalid texture mode");

        return vk::DescriptorImageInfo();
    }
    }

    return imageInfo;
}

static void GeneratePushBindings(const Array<Input, RenderScratchAlloc>& a_bindings, const Array<VulkanShaderInput, RenderScratchAlloc>& a_inputs, const vk::Device a_device, Array<vk::DescriptorSetLayout, RenderScratchAlloc>* a_layouts, Array<VulkanPushDescriptor>* a_pushDescriptors)
{
    for (const Input& i : a_bindings)
    {
        const vk::DescriptorSetLayoutCreateInfo descriptorLayoutInfo = vk::DescriptorSetLayoutCreateInfo
        (
            { },
            1,
            &i.Binding
        );

        const VulkanShaderInput& input = a_inputs[i.Slot];

        vk::DescriptorSetLayout layout;
        VKRESERRMSG(a_device.createDescriptorSetLayout(&descriptorLayoutInfo, nullptr, &layout), "Failed to create Push Descriptor Layout");
        a_layouts->Push(layout);

        const VulkanPushDescriptor d = 
        {
            .Slot = input.Slot,
            .Count = input.Count,
            .DescriptorLayout = layout
        };
            
        a_pushDescriptors->Push(d);
    }
}

static void PushVulkanShaderBufferInput(Array<VulkanShaderInput, RenderScratchAlloc>* a_inputs, const ShaderBufferInput& a_bufferInput, vk::ShaderStageFlags a_stage)
{
    for (VulkanShaderInput& i : *a_inputs)
    {
        if (i.Slot == a_bufferInput.Slot)
        {
            if (i.BufferType != a_bufferInput.BufferType)
            {
                IERROR("Vulkan Shader type mixmatch: " + std::to_string(a_bufferInput.Slot));
            }

            i.StageFlags |= a_stage;

            return;
        }
    }

    const VulkanShaderInput vInput = 
    {
        .Slot = a_bufferInput.Slot,
        .Count = a_bufferInput.Count,
        .BufferType = a_bufferInput.BufferType,
        .StageFlags = a_stage
    };

    a_inputs->Push(vInput);
}

VulkanShaderData::VulkanShaderData(VulkanRenderEngineBackend* a_engine, VulkanGraphicsEngine* a_gEngine, const RenderProgram& a_program, Allocator* a_allocator)
{
    RENDERSCRATCHFRAME;
    
    TRACE("Creating Shader Data");
    m_allocator = a_allocator;

    m_engine = a_engine;
    m_gEngine = a_gEngine;

    m_shadowLayout = nullptr;

    m_userUniformBuffer = nullptr;

    m_shadowSlotInputCount = 0;
    m_shadowSlotInputs = nullptr;

    const vk::Device device = m_engine->GetLogicalDevice();

    Array<VulkanShaderInput, RenderScratchAlloc> vulkanInputs;

    if (a_program.VertexShader != uint32_t(-1))
    {
        switch (a_program.MaterialMode) 
        {
        case MaterialMode_BaseVertex:
        {
            const VulkanVertexShader* vertexShader = m_gEngine->GetVertexShader(a_program.VertexShader);
            IVERIFY(vertexShader != nullptr);

            const uint32_t inputCount = vertexShader->GetShaderInputCount();
            for (uint32_t i = 0; i < inputCount; ++i)
            {
                const ShaderBufferInput input = vertexShader->GetShaderInput(i);

                PushVulkanShaderBufferInput(&vulkanInputs, input, vk::ShaderStageFlagBits::eVertex);
            }

            break;
        }
        case MaterialMode_BaseMesh:
        {
            const VulkanMeshShader* meshShader = m_gEngine->GetMeshShader(a_program.VertexShader);
            IVERIFY(meshShader != nullptr);

            const uint32_t inputCount = meshShader->GetShaderInputCount();
            for (uint32_t i = 0; i < inputCount; ++i)
            {
                const ShaderBufferInput input = meshShader->GetShaderInput(i);

                PushVulkanShaderBufferInput(&vulkanInputs, input, vk::ShaderStageFlagBits::eMeshEXT);
            }

            if (a_program.ExtraShader != uint32_t(-1))
            {
                const VulkanTaskShader* taskShader = m_gEngine->GetTaskShader(a_program.ExtraShader);
                IVERIFY(taskShader != nullptr);

                const uint32_t inputCount = taskShader->GetShaderInputCount();
                for (uint32_t i = 0; i < inputCount; ++i)
                {
                    const ShaderBufferInput input = taskShader->GetShaderInput(i);

                    PushVulkanShaderBufferInput(&vulkanInputs, input, vk::ShaderStageFlagBits::eTaskEXT);
                }
            }

            break;
        }
        }
    }

    if (a_program.PixelShader != uint32_t(-1))
    {
        const VulkanPixelShader* pixelShader = m_gEngine->GetPixelShader(a_program.PixelShader);
        IVERIFY(pixelShader != nullptr);

        const uint32_t inputCount = pixelShader->GetShaderInputCount();
        for (uint32_t i = 0; i < inputCount; ++i)
        {
            const ShaderBufferInput input = pixelShader->GetShaderInput(i);

            PushVulkanShaderBufferInput(&vulkanInputs, input, vk::ShaderStageFlagBits::eFragment);
        }
    }

    m_slotInputCount = vulkanInputs.Size();
    m_slotInputs = m_allocator->TAllocate<VulkanShaderInput>(m_slotInputCount);

    for (uint32_t i = 0; i < m_slotInputCount; ++i)
    {
        m_slotInputs[i] = vulkanInputs[i];
    }

    Array<vk::PushConstantRange, RenderScratchAlloc> pushConstants;
    Array<Input, RenderScratchAlloc> pushBindings;
    GetLayoutInfo(vulkanInputs, &pushConstants, &pushBindings);

    Array<vk::DescriptorSetLayout, RenderScratchAlloc> layouts;
    GeneratePushBindings(pushBindings, vulkanInputs, device, &layouts, &m_pushDescriptors);    

    const vk::PipelineLayoutCreateInfo pipelineLayoutInfo = vk::PipelineLayoutCreateInfo
    (
        { },
        layouts.Size(),
        layouts.Data(),
        pushConstants.Size(),
        pushConstants.Data()
    );

    if (a_program.UBOData != NULL && a_program.UBODataSize > 0)
    {
        m_userUniformBuffer = m_allocator->Create<VulkanUniformBuffer>(m_engine, a_program.UBODataSize);
    }

    TRACE("Creating Pipeline Layout");
    VKRESERRMSG(device.createPipelineLayout(&pipelineLayoutInfo, nullptr, &m_layout), "Failed to create PipelineLayout");

    if (a_program.ShadowVertexShader != uint32_t(-1))
    {
        const VulkanVertexShader* vertexShader = m_gEngine->GetVertexShader(a_program.ShadowVertexShader);

        vulkanInputs.Clear();

        const uint32_t inputCount = vertexShader->GetShaderInputCount();
        for (uint32_t i = 0; i < inputCount; ++i)
        {
            const ShaderBufferInput input = vertexShader->GetShaderInput(i);

            PushVulkanShaderBufferInput(&vulkanInputs, input, vk::ShaderStageFlagBits::eVertex);
        }

        Array<vk::PushConstantRange, RenderScratchAlloc> shadowPushConstants;
        Array<Input, RenderScratchAlloc> shadowPushBindings;
        GetLayoutInfo(vulkanInputs, &shadowPushConstants, &shadowPushBindings);

        Array<vk::DescriptorSetLayout, RenderScratchAlloc> shadowLayouts;
        GeneratePushBindings(shadowPushBindings, vulkanInputs, device, &shadowLayouts, &m_shadowPushDescriptors);

        m_shadowSlotInputCount = vulkanInputs.Size();
        m_shadowSlotInputs = m_allocator->TAllocate<VulkanShaderInput>(m_shadowSlotInputCount);
        for (uint32_t i = 0; i < m_shadowSlotInputCount; ++i)
        {
            m_shadowSlotInputs[i] = vulkanInputs[i];
        }

        const vk::PipelineLayoutCreateInfo shadowPipelineLayoutInfo = vk::PipelineLayoutCreateInfo
        (
            { },
            shadowLayouts.Size(),
            shadowLayouts.Data(),
            shadowPushConstants.Size(),
            shadowPushConstants.Data()
        );

        TRACE("Creating Shadow Pipeline Layout");
        VKRESERRMSG(device.createPipelineLayout(&shadowPipelineLayoutInfo, nullptr, &m_shadowLayout), "Failed to create Shadow PipelineLayout");
    }
}
VulkanShaderData::VulkanShaderData(VulkanRenderEngineBackend* a_engine, VulkanGraphicsEngine* a_gEngine, const VulkanDecalShader* a_shader, Allocator* a_allocator)
{
    TRACE("Creating Decal Shader Data");
    RENDERSCRATCHFRAME;

    m_allocator = a_allocator;

    m_engine = a_engine;
    m_gEngine = a_gEngine;

    m_shadowLayout = nullptr;

    m_userUniformBuffer = nullptr;

    m_shadowSlotInputCount = 0;
    m_shadowSlotInputs = nullptr;

    const vk::Device device = m_engine->GetLogicalDevice();

    Array<VulkanShaderInput, RenderScratchAlloc> vulkanInputs;

    const VulkanVertexShader* vertexShader = a_shader->GetVertexShader();
    IVERIFY(vertexShader != nullptr);

    const uint32_t vertexInputCount = vertexShader->GetShaderInputCount();
    for (uint32_t i = 0; i < vertexInputCount; ++i)
    {
        const ShaderBufferInput input = vertexShader->GetShaderInput(i);
        
        PushVulkanShaderBufferInput(&vulkanInputs, input, vk::ShaderStageFlagBits::eVertex);
    }

    const VulkanPixelShader* pixelShader = a_shader->GetPixelShader();
    IVERIFY(pixelShader != nullptr);

    const uint32_t pixelInputCount = pixelShader->GetShaderInputCount();
    for (uint32_t i = 0; i < pixelInputCount; ++i)
    {
        const ShaderBufferInput input = pixelShader->GetShaderInput(i);

        PushVulkanShaderBufferInput(&vulkanInputs, input, vk::ShaderStageFlagBits::eFragment);
    }

    m_slotInputCount = vulkanInputs.Size();
    m_slotInputs = m_allocator->TAllocate<VulkanShaderInput>(m_slotInputCount);

    for (uint32_t i = 0; i < m_slotInputCount; ++i)
    {
        m_slotInputs[i] = vulkanInputs[i];
    }

    Array<vk::PushConstantRange, RenderScratchAlloc> pushConstants;
    Array<Input, RenderScratchAlloc> pushBindings;
    GetLayoutInfo(vulkanInputs, &pushConstants, &pushBindings);

    Array<vk::DescriptorSetLayout, RenderScratchAlloc> layouts;
    GeneratePushBindings(pushBindings, vulkanInputs, device, &layouts, &m_pushDescriptors);    

    const vk::PipelineLayoutCreateInfo pipelineLayoutInfo = vk::PipelineLayoutCreateInfo
    (
        { },
        layouts.Size(),
        layouts.Data(),
        pushConstants.Size(),
        pushConstants.Data()
    );

    TRACE("Creating Pipeline Layout");
    VKRESERRMSG(device.createPipelineLayout(&pipelineLayoutInfo, nullptr, &m_layout), "Failed to create PipelineLayout");
}
VulkanShaderData::~VulkanShaderData()
{
    TRACE("Queueing Shader Data for deletion");
    m_engine->PushDeletionObject<VulkanShaderDataDeletionObject>(m_engine, m_pushDescriptors, m_shadowPushDescriptors, m_layout, m_shadowLayout);    

    if (m_userUniformBuffer != nullptr)
    {
        m_allocator->Destroy(m_userUniformBuffer);
    }

    m_allocator->Free(m_slotInputs);
    
    if (m_shadowSlotInputs != nullptr)
    {
        m_allocator->Free(m_shadowSlotInputs);
    }
}

void VulkanShaderData::SetTexture(uint32_t a_slot, uint32_t a_sampleAddr)
{
    for (VulkanTextureBinding& b : m_textures)
    {
        if (b.Slot == a_slot)
        {
            b.SamplerAddr = a_sampleAddr;

            return;
        }
    }

    const VulkanTextureBinding binding =
    {
        .Slot = a_slot,
        .SamplerAddr = a_sampleAddr
    };

    m_textures.Push(binding);
}

void VulkanShaderData::PushTexture(vk::CommandBuffer a_commandBuffer, uint32_t a_slot, const TextureSamplerBuffer& a_sampler, uint32_t a_index) const
{   
    const vk::Device device = m_engine->GetLogicalDevice();

    const VulkanTextureSampler* vSampler = (VulkanTextureSampler*)a_sampler.Data;
    IVERIFY(vSampler != nullptr);

    for (const VulkanPushDescriptor& d : m_pushDescriptors)
    {
        if (d.Slot == a_slot)
        {
            VulkanPushPool* pool = m_engine->GetPushPool();
            IVERIFY(pool != nullptr);

            vk::DescriptorSet descriptorSet = pool->AllocateDescriptor(a_index, vk::DescriptorType::eCombinedImageSampler, &d.DescriptorLayout);

            const vk::DescriptorImageInfo imageInfo = GetDescriptorImageInfo(a_sampler, vSampler, m_gEngine);
            
            const vk::WriteDescriptorSet descriptorWrite = vk::WriteDescriptorSet
            (
                descriptorSet,
                d.Slot,
                0,
                1,
                vk::DescriptorType::eCombinedImageSampler,
                &imageInfo
            );

            device.updateDescriptorSets(1, &descriptorWrite, 0, nullptr);
            
            a_commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_layout, d.Slot, 1, &descriptorSet, 0, nullptr);

            return;
        }
    }

    IERROR("PushTexture binding not found");
}
void VulkanShaderData::PushTextures(vk::CommandBuffer a_commandBuffer, uint32_t a_slot, const TextureSamplerBuffer* a_samplers, uint32_t a_count, uint32_t a_index) const
{
    RENDERSCRATCHFRAME;

    const vk::Device device = m_engine->GetLogicalDevice();

    for (const VulkanPushDescriptor& d : m_pushDescriptors)
    {
        if (d.Slot == a_slot)
        {
            VulkanPushPool* pool = m_engine->GetPushPool();
            IVERIFY(pool != nullptr);

            const uint32_t size = d.Count;

            vk::DescriptorSet descriptorSet = pool->AllocateDescriptor(a_index, vk::DescriptorType::eCombinedImageSampler, &d.DescriptorLayout, size);

            const uint32_t min = glm::min(size, a_count);

            vk::DescriptorImageInfo* imageInfos = RenderScratchAlloc::TAllocate<vk::DescriptorImageInfo>(min);
            
            for (uint32_t i = 0; i < min; ++i)
            {
                const VulkanTextureSampler* vSampler = (VulkanTextureSampler*)a_samplers[i].Data;
                IVERIFY(vSampler != nullptr);

                imageInfos[i] = GetDescriptorImageInfo(a_samplers[i], vSampler, m_gEngine);
            }

            const vk::WriteDescriptorSet descriptorWrite = vk::WriteDescriptorSet
            (
                descriptorSet,
                d.Slot,
                0,
                min,
                vk::DescriptorType::eCombinedImageSampler,
                imageInfos
            );

            device.updateDescriptorSets(1, &descriptorWrite, 0, nullptr);

            a_commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_layout, d.Slot, 1, &descriptorSet, 0, nullptr);

            return;
        }
    }

    IERROR("PushTextures binding not found");
}

void VulkanShaderData::PushUniformBuffer(vk::CommandBuffer a_commandBuffer, uint32_t a_slot, const VulkanUniformBuffer* a_buffer, uint32_t a_index) const
{
    const vk::Device device = m_engine->GetLogicalDevice();

    for (const VulkanPushDescriptor& d : m_pushDescriptors)
    {
        if (d.Slot == a_slot)
        {
            VulkanPushPool* pool = m_engine->GetPushPool();
            
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
                d.Slot,
                0,
                1,
                vk::DescriptorType::eUniformBuffer,
                nullptr,
                &bufferInfo
            );

            device.updateDescriptorSets(1, &descriptorWrite, 0, nullptr);

            a_commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_layout, d.Slot, 1, &descriptorSet, 0, nullptr);

            return;
        }
    }

    IERROR("PushUniformBuffer binding not found");
}
void VulkanShaderData::PushShaderStorageObject(vk::CommandBuffer a_commandBuffer, uint32_t a_slot, const VulkanShaderStorageObject* a_object, uint32_t a_index) const
{
    return PushShaderStorageObject(a_commandBuffer, a_slot, a_object->GetBuffer(), a_index);   
}
void VulkanShaderData::PushShaderStorageObject(vk::CommandBuffer a_commandBuffer, uint32_t a_slot, vk::Buffer a_object, uint32_t a_index) const
{
    const vk::Device device = m_engine->GetLogicalDevice();

    for (const VulkanPushDescriptor& d : m_pushDescriptors)
    {
        if (d.Slot == a_slot)
        {
            VulkanPushPool* pool = m_engine->GetPushPool();

            vk::DescriptorSet descriptorSet = pool->AllocateDescriptor(a_index, vk::DescriptorType::eStorageBuffer, &d.DescriptorLayout);

            const vk::DescriptorBufferInfo bufferInfo = vk::DescriptorBufferInfo
            (
                a_object,
                0,
                VK_WHOLE_SIZE
            );

            const vk::WriteDescriptorSet descriptorWrite = vk::WriteDescriptorSet
            (
                descriptorSet,
                d.Slot,
                0,
                1,
                vk::DescriptorType::eStorageBuffer,
                nullptr,
                &bufferInfo
            );

            device.updateDescriptorSets(1, &descriptorWrite, 0, nullptr);

            a_commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_layout, d.Slot, 1, &descriptorSet, 0, nullptr);

            return;
        }
    }

    IERROR("PushShaderStorageObject binding not found");
}

void VulkanShaderData::PushShadowTexture(vk::CommandBuffer a_commandBuffer, uint32_t a_slot, const TextureSamplerBuffer& a_sampler, uint32_t a_index) const
{
    const vk::Device device = m_engine->GetLogicalDevice();

    const VulkanTextureSampler* vSampler = (VulkanTextureSampler*)a_sampler.Data;
    IVERIFY(vSampler != nullptr);

    for (const VulkanPushDescriptor& d : m_shadowPushDescriptors)
    {
        if (d.Slot == a_slot)
        {
            VulkanPushPool* pool = m_engine->GetPushPool();

            vk::DescriptorSet descriptorSet = pool->AllocateDescriptor(a_index, vk::DescriptorType::eCombinedImageSampler, &d.DescriptorLayout);

            const vk::DescriptorImageInfo imageInfo = GetDescriptorImageInfo(a_sampler, vSampler, m_gEngine);

            const vk::WriteDescriptorSet descriptorWrite = vk::WriteDescriptorSet
            (
                descriptorSet,
                d.Slot,
                0,
                1,
                vk::DescriptorType::eCombinedImageSampler,
                &imageInfo
            );

            device.updateDescriptorSets(1, &descriptorWrite, 0, nullptr);

            a_commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_shadowLayout, d.Slot, 1, &descriptorSet, 0, nullptr);

            return;
        }
    }

    IERROR("PushShadowTexture binding not found");
}
void VulkanShaderData::PushShadowUniformBuffer(vk::CommandBuffer a_commandBuffer, uint32_t a_slot, const VulkanUniformBuffer* a_buffer, uint32_t a_index) const
{
    const vk::Device device = m_engine->GetLogicalDevice();

    for (const VulkanPushDescriptor& d : m_shadowPushDescriptors)
    {
        if (d.Slot == a_slot)
        {
            VulkanPushPool* pool = m_engine->GetPushPool();

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
                d.Slot,
                0,
                1,
                vk::DescriptorType::eUniformBuffer,
                nullptr,
                &bufferInfo
            );

            device.updateDescriptorSets(1, &descriptorWrite, 0, nullptr);

            a_commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_shadowLayout, d.Slot, 1, &descriptorSet, 0, nullptr);

            return;
        }
    }

    IERROR("PushShadowUniformBuffer binding not found");
}
void VulkanShaderData::PushShadowShaderStorageObject(vk::CommandBuffer a_commandBuffer, uint32_t a_slot, const VulkanShaderStorageObject* a_object, uint32_t a_index) const
{
    const vk::Device device = m_engine->GetLogicalDevice();

    for (const VulkanPushDescriptor& d : m_shadowPushDescriptors)
    {
        if (d.Slot == a_slot)
        {
            VulkanPushPool* pool = m_engine->GetPushPool();

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
                d.Slot,
                0,
                1,
                vk::DescriptorType::eStorageBuffer,
                nullptr,
                &bufferInfo
            );

            device.updateDescriptorSets(1, &descriptorWrite, 0, nullptr);

            a_commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_shadowLayout, d.Slot, 1, &descriptorSet, 0, nullptr);

            return;
        }
    }

    IERROR("PushShadowShaderStorageObject binding not found");
}

void VulkanShaderData::UpdateTransformBuffer(vk::CommandBuffer a_commandBuffer, const glm::mat4& a_transform) const
{
    for (uint32_t i = 0; i < m_slotInputCount; ++i)
    {
        const VulkanShaderInput& input = m_slotInputs[i];

        if (input.BufferType != ShaderBufferType_PModelBuffer)
        {
            continue;
        }

        const IcarianCore::ShaderModelBuffer buffer =
        {
            .Model = a_transform,
            .InvModel = glm::inverse(a_transform)
        };

        a_commandBuffer.pushConstants(m_layout, input.StageFlags, 0, sizeof(IcarianCore::ShaderModelBuffer), &buffer);

        return;
    }
}
void VulkanShaderData::UpdateShadowTransformBuffer(vk::CommandBuffer a_commandBuffer, const glm::mat4& a_transform) const
{
    for (uint32_t i = 0; i < m_shadowSlotInputCount; ++i)
    {
        const VulkanShaderInput& input = m_shadowSlotInputs[i];

        if (input.BufferType != ShaderBufferType_PModelBuffer)
        {
            continue;
        }

        const IcarianCore::ShaderModelBuffer buffer =
        {
            .Model = a_transform,
            .InvModel = glm::inverse(a_transform)
        };

        a_commandBuffer.pushConstants(m_shadowLayout, input.StageFlags, 0, sizeof(IcarianCore::ShaderModelBuffer), &buffer);

        return;
    }
}

void VulkanShaderData::UpdateUIBuffer(vk::CommandBuffer a_commandBuffer, const UIElement* a_element) const
{
    for (uint32_t i = 0; i < m_slotInputCount; ++i)
    {
        const VulkanShaderInput& input = m_slotInputs[i];

        if (input.BufferType != ShaderBufferType_PUIBuffer)
        {
            continue;
        }

        const IcarianCore::ShaderUIBuffer buffer =
        {
            .Color = a_element->GetColor()
        };

        a_commandBuffer.pushConstants(m_layout, input.StageFlags, 0, sizeof(IcarianCore::ShaderUIBuffer), &buffer);
    }
}

void VulkanShaderData::UpdateShadowLightBuffer(vk::CommandBuffer a_commandBuffer, const glm::mat4& a_lvp, float a_split) const
{
    for (uint32_t i = 0; i < m_shadowSlotInputCount; ++i)
    {
        const VulkanShaderInput& input = m_shadowSlotInputs[i];

        if (input.BufferType != ShaderBufferType_PShadowLightBuffer)
        {
            continue;
        }

        const IcarianCore::ShaderShadowLightBuffer buffer =
        {
            .LVP = a_lvp,
            .Split = a_split
        };

        a_commandBuffer.pushConstants(m_shadowLayout, input.StageFlags, 0, sizeof(IcarianCore::ShaderShadowLightBuffer), &buffer);

        return;
    }
}

void VulkanShaderData::Update(uint32_t a_index, const RenderProgram& a_program)
{
    // Want the user to be able to update the uniform buffer whenever they want
    // However updating mid render can cause issues
    if (m_userUniformBuffer != nullptr)
    {
        m_userUniformBuffer->SetData(a_index, a_program.UBOData);
    }
}
void VulkanShaderData::BindShadow(uint32_t a_index, vk::CommandBuffer a_commandBuffer) const
{
    if (m_userUniformBuffer != nullptr)
    {
        for (uint32_t i = 0; i < m_shadowSlotInputCount; ++i)
        {
            const VulkanShaderInput& input = m_shadowSlotInputs[i];

            if (input.BufferType != ShaderBufferType_UserUBO)
            {
                continue;
            }

            PushUniformBuffer(a_commandBuffer, input.Slot, m_userUniformBuffer, a_index);
        }
    }
}
void VulkanShaderData::Bind(uint32_t a_index, vk::CommandBuffer a_commandBuffer) const
{
    if (m_userUniformBuffer != nullptr)
    {
        for (uint32_t i = 0; i < m_slotInputCount; ++i)
        {
            const VulkanShaderInput& input = m_slotInputs[i];

            if (input.BufferType != ShaderBufferType_UserUBO)
            {
                continue;
            }

            PushUniformBuffer(a_commandBuffer, input.Slot, m_userUniformBuffer, a_index);
        }
    }

    for (const VulkanTextureBinding& b : m_textures)
    {
        const TextureSamplerBuffer sampler = m_gEngine->GetTextureSampler(b.SamplerAddr);

        PushTexture(a_commandBuffer, b.Slot, sampler, a_index);
    }
}

bool VulkanShaderData::GetShaderBufferInput(e_ShaderBufferType a_bufferType, ShaderBufferInput* a_input) const
{
    for (uint32_t i = 0; i < m_slotInputCount; ++i)
    {
        const VulkanShaderInput& input = m_slotInputs[i];

        if (input.BufferType != a_bufferType)
        {
            continue;
        }

        const ShaderBufferInput sInput = 
        {
            .Slot = (uint16_t)input.Slot,
            .BufferType = input.BufferType,
            .Count = (uint16_t)input.Count
        };

        *a_input = sInput;
        
        return true;
    }

    return false;
}
bool VulkanShaderData::GetShadowShaderBufferInput(e_ShaderBufferType a_bufferType, ShaderBufferInput* a_input) const
{
    for (uint32_t i = 0; i < m_shadowSlotInputCount; ++i)
    {
        const VulkanShaderInput& input = m_shadowSlotInputs[i];

        if (input.BufferType != a_bufferType)
        {
            continue;
        }

        const ShaderBufferInput sInput = 
        {
            .Slot = (uint16_t)input.Slot,
            .BufferType = input.BufferType,
            .Count = (uint16_t)input.Count
        };

        *a_input = sInput;

        return true;
    }

    return false;
}

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