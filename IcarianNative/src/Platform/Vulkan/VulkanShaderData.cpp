// Icarian Engine - C# Game Engine
// 
// License at end of file.

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/VulkanShaderData.h"

#include "Core/IcarianLambda.h"
#include "Core/ShaderBuffers.h"
#include "DataTypes/Allocators/Allocator.h"
#include "Rendering/FlareShader.h"
#include "Rendering/UI/UIElement.h"
#include "Rendering/Vulkan/Shaders/VulkanComputeShader.h"
#include "Rendering/Vulkan/Shaders/VulkanMeshShader.h"
#include "Rendering/Vulkan/Shaders/VulkanPixelShader.h"
#include "Rendering/Vulkan/Shaders/VulkanTaskShader.h"
#include "Rendering/Vulkan/Shaders/VulkanVertexShader.h"
#include "Rendering/Vulkan/VulkanDepthCubeRenderTexture.h"
#include "Rendering/Vulkan/VulkanDepthRenderTexture.h"
#include "Rendering/Vulkan/VulkanGraphicsEngine.h"
#include "Rendering/Vulkan/VulkanMesh.h"
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
    VulkanRenderEngineBackend* m_engine;

    VulkanPushDescriptor*      m_pushDescriptors;
    uint32_t                   m_pushDescriptorCount;

    vk::PipelineLayout         m_layout;

protected:

public:
    VulkanShaderDataDeletionObject
    (
        VulkanRenderEngineBackend* a_engine,
        const VulkanPushDescriptor* a_pushDescriptors,
        uint32_t a_pushDescriptorCount,
        vk::PipelineLayout a_layout
    )
    {
        m_engine = a_engine;

        m_layout = a_layout;

        Allocator* allocator = m_engine->GetDeletionAllocator();

        m_pushDescriptorCount = a_pushDescriptorCount;
        m_pushDescriptors = allocator->TAllocate<VulkanPushDescriptor>(m_pushDescriptorCount);
        for (uint32_t i = 0; i < m_pushDescriptorCount; ++i)
        {
            m_pushDescriptors[i] = a_pushDescriptors[i];
        }
    }
    virtual ~VulkanShaderDataDeletionObject()
    {
        Allocator* allocator = m_engine->GetDeletionAllocator();

        allocator->Free(m_pushDescriptors);
    }

    virtual void Destroy()
    {
        const vk::Device device = m_engine->GetLogicalDevice();

        for (uint32_t i = 0; i < m_pushDescriptorCount; ++i)
        {
            device.destroyDescriptorSetLayout(m_pushDescriptors[i].DescriptorLayout);
        }

        device.destroyPipelineLayout(m_layout);
    }
};

constexpr static vk::PipelineBindPoint GetBindPoint(e_MaterialMode a_materialMode)
{
    switch (a_materialMode) 
    {
    case MaterialMode_BaseMesh:
    case MaterialMode_BaseVertex:
    {
        return vk::PipelineBindPoint::eGraphics;
    }
    case MaterialMode_Compute:
    {
        return vk::PipelineBindPoint::eCompute;
    }
    default:
    {
        break;
    }
    }

    IERROR("Invalid MaterialMode Bind Point");

    return vk::PipelineBindPoint::eGraphics;
}

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
    case ShaderBufferType_BufferTexture:
    {
        return vk::DescriptorType::eStorageImage;
    }
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
    case ShaderBufferType_UserArray:
    case ShaderBufferType_MeshVertex:
    case ShaderBufferType_MeshletVertices:
    case ShaderBufferType_MeshletTriangles:
    case ShaderBufferType_Meshlet:
    case ShaderBufferType_OutMeshEmulationIndex:
    case ShaderBufferType_OutMeshEmulationVertex:
    case ShaderBufferType_OutMeshEmulationDraw:
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

static void GetLayoutInfo(const Array<VulkanShaderInput>& a_inputs, Array<vk::PushConstantRange>* a_pushConstants, Array<Input>* a_pushBindings)
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
            const uint32_t size = GetBufferSize(input.BufferType);

            const vk::PushConstantRange pushConstant = vk::PushConstantRange
            (
                input.StageFlags,
                0,
                size
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
        case ShaderBufferType_BufferTexture:
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
        case ShaderBufferType_UserArray:
        case ShaderBufferType_UserUBO:
        case ShaderBufferType_MeshVertex:
        case ShaderBufferType_MeshletVertices:
        case ShaderBufferType_MeshletTriangles:
        case ShaderBufferType_Meshlet:
        case ShaderBufferType_OutMeshEmulationIndex:
        case ShaderBufferType_OutMeshEmulationVertex:
        case ShaderBufferType_OutMeshEmulationDraw:
        {
            const vk::DescriptorType type = GetDescriptorType(input.BufferType);

            const vk::DescriptorSetLayoutBinding binding = vk::DescriptorSetLayoutBinding
            (
                (uint32_t)input.RealSlot,
                type,
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
            const vk::DescriptorType type = GetDescriptorType(input.BufferType);

            const vk::DescriptorSetLayoutBinding binding = vk::DescriptorSetLayoutBinding
            (
                (uint32_t)input.RealSlot,
                type,
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

static vk::DescriptorImageInfo GetDescriptorImageInfo
(
    const TextureSamplerBuffer& a_baseSampler,
    const VulkanTextureSampler* a_sampler,
    VulkanGraphicsEngine* a_engine
)
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

static void GeneratePushBindings
(
    const vk::Device a_device,
    const Array<Input>& a_bindings,
    const Array<VulkanShaderInput>& a_inputs,
    vk::DescriptorSetLayout** a_layouts,
    VulkanPushDescriptor** a_pushDescriptors,
    uint32_t* a_pushDesciptorCount,
    Allocator* a_allocator
)
{
    IVERIFY(a_pushDesciptorCount != nullptr);
    IVERIFY(a_pushDescriptors != nullptr);
    IVERIFY(a_layouts != nullptr);

    *a_pushDesciptorCount = a_bindings.Size();
    *a_pushDescriptors = a_allocator->TAllocate<VulkanPushDescriptor>(*a_pushDesciptorCount);
    *a_layouts = a_allocator->TAllocate<vk::DescriptorSetLayout>(*a_pushDesciptorCount);

    for (uint32_t i = 0; i < *a_pushDesciptorCount; ++i)
    {
        const vk::DescriptorSetLayoutCreateInfo descriptorLayoutInfo = vk::DescriptorSetLayoutCreateInfo
        (
            { },
            1,
            &a_bindings[i].Binding
        );

        const VulkanShaderInput& input = a_inputs[a_bindings[i].Slot];

        vk::DescriptorSetLayout layout;
        VKRESERRMSG(a_device.createDescriptorSetLayout
        (
            &descriptorLayoutInfo,
            nullptr,
            &layout
        ), "Failed to create Push Descriptor Layout");
        (*a_layouts)[i] = layout;

        const VulkanPushDescriptor d =
        {
            .DescriptorLayout = layout,
            .Count = input.Count,
            .RealSlot = input.RealSlot,
            .UserSlot = input.UserSlot,
        };

        (*a_pushDescriptors)[i] = d;
    }
}

static void PushVulkanShaderBufferInput(Array<VulkanShaderInput>* a_inputs, const ShaderBufferInput& a_bufferInput, vk::ShaderStageFlags a_stage)
{
    for (VulkanShaderInput& i : *a_inputs)
    {
        if (i.RealSlot == a_bufferInput.RealSlot)
        {
            if (i.BufferType != a_bufferInput.BufferType)
            {
                IERROR("Vulkan Shader type mixmatch at " + COWU8String::FromValue(a_bufferInput.UserSlot, 10, MallocAllocator::Instance));
            }

            i.StageFlags |= a_stage;

            return;
        }
    }

    const VulkanShaderInput vInput = 
    {
        .StageFlags = a_stage,
        .BufferType = a_bufferInput.BufferType,
        .Count = a_bufferInput.Count,
        .RealSlot = a_bufferInput.RealSlot,
        .UserSlot = a_bufferInput.UserSlot,
    };

    a_inputs->Push(vInput);
}

VulkanShaderData::VulkanShaderData(Allocator* a_allocator) :
    m_textures(a_allocator)
{
    m_allocator = a_allocator;
}
VulkanShaderData::~VulkanShaderData()
{
    TRACE("Queueing Shader Data for deletion");
    m_engine->PushDeletionObject<VulkanShaderDataDeletionObject>(m_engine, m_pushDescriptors, m_pushDesciptorCount, m_layout);

    if (m_pushDescriptors != nullptr)
    {
        m_allocator->Destroy(m_pushDescriptors);
    }

    if (m_userUniformBuffer != nullptr)
    {
        m_allocator->Destroy(m_userUniformBuffer);
    }

    if (m_userArray != nullptr)
    {
        m_allocator->Destroy(m_userArray);
    }

    for (uint32_t i = 0; i < m_shaderCount; ++i)
    {
        m_allocator->Destroy(m_shaders[i]);
    }
    m_allocator->Free(m_shaders);

    m_allocator->Free(m_slotInputs);

    if (m_attributes != nullptr)
    {
        m_allocator->Free(m_attributes);
    }
}

void VulkanShaderData::CreateBaseShaderData
(
    VulkanShaderData* a_data,
    const VulkanBaseShaderDataBuilder& a_builder,
    Allocator* a_allocator,
    Allocator* a_tempAllocator
)
{
    IVERIFY(a_builder.Engine != nullptr);
    IVERIFY(a_builder.GraphicsEngine != nullptr);

    TRACE("Creating Shader Data");
    const vk::Device device = a_builder.Engine->GetLogicalDevice();

    Array<VulkanShader*> vulkanShaders = Array<VulkanShader*>(a_tempAllocator);
    Array<VulkanShaderInput> vulkanInputs = Array<VulkanShaderInput>(a_tempAllocator);
    Array<FlareShader::MeshShaderOut> vulkanOutputs = Array<FlareShader::MeshShaderOut>(a_tempAllocator);
    switch (a_builder.Program.MaterialMode)
    {
    case MaterialMode_BaseVertex:
    {
        IVERIFY(a_builder.Program.VertexShader != uint32_t(-1));

        Array<ShaderBufferInput> otherInputs = Array<ShaderBufferInput>(a_tempAllocator);

        const VulkanShaderInfo vertexInfo = a_builder.GraphicsEngine->GetVertexShaderInfo(a_builder.Program.VertexShader);
        IVERIFY(vertexInfo.Type == VulkanShaderInfoType_Flare);
        IVERIFY(!vertexInfo.Data.Empty());

        const COWU8String entryPoint = COWU8String("main", a_tempAllocator);

        const Dictionary<COWU8String, COWU8String> imports = a_builder.GraphicsEngine->GetVertexShaderImports();

        const VulkanVertexFShaderBuilder vertexBuilder =
        {
            .Engine = a_builder.Engine,
            .String = vertexInfo.Data,
            .Imports = imports,
            .EntryPoint = entryPoint,
            .OtherInputs = otherInputs,
        };

        VulkanVertexShader* vertexShader = a_allocator->TAllocate<VulkanVertexShader>();
        VulkanVertexShader::CreateFromFShader(vertexShader, vertexBuilder, a_allocator, a_tempAllocator);
        vulkanShaders.Push(vertexShader);

        const uint32_t inputCount = vertexShader->GetShaderInputCount();
        for (uint32_t i = 0; i < inputCount; ++i)
        {
            const ShaderBufferInput input = vertexShader->GetShaderInput(i);

            for (const ShaderBufferInput& other : otherInputs)
            {
                if (input.RealSlot != other.RealSlot)
                {
                    continue;
                }

                if (input.BufferType != other.BufferType)
                {
                    continue;
                }

                goto NextBaseVertexVertexInputFound;
            }

            otherInputs.Push(input);

NextBaseVertexVertexInputFound:;

            PushVulkanShaderBufferInput(&vulkanInputs, input, vk::ShaderStageFlagBits::eVertex);
        }

        if (a_builder.Program.PixelShader != uint32_t(-1))
        {
            const VulkanShaderInfo pixelInfo = a_builder.GraphicsEngine->GetPixelShaderInfo(a_builder.Program.PixelShader);
            IVERIFY(pixelInfo.Type == VulkanShaderInfoType_Flare);
            IVERIFY(!pixelInfo.Data.Empty());

            const Dictionary<COWU8String, COWU8String> imports = a_builder.GraphicsEngine->GetPixelShaderImports();

            const VulkanPixelFShaderBuilder pixelBuilder =
            {
                .Engine = a_builder.Engine,
                .String = pixelInfo.Data,
                .Imports = imports,
                .EntryPoint = entryPoint,
                .OtherInputs = otherInputs,
            };

            VulkanPixelShader* pixelShader = a_allocator->TAllocate<VulkanPixelShader>();
            VulkanPixelShader::CreateFromFShader(pixelShader, pixelBuilder, a_allocator, a_tempAllocator);
            vulkanShaders.Push(pixelShader);

            const uint32_t inputCount = pixelShader->GetShaderInputCount();
            for (uint32_t i = 0; i < inputCount; ++i)
            {
                const ShaderBufferInput input = pixelShader->GetShaderInput(i);

                for (const ShaderBufferInput& other : otherInputs)
                {
                    if (input.RealSlot != other.RealSlot)
                    {
                        continue;
                    }

                    if (input.BufferType != other.BufferType)
                    {
                        continue;
                    }

                    goto NextBaseVertexPixelInputFound;
                }

                otherInputs.Push(input);

NextBaseVertexPixelInputFound:;

                PushVulkanShaderBufferInput(&vulkanInputs, input, vk::ShaderStageFlagBits::eFragment);
            }
        }

        break;
    }
    case MaterialMode_BaseMesh:
    {
        IVERIFY(a_builder.Program.VertexShader != uint32_t(-1));

        const bool isMeshEnabled = a_builder.Engine->IsMeshEnabled();

        const COWU8String entryPoint = COWU8String("main", a_tempAllocator);

        const VulkanShaderInfo meshInfo = a_builder.GraphicsEngine->GetMeshShaderInfo(a_builder.Program.VertexShader);
        IVERIFY(meshInfo.Type == VulkanShaderInfoType_Flare);
        IVERIFY(!meshInfo.Data.Empty());

        Array<ShaderBufferInput> otherInputs = Array<ShaderBufferInput>(a_tempAllocator);
        if (isMeshEnabled)
        {
            const Dictionary<COWU8String, COWU8String> imports = a_builder.GraphicsEngine->GetMeshShaderImports();

            if (a_builder.Program.ExtraShader != uint32_t(-1))
            {
                const VulkanShaderInfo taskInfo = a_builder.GraphicsEngine->GetTaskShaderInfo(a_builder.Program.ExtraShader);
                IVERIFY(taskInfo.Type == VulkanShaderInfoType_Flare);
                IVERIFY(!taskInfo.Data.Empty());

                const VulkanTaskFShaderBuilder taskBuilder = 
                {
                    .Engine = a_builder.Engine,
                    .String = taskInfo.Data,
                    .Imports = imports,
                    .EntryPoint = entryPoint,
                    .OtherInputs = otherInputs,
                };

                VulkanTaskShader* taskShader = a_allocator->TAllocate<VulkanTaskShader>();
                VulkanTaskShader::CreateFromFShader(taskShader, taskBuilder, a_allocator, a_tempAllocator);
                vulkanShaders.Push(taskShader);

                const uint32_t inputCount = taskShader->GetShaderInputCount();
                for (uint32_t i = 0; i < inputCount; ++i)
                {
                    const ShaderBufferInput input = taskShader->GetShaderInput(i);

                    for (const ShaderBufferInput& other : otherInputs)
                    {
                        if (input.RealSlot != other.RealSlot)
                        {
                            continue;
                        }

                        if (input.BufferType != other.BufferType)
                        {
                            continue;
                        }

                        goto NextBaseMeshTaskInputFound;
                    }

                    otherInputs.Push(input);

NextBaseMeshTaskInputFound:;

                    PushVulkanShaderBufferInput(&vulkanInputs, input, vk::ShaderStageFlagBits::eTaskEXT);
                }
            }

            const VulkanMeshFShaderBuilder meshBuilder =
            {
                .Engine = a_builder.Engine,
                .String = meshInfo.Data,
                .Imports = imports,
                .EntryPoint = entryPoint,
                .OtherInputs = otherInputs,
            };

            VulkanMeshShader* meshShader = a_allocator->TAllocate<VulkanMeshShader>();
            VulkanMeshShader::CreateFromFShader(meshShader, meshBuilder, a_allocator, a_tempAllocator);
            vulkanShaders.Push(meshShader);

            const uint32_t inputCount = meshShader->GetShaderInputCount();
            for (uint32_t i = 0; i < inputCount; ++i)
            {
                const ShaderBufferInput input = meshShader->GetShaderInput(i);

                for (const ShaderBufferInput& other : otherInputs)
                {
                    if (input.RealSlot != other.RealSlot)
                    {
                        continue;
                    }

                    if (input.BufferType != other.BufferType)
                    {
                        continue;
                    }

                    goto NextBaseMeshMeshInputFound;
                }

                otherInputs.Push(input);

NextBaseMeshMeshInputFound:;

                PushVulkanShaderBufferInput(&vulkanInputs, input, vk::ShaderStageFlagBits::eMeshEXT);
            }
        }
        else
        {
            // Emulating so we need to use a stub Vertex Shader to passthrough info from a Compute Shader rather then a Mesh Shader
            // Take a massive performance hit but if the hardware does not support Mesh Shaders we do not have much choice
            const COWU8String shaderStr = FlareShader::GenerateMeshVertexStub(meshInfo.Data, &vulkanOutputs, a_allocator, a_tempAllocator);

            const VulkanVertexFShaderBuilder builder =
            {
                .Engine = a_builder.Engine,
                .String = shaderStr,
                .Imports = Dictionary<COWU8String, COWU8String>(a_tempAllocator),
                .EntryPoint = entryPoint,
                .OtherInputs = otherInputs,
            };

            VulkanVertexShader* vertexShader = a_allocator->TAllocate<VulkanVertexShader>();
            VulkanVertexShader::CreateFromFShader(vertexShader, builder, a_allocator, a_tempAllocator);
            vulkanShaders.Push(vertexShader);

            const uint32_t inputCount = vertexShader->GetShaderInputCount();
            for (uint32_t i = 0; i < inputCount; ++i)
            {
                const ShaderBufferInput input = vertexShader->GetShaderInput(i);

                for (const ShaderBufferInput& other : otherInputs)
                {
                    if (input.RealSlot != other.RealSlot)
                    {
                        continue;
                    }

                    if (input.BufferType != other.BufferType)
                    {
                        continue;
                    }

                    goto NextBaseMeshVertexInputFound;
                }

                otherInputs.Push(input);

NextBaseMeshVertexInputFound:;

                PushVulkanShaderBufferInput(&vulkanInputs, input, vk::ShaderStageFlagBits::eVertex);
            }
        }

        if (a_builder.Program.PixelShader != uint32_t(-1))
        {
            const VulkanShaderInfo info = a_builder.GraphicsEngine->GetPixelShaderInfo(a_builder.Program.PixelShader);
            IVERIFY(info.Type == VulkanShaderInfoType_Flare);
            IVERIFY(!info.Data.Empty());

            const Dictionary<COWU8String, COWU8String> imports = a_builder.GraphicsEngine->GetPixelShaderImports();

            const VulkanPixelFShaderBuilder builder =
            {
                .Engine = a_builder.Engine,
                .String = info.Data,
                .Imports = imports,
                .EntryPoint = entryPoint,
                .OtherInputs = otherInputs,
            };

            VulkanPixelShader* pixelShader = a_allocator->TAllocate<VulkanPixelShader>();
            VulkanPixelShader::CreateFromFShader(pixelShader, builder, a_allocator, a_tempAllocator);
            vulkanShaders.Push(pixelShader);

            const uint32_t inputCount = pixelShader->GetShaderInputCount();
            for (uint32_t i = 0; i < inputCount; ++i)
            {
                const ShaderBufferInput input = pixelShader->GetShaderInput(i);

                for (const ShaderBufferInput& other : otherInputs)
                {
                    if (input.RealSlot != other.RealSlot)
                    {
                        continue;
                    }

                    if (input.BufferType != other.BufferType)
                    {
                        continue;
                    }

                    goto NextBaseMeshPixelInputFound;
                }

                otherInputs.Push(input);

NextBaseMeshPixelInputFound:;

                PushVulkanShaderBufferInput(&vulkanInputs, input, vk::ShaderStageFlagBits::eFragment);
            }
        }

        break;
    }
    case MaterialMode_Compute:
    {
        IVERIFY(a_builder.Program.ExtraShader != uint32_t(-1));

        const VulkanShaderInfo info = a_builder.GraphicsEngine->GetComputeShaderInfo(a_builder.Program.ExtraShader);
        IVERIFY(info.Type == VulkanShaderInfoType_Flare);
        IVERIFY(!info.Data.Empty());

        const COWU8String entryPoint = COWU8String("main", a_tempAllocator);

        const Dictionary<COWU8String, COWU8String> imports = a_builder.GraphicsEngine->GetComputeShaderImports();

        const VulkanComputeFShaderBuilder builder =
        {
            .Engine = a_builder.Engine,
            .String = info.Data,
            .Imports = imports,
            .EntryPoint = entryPoint,
        };

        VulkanComputeShader* computeShader = a_allocator->TAllocate<VulkanComputeShader>();
        VulkanComputeShader::CreateFromFShader(computeShader, builder, a_allocator, a_tempAllocator);
        vulkanShaders.Push(computeShader);

        const uint32_t inputCount = computeShader->GetShaderInputCount();
        for (uint32_t i = 0; i < inputCount; ++i)
        {
            const ShaderBufferInput input = computeShader->GetShaderInput(i);

            PushVulkanShaderBufferInput(&vulkanInputs, input, vk::ShaderStageFlagBits::eCompute);
        }

        break;
    }
    default:
    {
        IERROR("Invalid material mode");

        break;
    }
    }

    const uint32_t slotInputCount = vulkanInputs.Size();
    VulkanShaderInput* slotInputs = ILAMBDA(
    {
        VulkanShaderInput* vals = a_allocator->TAllocate<VulkanShaderInput>(slotInputCount);

        for (uint32_t i = 0; i < slotInputCount; ++i)
        {
            vals[i] = vulkanInputs[i];
        }

        ILRETURN vals;
    });

    const uint32_t shaderCount = vulkanShaders.Size();
    VulkanShader** shaders = ILAMBDA(
    {
        VulkanShader** vals = a_allocator->TAllocate<VulkanShader*>(shaderCount);

        for (uint32_t i = 0; i < shaderCount; ++i)
        {
            vals[i] = vulkanShaders[i];
        }

        ILRETURN vals;
    });

    Array<vk::PushConstantRange> pushConstants = Array<vk::PushConstantRange>(a_tempAllocator);
    Array<Input> pushBindings = Array<Input>(a_tempAllocator);
    GetLayoutInfo(vulkanInputs, &pushConstants, &pushBindings);

    uint32_t pushDescriptorCount;
    VulkanPushDescriptor* pushDesciptors;
    vk::DescriptorSetLayout* layouts;
    GeneratePushBindings
    (
        device,
        pushBindings,
        vulkanInputs,
        &layouts,
        &pushDesciptors,
        &pushDescriptorCount,
        a_allocator
    );
    IDEFER(
    {
        if (layouts != nullptr)
        {
            a_allocator->Free(layouts);
        }
    });

    VulkanUniformBuffer* ubo = ILAMBDA(
    {
        if (a_builder.Program.UBOData != NULL && a_builder.Program.UBODataSize > 0)
        {
            ILRETURN a_allocator->Create<VulkanUniformBuffer>(a_builder.Engine, a_builder.Program.UBODataSize);
        }

        ILRETURN (VulkanUniformBuffer*)nullptr;
    });

    const uint32_t attributeCount = vulkanOutputs.Size();
    VertexInputAttribute* attibutes = ILAMBDA(
    {
        VertexInputAttribute* vals = a_allocator->TAllocate<VertexInputAttribute>(attributeCount + 1);

        vals[0].Count = 4;
        vals[0].Location = 0;
        vals[0].Offset = 0;
        vals[0].Type = VertexType_Float;

        for (uint16_t i = 0; i < attributeCount; ++i)
        {
            const uint32_t offsetIndex = i + 1;

            switch (vulkanOutputs[i].Type)
            {
            case FlareShader::MeshOutType_int:
            {
                vals[offsetIndex].Count = 1;
                vals[offsetIndex].Location = offsetIndex;
                vals[offsetIndex].Offset = offsetIndex * 16;
                vals[offsetIndex].Type = VertexType_Int;

                break;
            }
            case FlareShader::MeshOutType_uint:
            {
                vals[offsetIndex].Count = 1;
                vals[offsetIndex].Location = offsetIndex;
                vals[offsetIndex].Offset = offsetIndex * 16;
                vals[offsetIndex].Type = VertexType_UInt;

                break;
            }
            case FlareShader::MeshOutType_float:
            {
                vals[offsetIndex].Count = 1;
                vals[offsetIndex].Location = offsetIndex;
                vals[offsetIndex].Offset = offsetIndex * 16;
                vals[offsetIndex].Type = VertexType_Float;

                break;
            }
            case FlareShader::MeshOutType_vec2:
            {
                vals[offsetIndex].Count = 2;
                vals[offsetIndex].Location = offsetIndex;
                vals[offsetIndex].Offset = offsetIndex * 16;
                vals[offsetIndex].Type = VertexType_Float;

                break;
            }
            case FlareShader::MeshOutType_vec3:
            {
                vals[offsetIndex].Count = 3;
                vals[offsetIndex].Location = offsetIndex;
                vals[offsetIndex].Offset = offsetIndex * 16;
                vals[offsetIndex].Type = VertexType_Float;

                break;
            }
            case FlareShader::MeshOutType_vec4:
            {
                vals[offsetIndex].Count = 4;
                vals[offsetIndex].Location = offsetIndex;
                vals[offsetIndex].Offset = offsetIndex;
                vals[offsetIndex].Type = VertexType_Float;

                break;
            }
            default:
            {
                IERROR("Invalid meshout type");

                break;
            }
            }
        }

        ILRETURN vals;
    });

    TRACE("Creating Pipeline Layout");
    const vk::PipelineLayoutCreateInfo pipelineLayoutInfo = vk::PipelineLayoutCreateInfo
    (
        { },
        pushDescriptorCount,
        layouts,
        pushConstants.Size(),
        pushConstants.Data()
    );

    vk::PipelineLayout layout;
    VKRESERRMSG(device.createPipelineLayout
    (
        &pipelineLayoutInfo,
        nullptr,
        &layout
    ), "Failed to create PipelineLayout");

    new (a_data) VulkanShaderData(a_allocator);

    a_data->m_engine = a_builder.Engine;
    a_data->m_gEngine = a_builder.GraphicsEngine;

    a_data->m_userUniformBuffer = ubo;
    a_data->m_userArray = nullptr;
    a_data->m_slotInputs = slotInputs;
    a_data->m_slotInputCount = slotInputCount;
    a_data->m_shaders = shaders;
    a_data->m_shaderCount = shaderCount;
    a_data->m_pushDescriptors = pushDesciptors;
    a_data->m_pushDesciptorCount = pushDescriptorCount;
    a_data->m_attributes = attibutes;
    a_data->m_attributeCount = attributeCount + 1;

    a_data->m_layout = layout;

    a_data->m_materialMode = a_builder.Program.MaterialMode;
}
void VulkanShaderData::CreateComputeMeshShaderData
(
    VulkanShaderData* a_data,
    const VulkanComputeMeshShaderDataBuilder& a_builder,
    Allocator* a_allocator,
    Allocator* a_tempAllocator
)
{
    IVERIFY(a_builder.Engine != nullptr);
    IVERIFY(a_builder.GraphicsEngine != nullptr);
    IVERIFY(!a_builder.Engine->IsMeshEnabled());

    TRACE("Creating Mesh Compute Shader Data");
    const vk::Device device = a_builder.Engine->GetLogicalDevice();

    IVERIFY(a_builder.Program.VertexShader != uint32_t(-1));

    const VulkanShaderInfo info = a_builder.GraphicsEngine->GetMeshShaderInfo(a_builder.Program.VertexShader);
    IVERIFY(info.Type == VulkanShaderInfoType_Flare);
    IVERIFY(!info.Data.Empty());

    const COWU8String entryPoint = COWU8String("main", a_tempAllocator);

    const Dictionary<COWU8String, COWU8String> imports = a_builder.GraphicsEngine->GetMeshShaderImports();

    const VulkanComputeFShaderBuilder builder =
    {
        .Engine = a_builder.Engine,
        .String = info.Data,
        .Imports = imports,
        .EntryPoint = entryPoint,
    };

    VulkanComputeShader* computeShader = a_allocator->TAllocate<VulkanComputeShader>();
    VulkanComputeShader::CreateFromFShader(computeShader, builder, a_allocator, a_tempAllocator);

    Array<VulkanShaderInput> vulkanInputs = Array<VulkanShaderInput>(a_tempAllocator);
    const uint32_t inputCount = computeShader->GetShaderInputCount();
    for (uint32_t i = 0; i < inputCount; ++i)
    {
        const ShaderBufferInput input = computeShader->GetShaderInput(i);

        PushVulkanShaderBufferInput(&vulkanInputs, input, vk::ShaderStageFlagBits::eCompute);
    }

    const uint32_t slotInputCount = vulkanInputs.Size();
    VulkanShaderInput* slotInputs = a_allocator->TAllocate<VulkanShaderInput>(slotInputCount);
    for (uint32_t i = 0; i < slotInputCount; ++i)
    {
        slotInputs[i] = vulkanInputs[i];
    }

    Array<vk::PushConstantRange> pushConstants = Array<vk::PushConstantRange>(a_tempAllocator);
    Array<Input> pushBindings = Array<Input>(a_tempAllocator);
    GetLayoutInfo(vulkanInputs, &pushConstants, &pushBindings);

    uint32_t pushDescriptorCount;
    VulkanPushDescriptor* pushDesciptors;
    vk::DescriptorSetLayout* layouts;
    GeneratePushBindings
    (
        device,
        pushBindings,
        vulkanInputs,
        &layouts,
        &pushDesciptors,
        &pushDescriptorCount,
        a_allocator
    );
    IDEFER(
    {
        if (layouts != nullptr)
        {
            a_allocator->Free(layouts);
        }
    });

    const vk::PipelineLayoutCreateInfo pipelineLayoutInfo = vk::PipelineLayoutCreateInfo
    (
        { },
        pushDescriptorCount,
        layouts,
        pushConstants.Size(),
        pushConstants.Data()
    );

    VulkanUniformBuffer* ubo = ILAMBDA(
    {
        if (a_builder.Program.UBOData != NULL && a_builder.Program.UBODataSize > 0)
        {
            ILRETURN a_allocator->Create<VulkanUniformBuffer>(a_builder.Engine, a_builder.Program.UBODataSize);
        }

        ILRETURN (VulkanUniformBuffer*)nullptr;
    });

    VulkanShader** shaders = ILAMBDA(
    {
        VulkanShader** vals = a_allocator->TAllocate<VulkanShader*>(1);
        vals[0] = computeShader;

        ILRETURN vals;
    });

    TRACE("Creating Mesh Compute Pipeline Layout");
    vk::PipelineLayout layout;
    VKRESERRMSG(device.createPipelineLayout
    (
        &pipelineLayoutInfo,
        nullptr,
        &layout
    ), "Failed to create Mesh Compute PipelineLayout");

    new (a_data) VulkanShaderData(a_allocator);

    a_data->m_engine = a_builder.Engine;
    a_data->m_gEngine = a_builder.GraphicsEngine;

    a_data->m_userUniformBuffer = ubo;
    a_data->m_userArray = nullptr;
    a_data->m_slotInputs = slotInputs;
    a_data->m_slotInputCount = slotInputCount;
    a_data->m_shaders = shaders;
    a_data->m_shaderCount = 1;
    a_data->m_pushDescriptors = pushDesciptors;
    a_data->m_pushDesciptorCount = pushDescriptorCount;
    a_data->m_attributes = nullptr;
    a_data->m_attributeCount = 0;

    a_data->m_layout = layout;

    a_data->m_materialMode = MaterialMode_Compute;
}
void VulkanShaderData::CreateShadowShaderData
(
    VulkanShaderData* a_data,
    const VulkanShadowShaderDataBuilder& a_builder,
    Allocator* a_allocator,
    Allocator* a_tempAllocator
)
{
    IVERIFY(a_builder.Engine != nullptr);
    IVERIFY(a_builder.GraphicsEngine != nullptr);

    TRACE("Creating Shadow Shader Data");
    const vk::Device device = a_builder.Engine->GetLogicalDevice();

    IVERIFY(a_builder.Program.ShadowVertexShader != uint32_t(-1));

    // TODO: Add a mesh variant of the Shadow Shader
    const VulkanShaderInfo info = a_builder.GraphicsEngine->GetVertexShaderInfo(a_builder.Program.ShadowVertexShader);
    IVERIFY(info.Type == VulkanShaderInfoType_Flare);
    IVERIFY(!info.Data.Empty());

    const COWU8String entry = COWU8String("main", a_tempAllocator);

    const Dictionary<COWU8String, COWU8String> imports = a_builder.GraphicsEngine->GetVertexShaderImports();

    const VulkanVertexFShaderBuilder builder =
    {
        .Engine = a_builder.Engine,
        .String = info.Data,
        .Imports = imports,
        .EntryPoint = entry,
        .OtherInputs = Array<ShaderBufferInput>(a_tempAllocator),
    };

    VulkanVertexShader* vertexShader = a_allocator->TAllocate<VulkanVertexShader>();
    VulkanVertexShader::CreateFromFShader(vertexShader, builder, a_allocator, a_tempAllocator);

    Array<VulkanShaderInput> vulkanInputs = Array<VulkanShaderInput>(a_tempAllocator);

    const uint32_t inputCount = vertexShader->GetShaderInputCount();
    for (uint32_t i = 0; i < inputCount; ++i)
    {
        const ShaderBufferInput input = vertexShader->GetShaderInput(i);

        PushVulkanShaderBufferInput(&vulkanInputs, input, vk::ShaderStageFlagBits::eVertex);
    }

    Array<vk::PushConstantRange> pushConstants = Array<vk::PushConstantRange>(a_tempAllocator);
    Array<Input> pushBindings = Array<Input>(a_tempAllocator);
    GetLayoutInfo(vulkanInputs, &pushConstants, &pushBindings);

    uint32_t pushDescriptorCount;
    VulkanPushDescriptor* pushDesciptors;
    vk::DescriptorSetLayout* layouts;
    GeneratePushBindings
    (
        device,
        pushBindings,
        vulkanInputs,
        &layouts,
        &pushDesciptors,
        &pushDescriptorCount,
        a_allocator
    );
    IDEFER(
    {
        if (layouts != nullptr)
        {
            a_allocator->Free(layouts);
        }
    });

    const uint32_t slotInputCount = vulkanInputs.Size();
    VulkanShaderInput* slotInputs = a_allocator->TAllocate<VulkanShaderInput>(slotInputCount);
    for (uint32_t i = 0; i < slotInputCount; ++i)
    {
        slotInputs[i] = vulkanInputs[i];
    }

    const vk::PipelineLayoutCreateInfo pipelineLayoutInfo = vk::PipelineLayoutCreateInfo
    (
        { },
        pushDescriptorCount,
        layouts,
        pushConstants.Size(),
        pushConstants.Data()
    );

    TRACE("Creating Shadow Pipeline Layout");
    vk::PipelineLayout layout;
    VKRESERRMSG(device.createPipelineLayout
    (
        &pipelineLayoutInfo,
        nullptr,
        &layout
    ), "Failed to create Shadow PipelineLayout");

    VulkanUniformBuffer* ubo = ILAMBDA(
    {
        if (a_builder.Program.UBOData != NULL && a_builder.Program.UBODataSize > 0)
        {
            ILRETURN a_allocator->Create<VulkanUniformBuffer>(a_builder.Engine, a_builder.Program.UBODataSize);
        }

        ILRETURN (VulkanUniformBuffer*)nullptr;
    });

    VulkanShader** shaders = ILAMBDA(
    {
        VulkanShader** vals = a_allocator->TAllocate<VulkanShader*>(1);
        vals[0] = vertexShader;

        ILRETURN vals;
    });

    new (a_data) VulkanShaderData(a_allocator);

    a_data->m_engine = a_builder.Engine;
    a_data->m_gEngine = a_builder.GraphicsEngine;

    a_data->m_userUniformBuffer = ubo;
    a_data->m_userArray = nullptr;
    a_data->m_slotInputs = slotInputs;
    a_data->m_slotInputCount = slotInputCount;
    a_data->m_pushDescriptors = pushDesciptors;
    a_data->m_pushDesciptorCount = pushDescriptorCount;
    a_data->m_shaders = shaders;
    a_data->m_shaderCount = 1;
    a_data->m_attributes = nullptr;
    a_data->m_attributeCount = 0;

    a_data->m_layout = layout;

    a_data->m_materialMode = a_builder.Program.MaterialMode;
}

uint16_t VulkanShaderData::UserSlotToReal(uint16_t a_userSlot) const
{
    for (uint32_t i = 0; i < m_slotInputCount; ++i)
    {
        if (m_slotInputs[i].UserSlot == a_userSlot)
        {
            return m_slotInputs[i].RealSlot;
        }
    }

    return uint16_t(-1);
}

void VulkanShaderData::SetTexture(uint16_t a_slot, uint32_t a_sampleAddr)
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

bool VulkanShaderData::PushComputeBufferTexture(vk::CommandBuffer a_commandBuffer, uint16_t a_slot, const VulkanRenderTexture* a_renderTexture, uint32_t a_index) const
{
    for (uint32_t i = 0; i < m_pushDesciptorCount; ++i)
    {
        const VulkanPushDescriptor& d = m_pushDescriptors[i];
        if (d.RealSlot != a_slot)
        {
            continue;
        }

        const vk::Device device = m_engine->GetLogicalDevice();
        VulkanPushPool* pool = m_engine->GetPushPool();
        IVERIFY(pool != nullptr);

        vk::DescriptorSet descriptorSet = pool->AllocateDescriptor(a_index, vk::DescriptorType::eStorageImage, &d.DescriptorLayout);

        const vk::DescriptorImageInfo imageInfo = vk::DescriptorImageInfo
        (
            nullptr,
            a_renderTexture->GetImageView(d.Count),
            vk::ImageLayout::eGeneral
        );


        const vk::WriteDescriptorSet descriptorWrite = vk::WriteDescriptorSet
        (
            descriptorSet,
            (uint32_t)d.RealSlot,
            0,
            1,
            vk::DescriptorType::eStorageImage,
            &imageInfo
        );

        device.updateDescriptorSets(1, &descriptorWrite, 0, nullptr);

        a_commandBuffer.bindDescriptorSets
        (
            vk::PipelineBindPoint::eCompute, 
            m_layout,
            (uint32_t)d.RealSlot,
            1,
            &descriptorSet,
            0,
            nullptr
        );

        return true;
    }

    return false;
}

bool VulkanShaderData::PushTexture(vk::CommandBuffer a_commandBuffer, uint16_t a_slot, const TextureSamplerBuffer& a_sampler, uint32_t a_index) const
{
    IVERIFY(a_sampler.Data != nullptr);

    for (uint32_t i = 0; i < m_pushDesciptorCount; ++i)
    {
        const VulkanPushDescriptor& d = m_pushDescriptors[i];
        if (d.RealSlot != a_slot)
        {
            continue;
        }

        const vk::Device device = m_engine->GetLogicalDevice();
        VulkanPushPool* pool = m_engine->GetPushPool();
        IVERIFY(pool != nullptr);

        const VulkanTextureSampler* vSampler = (VulkanTextureSampler*)a_sampler.Data;

        vk::DescriptorSet descriptorSet = pool->AllocateDescriptor(a_index, vk::DescriptorType::eCombinedImageSampler, &d.DescriptorLayout);

        const vk::DescriptorImageInfo imageInfo = GetDescriptorImageInfo(a_sampler, vSampler, m_gEngine);

        const vk::WriteDescriptorSet descriptorWrite = vk::WriteDescriptorSet
        (
            descriptorSet,
            (uint32_t)d.RealSlot,
            0,
            1,
            vk::DescriptorType::eCombinedImageSampler,
            &imageInfo
        );

        device.updateDescriptorSets(1, &descriptorWrite, 0, nullptr);

        const vk::PipelineBindPoint bindPoint = GetBindPoint(m_materialMode);
        a_commandBuffer.bindDescriptorSets(bindPoint, m_layout, (uint32_t)d.RealSlot, 1, &descriptorSet, 0, nullptr);

        return true;
    }

    return false;
}
bool VulkanShaderData::PushTextures
(
    vk::CommandBuffer a_commandBuffer,
    uint16_t a_slot,
    const TextureSamplerBuffer* a_samplers,
    uint16_t a_count,
    uint32_t a_index,
    Allocator* a_tempAllocator
) const
{
    for (uint32_t i = 0; i < m_pushDesciptorCount; ++i)
    {
        const VulkanPushDescriptor& d = m_pushDescriptors[i];
        if (d.RealSlot != a_slot)
        {
            continue;
        }

        if (a_count > d.Count)
        {
            return false;
        }

        const vk::Device device = m_engine->GetLogicalDevice();
        VulkanPushPool* pool = m_engine->GetPushPool();
        IVERIFY(pool != nullptr);

        vk::DescriptorSet descriptorSet = pool->AllocateDescriptor
        (
            a_index,
            vk::DescriptorType::eCombinedImageSampler,
            &d.DescriptorLayout,
            a_count
        );

        const vk::DescriptorImageInfo* imageInfos = ILAMBDA(
        {
            vk::DescriptorImageInfo* vals = a_tempAllocator->TAllocate<vk::DescriptorImageInfo>(a_count);

            for (uint32_t i = 0; i < a_count; ++i)
            {
                const VulkanTextureSampler* vSampler = (VulkanTextureSampler*)a_samplers[i].Data;
                IVERIFY(vSampler != nullptr);

                vals[i] = GetDescriptorImageInfo(a_samplers[i], vSampler, m_gEngine);
            }

            ILRETURN vals;
        });

        const vk::WriteDescriptorSet descriptorWrite = vk::WriteDescriptorSet
        (
            descriptorSet,
            (uint32_t)d.RealSlot,
            0,
            a_count,
            vk::DescriptorType::eCombinedImageSampler,
            imageInfos
        );

        device.updateDescriptorSets(1, &descriptorWrite, 0, nullptr);

        const vk::PipelineBindPoint bindPoint = GetBindPoint(m_materialMode);
        a_commandBuffer.bindDescriptorSets(bindPoint, m_layout, d.RealSlot, 1, &descriptorSet, 0, nullptr);

        return true;
    }

    return false;
}

bool VulkanShaderData::PushUniformBuffer(vk::CommandBuffer a_commandBuffer, uint16_t a_slot, const VulkanUniformBuffer* a_buffer, uint32_t a_index) const
{
    for (uint32_t i = 0; i < m_pushDesciptorCount; ++i)
    {
        const VulkanPushDescriptor& d = m_pushDescriptors[i];
        if (d.RealSlot != a_slot)
        {
            continue;
        }

        const vk::Device device = m_engine->GetLogicalDevice();
        VulkanPushPool* pool = m_engine->GetPushPool();
        IVERIFY(pool != nullptr);

        vk::DescriptorSet descriptorSet = pool->AllocateDescriptor(a_index, vk::DescriptorType::eUniformBuffer, &d.DescriptorLayout);

        const vk::Buffer buffer = a_buffer->GetBuffer(a_index);
        const vk::DescriptorBufferInfo bufferInfo = vk::DescriptorBufferInfo
        (
            buffer,
            0,
            vk::WholeSize
        );

        const vk::WriteDescriptorSet descriptorWrite = vk::WriteDescriptorSet
        (
            descriptorSet,
            (uint32_t)d.RealSlot,
            0,
            1,
            vk::DescriptorType::eUniformBuffer,
            nullptr,
            &bufferInfo
        );

        device.updateDescriptorSets(1, &descriptorWrite, 0, nullptr);

        const vk::PipelineBindPoint bindPoint = GetBindPoint(m_materialMode);
        a_commandBuffer.bindDescriptorSets(bindPoint, m_layout, (uint32_t)d.RealSlot, 1, &descriptorSet, 0, nullptr);

        return true;
    }

    return false;
}
bool VulkanShaderData::PushShaderStorageObject(vk::CommandBuffer a_commandBuffer, uint16_t a_slot, const VulkanShaderStorageObject* a_object, uint32_t a_index) const
{
    const vk::Buffer buffer = a_object->GetBuffer();

    return PushShaderStorageObject(a_commandBuffer, a_slot, buffer, 0, a_index);
}
bool VulkanShaderData::PushShaderStorageObject
(
    vk::CommandBuffer a_commandBuffer,
    uint16_t a_slot,
    vk::Buffer a_object,
    vk::DeviceSize a_offset,
    uint32_t a_index
) const
{
    for (uint32_t i = 0; i < m_pushDesciptorCount; ++i)
    {
        const VulkanPushDescriptor& d = m_pushDescriptors[i];
        if (d.RealSlot != a_slot)
        {
            continue;
        }

        const vk::Device device = m_engine->GetLogicalDevice();
        VulkanPushPool* pool = m_engine->GetPushPool();
        IVERIFY(pool != nullptr);

        vk::DescriptorSet descriptorSet = pool->AllocateDescriptor(a_index, vk::DescriptorType::eStorageBuffer, &d.DescriptorLayout);

        const vk::DescriptorBufferInfo bufferInfo = vk::DescriptorBufferInfo
        (
            a_object,
            a_offset,
            vk::WholeSize
        );

        const vk::WriteDescriptorSet descriptorWrite = vk::WriteDescriptorSet
        (
            descriptorSet,
            (uint32_t)d.RealSlot,
            0,
            1,
            vk::DescriptorType::eStorageBuffer,
            nullptr,
            &bufferInfo
        );

        device.updateDescriptorSets(1, &descriptorWrite, 0, nullptr);

        const vk::PipelineBindPoint bindPoint = GetBindPoint(m_materialMode);
        a_commandBuffer.bindDescriptorSets(bindPoint, m_layout, (uint32_t)d.RealSlot, 1, &descriptorSet, 0, nullptr);

        return true;
    }

    return false;
}

void VulkanShaderData::PushMeshBuffers(vk::CommandBuffer a_commandBuffer, const VulkanMesh* a_mesh, uint32_t a_index) const
{
    if (a_mesh == nullptr)
    {
        return;
    }

    const vk::Buffer buffer = a_mesh->GetBuffer();

    for (uint32_t i = 0; i < m_slotInputCount; ++i)
    {
        const VulkanShaderInput& input = m_slotInputs[i];
        switch (input.BufferType)
        {
        case ShaderBufferType_MeshVertex:
        {
            const vk::DeviceSize offset = a_mesh->GetVertexOffset();

            PushShaderStorageObject(a_commandBuffer, input.RealSlot, buffer, offset, a_index);

            break;
        }
        case ShaderBufferType_MeshletVertices:
        {
            const vk::DeviceSize offset = a_mesh->GetMeshletVertexOffset();

            PushShaderStorageObject(a_commandBuffer, input.RealSlot, buffer, offset, a_index);

            break;
        }
        case ShaderBufferType_MeshletTriangles:
        {
            const vk::DeviceSize offset = a_mesh->GetMeshletTriangleOffset();

            PushShaderStorageObject(a_commandBuffer, input.RealSlot, buffer, offset, a_index);

            break;
        }
        case ShaderBufferType_Meshlet:
        {
            const vk::DeviceSize offset = a_mesh->GetMeshletOffset();

            PushShaderStorageObject(a_commandBuffer, input.RealSlot, buffer, offset, a_index);

            break;
        }
        default:
        {
            break;
        }
        }
    }
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

        a_commandBuffer.pushConstants
        (
            m_layout,
            input.StageFlags,
            0,
            sizeof(IcarianCore::ShaderModelBuffer),
            &buffer);

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
    for (uint32_t i = 0; i < m_slotInputCount; ++i)
    {
        const VulkanShaderInput& input = m_slotInputs[i];
        if (input.BufferType != ShaderBufferType_PShadowLightBuffer)
        {
            continue;
        }

        const IcarianCore::ShaderShadowLightBuffer buffer =
        {
            .LVP = a_lvp,
            .Split = a_split
        };

        a_commandBuffer.pushConstants
        (
            m_layout,
            input.StageFlags,
            0,
            sizeof(IcarianCore::ShaderShadowLightBuffer),
            &buffer
        );

        return;
    }
}

void VulkanShaderData::Update(uint32_t a_index, const RenderProgram& a_program)
{
    // Want the user to be able to update the uniform buffer whenever they want
    // However updating mid render can cause issues
    if (m_userUniformBuffer != nullptr)
    {
        IVERIFY(a_program.UBOData != NULL);

        m_userUniformBuffer->SetData(a_index, a_program.UBOData);
    }

    if (m_userArray != nullptr)
    {
        m_allocator->Destroy(m_userArray);

        m_userArray = nullptr;
    }

    if (a_program.UserArrayData != NULL && a_program.UserArrayStride > 0)
    {
        const uint32_t size = a_program.UserArrayCount * a_program.UserArrayStride;

        m_userArray = m_allocator->Create<VulkanShaderStorageObject>(m_engine, size, a_program.UserArrayCount, a_program.UserArrayData);
    }
}

bool VulkanShaderData::Bind(uint32_t a_index, vk::CommandBuffer a_commandBuffer) const
{
    for (uint32_t i = 0; i < m_slotInputCount; ++i)
    {
        const VulkanShaderInput& input = m_slotInputs[i];

        switch (input.BufferType)
        {
        case ShaderBufferType_UserUBO:
        {
            if (m_userUniformBuffer == nullptr)
            {
                return false;
            }

            PushUniformBuffer(a_commandBuffer, input.RealSlot, m_userUniformBuffer, a_index);

            break;
        }
        case ShaderBufferType_UserArray:
        {
            if (m_userArray == nullptr)
            {
                const VulkanShaderStorageObject emptyArray = VulkanShaderStorageObject(m_engine, 0, 0, nullptr);
                PushShaderStorageObject(a_commandBuffer, input.RealSlot, &emptyArray, a_index);

                break;
            }

            PushShaderStorageObject(a_commandBuffer, input.RealSlot, m_userArray, a_index);

            break;
        }
        case ShaderBufferType_OutMeshEmulationDraw:
        {
            IVERIFY(!m_engine->IsMeshEnabled());

            const VulkanMeshEmulationData* data = m_gEngine->GetMeshEmulationData();
            IVERIFY(data != nullptr);

            PushShaderStorageObject(a_commandBuffer, input.RealSlot, data->DrawBuffer, 0, a_index);

            break;
        }
        case ShaderBufferType_OutMeshEmulationVertex:
        {
            IVERIFY(!m_engine->IsMeshEnabled());

            const VulkanMeshEmulationData* data = m_gEngine->GetMeshEmulationData();
            IVERIFY(data != nullptr);

            PushShaderStorageObject(a_commandBuffer, input.RealSlot, data->VertexBuffer, 0, a_index);

            break;
        }
        case ShaderBufferType_OutMeshEmulationIndex:
        {
            IVERIFY(!m_engine->IsMeshEnabled());

            const VulkanMeshEmulationData* data = m_gEngine->GetMeshEmulationData();
            IVERIFY(data != nullptr);

            PushShaderStorageObject(a_commandBuffer, input.RealSlot, data->IndexBuffer, 0, a_index);

            break;
        }
        case ShaderBufferType_TimeBuffer:
        {
            const VulkanUniformBuffer* timeUBO = m_gEngine->GetTimeUniformBuffer();

            PushUniformBuffer(a_commandBuffer, input.RealSlot, timeUBO, a_index);

            break;
        }
        default:
        {
            break;
        }
        }
    }

    // TODO: Handle texture transitions for compute textures
    for (const VulkanTextureBinding& b : m_textures)
    {
        const TextureSamplerBuffer sampler = m_gEngine->GetTextureSampler(b.SamplerAddr);

        PushTexture(a_commandBuffer, b.Slot, sampler, a_index);
    }

    return true;
}
void VulkanShaderData::Unbind(uint32_t a_index, vk::CommandBuffer a_commandBuffer) const
{
    // Will need but not this current second
}

VertexInputAttribute VulkanShaderData::GetAttribute(uint32_t a_index) const
{
    IVERIFY(a_index < m_attributeCount);

    return m_attributes[a_index];
}
VulkanShader* VulkanShaderData::GetShader(uint32_t a_index) const
{
    IVERIFY(a_index < m_shaderCount);

    return m_shaders[a_index];
}

Array<ShaderBufferInput> VulkanShaderData::GetShaderBufferInputs(e_ShaderBufferType a_type, Allocator* a_allocator) const
{
    Array<ShaderBufferInput> inputs = Array<ShaderBufferInput>(a_allocator);
    inputs.Reserve(m_slotInputCount);

    for (uint32_t i = 0; i < m_slotInputCount; ++i)
    {
        const VulkanShaderInput& input = m_slotInputs[i];
        if (input.BufferType != a_type)
        {
            continue;
        }

        const ShaderBufferInput sInput =
        {
            .UserSlot = input.UserSlot,
            .RealSlot = input.RealSlot,
            .BufferType = input.BufferType,
            .Count = input.Count,
        };

        inputs.Push(sInput);
    }

    return inputs;
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
            .UserSlot = input.UserSlot,
            .RealSlot = input.RealSlot,
            .BufferType = input.BufferType,
            .Count = input.Count
        };

        *a_input = sInput;

        return true;
    }

    return false;
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
