// Icarian Engine - C# Game Engine
//
// License at end of file.

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/VulkanPipeline.h"

#include "Core/IcarianLambda.h"
#include "Rendering/Vulkan/Shaders/VulkanComputeShader.h"
#include "Rendering/Vulkan/Shaders/VulkanMeshShader.h"
#include "Rendering/Vulkan/Shaders/VulkanPixelShader.h"
#include "Rendering/Vulkan/Shaders/VulkanVertexShader.h"
#include "Rendering/Vulkan/Shaders/VulkanTaskShader.h"
#include "Rendering/Vulkan/VulkanGraphicsEngine.h"
#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"
#include "Rendering/Vulkan/VulkanRenderProgramBlob.h"
#include "Rendering/Vulkan/VulkanShaderData.h"
#include "Trace.h"

#include "EngineMaterialInteropStructures.h"

static constexpr vk::DynamicState DynamicStates[] =
{
    vk::DynamicState::eViewport,
    vk::DynamicState::eScissor
};

static constexpr uint32_t DynamicStateCount = sizeof(DynamicStates) / sizeof(*DynamicStates);

static constexpr vk::DynamicState ShadowDynamicStates[] =
{
    vk::DynamicState::eViewport,
    vk::DynamicState::eScissor,
    vk::DynamicState::eDepthBias
};

static constexpr uint32_t ShadowDynamicStateCount = sizeof(ShadowDynamicStates) / sizeof(*ShadowDynamicStates);

static constexpr vk::Viewport Viewport = vk::Viewport(0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f);
static constexpr vk::Rect2D Scissor = vk::Rect2D({ 0, 0 }, { 1, 1 });

static constexpr vk::PipelineMultisampleStateCreateInfo Multisampling = vk::PipelineMultisampleStateCreateInfo
(
    { },
    vk::SampleCountFlagBits::e1
);

class VulkanPipelineDeletionObject : public VulkanDeletionObject
{
private:
    VulkanRenderEngineBackend* m_engine;

    vk::Pipeline               m_pipeline;

protected:

public:
    VulkanPipelineDeletionObject(VulkanRenderEngineBackend* a_engine, vk::Pipeline a_pipeline)
    {
        m_engine = a_engine;

        m_pipeline = a_pipeline;
    }
    virtual ~VulkanPipelineDeletionObject()
    {

    }

    virtual void Destroy() override
    {
        TRACE("Destroying Pipeline");
        const vk::Device device = m_engine->GetLogicalDevice();

        device.destroyPipeline(m_pipeline);
    }
};

static IcarianCore::Array<vk::PipelineShaderStageCreateInfo> GetStageInfo(const RenderProgram& a_program, bool a_meshEnabled, IcarianCore::Allocator* a_allocator)
{
    IcarianCore::Array<vk::PipelineShaderStageCreateInfo> stages = IcarianCore::Array<vk::PipelineShaderStageCreateInfo>(a_allocator);

    IVERIFY(a_program.Data != nullptr);
    const VulkanRenderProgramBlob* blob = (VulkanRenderProgramBlob*)a_program.Data;

    IVERIFY(blob->Base != nullptr);
    const VulkanShaderData* data = blob->Base;

    const uint32_t shaderCount = data->GetShaderCount();

    if (a_program.VertexShader != uint32_t(-1))
    {
        switch (a_program.MaterialMode)
        {
        case MaterialMode_BaseVertex:
        {
            const VulkanVertexShader* vertexShader = ILAMBDA(
            {
                for (uint32_t i = 0; i < shaderCount; ++i)
                {
                    const VulkanShader* shader = data->GetShader(i);
                    IVERIFY(shader != nullptr);

                    const e_VulkanShaderType type = shader->GetShaderType();
                    if (type == VulkanShaderType_Vertex)
                    {
                        ILRETURN (VulkanVertexShader*)shader;
                    }
                }

                ILRETURN (VulkanVertexShader*)nullptr;
            });
            IVERIFY(vertexShader != nullptr);

            const vk::ShaderModule module = vertexShader->GetShaderModule();

            stages.Push(vk::PipelineShaderStageCreateInfo
            (
                { },
                vk::ShaderStageFlagBits::eVertex,
                module,
                "main"
            ));

            break;
        }
        case MaterialMode_BaseMesh:
        {
            if (a_meshEnabled)
            {
                const VulkanMeshShader* meshShader = ILAMBDA(
                {
                    for (uint32_t i = 0; i < shaderCount; ++i)
                    {
                        const VulkanShader* shader = data->GetShader(i);
                        IVERIFY(shader != nullptr);

                        const e_VulkanShaderType type = shader->GetShaderType();
                        if (type == VulkanShaderType_Mesh)
                        {
                            ILRETURN (VulkanMeshShader*)shader;
                        }
                    }

                    ILRETURN (VulkanMeshShader*)nullptr;
                });
                IVERIFY(meshShader != nullptr);

                const vk::ShaderModule module = meshShader->GetShaderModule();

                stages.Push(vk::PipelineShaderStageCreateInfo
                (
                    { },
                    vk::ShaderStageFlagBits::eMeshEXT,
                    module,
                    "main"
                ));

                if (a_program.ExtraShader != uint32_t(-1))
                {
                    const VulkanTaskShader* taskShader = ILAMBDA(
                    {
                        for (uint32_t i = 0; i < shaderCount; ++i)
                        {
                            const VulkanShader* shader = data->GetShader(i);
                            IVERIFY(shader != nullptr);

                            const e_VulkanShaderType type = shader->GetShaderType();
                            if (type == VulkanShaderType_Task)
                            {
                                ILRETURN (VulkanTaskShader*)shader;
                            }
                        }

                        ILRETURN (VulkanTaskShader*)nullptr;
                    });
                    IVERIFY(taskShader != nullptr);

                    const vk::ShaderModule module = taskShader->GetShaderModule();

                    stages.Push(vk::PipelineShaderStageCreateInfo
                    (
                        { },
                        vk::ShaderStageFlagBits::eTaskEXT,
                        module,
                        "main"
                    ));
                }
            }
            else
            {
                // GPU does not support Mesh shaders or in emulation mode so need to use stub Vertex mode and generate a secondary and tertiary pipeline on Compute
                // const VulkanVertexShader* vertexShader = meshShader->GetVertexShader();
                const VulkanVertexShader* vertexShader = ILAMBDA(
                {
                    for (uint32_t i = 0; i < shaderCount; ++i)
                    {
                        const VulkanShader* shader = data->GetShader(i);
                        IVERIFY(shader != nullptr);

                        const e_VulkanShaderType type = shader->GetShaderType();
                        if (type == VulkanShaderType_Vertex)
                        {
                            ILRETURN (VulkanVertexShader*)shader;
                        }
                    }

                    ILRETURN (VulkanVertexShader*)nullptr;
                });
                IVERIFY(vertexShader != nullptr);

                const vk::ShaderModule module = vertexShader->GetShaderModule();

                stages.Push(vk::PipelineShaderStageCreateInfo
                (
                    { },
                    vk::ShaderStageFlagBits::eVertex,
                    module,
                    "main"
                ));
            }

            break;
        }
        case MaterialMode_Compute:
        {
            IVERIFY(shaderCount == 1);

            const VulkanShader* shader = data->GetShader(0);
            IVERIFY(shader != nullptr);
            IVERIFY(shader->GetShaderType() == VulkanShaderType_Compute);

            const vk::ShaderModule module = shader->GetShaderModule();

            stages.Push(vk::PipelineShaderStageCreateInfo
            (
                { },
                vk::ShaderStageFlagBits::eCompute,
                module,
                "main"
            ));

            break;
        }
        default:
        {
            IERROR("Invalid MaterialMode");

            break;
        }
        }
    }

    if (a_program.PixelShader != uint32_t(-1))
    {
        IVERIFY(a_program.MaterialMode != MaterialMode_Compute);

        // const VulkanPixelShader* pixelShader = a_gEngine->GetPixelShader(a_program.PixelShader);
        const VulkanPixelShader* pixelShader = ILAMBDA(
        {
            for (uint32_t i = 0; i < shaderCount; ++i)
            {
                const VulkanShader* shader = data->GetShader(i);
                IVERIFY(shader != nullptr);

                const e_VulkanShaderType type = shader->GetShaderType();
                if (type == VulkanShaderType_Pixel)
                {
                    ILRETURN (VulkanPixelShader*)shader;
                }
            }

            ILRETURN (VulkanPixelShader*)nullptr;
        });
        IVERIFY(pixelShader != nullptr);

        const vk::ShaderModule module = pixelShader->GetShaderModule();

        stages.Push(vk::PipelineShaderStageCreateInfo
        (
            { },
            vk::ShaderStageFlagBits::eFragment,
            module,
            "main"
        ));
    }

    return stages;
}

constexpr static vk::CullModeFlags GetCullingMode(e_CullMode a_mode)
{
    switch (a_mode)
    {
    case CullMode_Front:
    {
        return vk::CullModeFlagBits::eFront;
    }
    case CullMode_Back:
    {
        return vk::CullModeFlagBits::eBack;
    }
    case CullMode_Both:
    {
        return vk::CullModeFlagBits::eFrontAndBack;
    }
    case CullMode_None:
    {
        return vk::CullModeFlagBits::eNone;
    }
    }

    IERROR("Invalid Culling mode");

    return vk::CullModeFlagBits::eNone;
}

constexpr static vk::PrimitiveTopology GetPrimitiveMode(e_PrimitiveMode a_mode)
{
    switch (a_mode)
    {
    case PrimitiveMode_TriangleStrip:
    {
        return vk::PrimitiveTopology::eTriangleStrip;
    }
    case PrimitiveMode_Triangles:
    {
        return vk::PrimitiveTopology::eTriangleList;
    }
    }

    IERROR("Invalid Primitive mode");

    return vk::PrimitiveTopology::eTriangleList;
}

constexpr static vk::Format GetFormat(const VertexInputAttribute& a_attrib)
{
    switch (a_attrib.Type)
    {
    case VertexType_Float:
    {
        switch (a_attrib.Count)
        {
        case 1:
        {
            return vk::Format::eR32Sfloat;
        }
        case 2:
        {
            return vk::Format::eR32G32Sfloat;
        }
        case 3:
        {
            return vk::Format::eR32G32B32Sfloat;
        }
        case 4:
        {
            return vk::Format::eR32G32B32A32Sfloat;
        }
        }

        break;
    }
    case VertexType_Int:
    {
        switch (a_attrib.Count)
        {
        case 1:
        {
            return vk::Format::eR32Sint;
        }
        case 2:
        {
            return vk::Format::eR32G32Sint;
        }
        case 3:
        {
            return vk::Format::eR32G32B32Sint;
        }
        case 4:
        {
            return vk::Format::eR32G32B32A32Sint;
        }
        }

        break;
    }
    case VertexType_UInt:
    {
        switch (a_attrib.Count)
        {
        case 1:
        {
            return vk::Format::eR32Uint;
        }
        case 2:
        {
            return vk::Format::eR32G32Uint;
        }
        case 3:
        {
            return vk::Format::eR32G32B32Uint;
        }
        case 4:
        {
            return vk::Format::eR32G32B32A32Uint;
        }
        }

        break;
    }
    default:
    {
        break;
    }
    }

    IERROR("Invalid Vertex format");

    return vk::Format::eUndefined;
}

VulkanPipeline::VulkanPipeline
(
    vk::Pipeline a_pipeline,
    VulkanRenderEngineBackend* a_engine,
    VulkanGraphicsEngine* a_gEngine,
    uint32_t a_programAddr,
    e_VulkanPipelineType a_type
)
{
    m_pipeline = a_pipeline;

    m_engine = a_engine;
    m_gEngine = a_gEngine;

    m_programAddr = a_programAddr;

    m_type = a_type;
}
VulkanPipeline::~VulkanPipeline()
{
    TRACE("Queueing Pipeline for deletion");
    m_engine->PushDeletionObject<VulkanPipelineDeletionObject>(m_engine, m_pipeline);
}

VulkanShaderData* VulkanPipeline::GetShaderData() const
{
    const RenderProgram program = m_gEngine->GetRenderProgram(m_programAddr);
    IVERIFY(program.Data != nullptr);

    const VulkanRenderProgramBlob* blob = (VulkanRenderProgramBlob*)program.Data;

    switch (m_type)
    {
    case VulkanPipelineType_Graphics:
    case VulkanPipelineType_Compute:
    {
        return blob->Base;
    }
    case VulkanPipelineType_Shadow:
    {
        return blob->Shadow;
    }
    case VulkanPipelineType_EmulatedMesh:
    {
        return blob->Secondary;
    }
    case VulkanPipelineType_EmulatedTask:
    {
        return blob->Tertiary;
    }
    default:
    {
        break;
    }
    }

    IERROR("Invalid Vulkan Pipeline type");

    ILRETURN (VulkanShaderData*)nullptr;
}

bool VulkanPipeline::Bind(uint32_t a_index, vk::CommandBuffer a_commandBuffer) const
{
    const VulkanShaderData* data = GetShaderData();
    IVERIFY(data != nullptr);

    if (!data->Bind(a_index, a_commandBuffer))
    {
        return false;
    }

    const vk::PipelineBindPoint bindPoint = ILAMBDA(
    {
        switch (m_type)
        {
        case VulkanPipelineType_Graphics:
        case VulkanPipelineType_Shadow:
        {
            ILRETURN vk::PipelineBindPoint::eGraphics;
        }
        case VulkanPipelineType_Compute:
        case VulkanPipelineType_EmulatedMesh:
        case VulkanPipelineType_EmulatedTask:
        {
            ILRETURN vk::PipelineBindPoint::eCompute;
        }
        default:
        {
            break;
        }
        }

        IERROR("Invalid pipeline type");

        ILRETURN vk::PipelineBindPoint::eGraphics;
    });

    a_commandBuffer.bindPipeline(bindPoint, m_pipeline);

    return true;
}
void VulkanPipeline::Unbind(uint32_t a_index, vk::CommandBuffer a_commandBuffer) const
{
    const VulkanShaderData* data = GetShaderData();
    IVERIFY(data != nullptr);

    data->Unbind(a_index, a_commandBuffer);
}

void VulkanPipeline::CreateComputePipeline(VulkanPipeline* a_out, const VulkanGraphicsComputePipelineBuilder& a_builder)
{
    const vk::Device device = a_builder.Engine->GetLogicalDevice();
    const RenderProgram program = a_builder.GraphicsEngine->GetRenderProgram(a_builder.ProgramAddr);
    IVERIFY(program.Data != nullptr);
    IVERIFY(program.ExtraShader != uint32_t(-1));

    const VulkanRenderProgramBlob* blob = (VulkanRenderProgramBlob*)program.Data;
    IVERIFY(blob->Base != nullptr);

    const VulkanShaderData* shaderData = blob->Base;

    IVERIFY(shaderData->GetShaderCount() == 1);

    const VulkanShader* shader = shaderData->GetShader(0);
    IVERIFY(shader != nullptr);
    IVERIFY(shader->GetShaderType() == VulkanShaderType_Compute);

    const vk::ShaderModule module = shader->GetShaderModule();
    const vk::PipelineShaderStageCreateInfo computeStage = vk::PipelineShaderStageCreateInfo
    (
        { },
        vk::ShaderStageFlagBits::eCompute,
        module,
        "main"
    );

    const vk::PipelineLayout layout = shaderData->GetLayout();
    const vk::ComputePipelineCreateInfo createInfo = vk::ComputePipelineCreateInfo
    (
        { },
        computeStage,
        layout
    );

    vk::Pipeline pipeline;
    VKRESERRMSG(device.createComputePipelines(nullptr, 1, &createInfo, nullptr, &pipeline), "Failed to create Vulkan Graphics Compute Pipeline");

    new (a_out) VulkanPipeline(pipeline, a_builder.Engine, a_builder.GraphicsEngine, a_builder.ProgramAddr, VulkanPipelineType_Compute);
}

void VulkanPipeline::CreatePipeline(VulkanPipeline* a_out, const VulkanGraphicsPipelineBuilder& a_builder, IcarianCore::Allocator* a_tempAllocator)
{
    TRACE("Creating Vulkan Pipeline");
    const vk::Device device = a_builder.Engine->GetLogicalDevice();

    const RenderProgram program = a_builder.GraphicsEngine->GetRenderProgram(a_builder.ProgramAddr);
    IVERIFY(program.Data != nullptr);

    const VulkanRenderProgramBlob* blob = (VulkanRenderProgramBlob*)program.Data;
    IVERIFY(blob->Base != nullptr);

    const VulkanShaderData* shaderData = blob->Base;

    const vk::PipelineDynamicStateCreateInfo dynamicState = vk::PipelineDynamicStateCreateInfo
    (
        { },
        DynamicStateCount,
        DynamicStates
    );

    const bool meshEnabled = a_builder.Engine->IsMeshEnabled();
    const uint32_t vertexStride = ILAMBDA(
    {
        if (!meshEnabled && program.MaterialMode == MaterialMode_BaseMesh)
        {
            // Forced to use std140 so just the number of elements x 16 byte stride between elements regardless of the size of said elements
            ILRETURN shaderData->GetAttributeCount() * 16;
        }

        ILRETURN (uint32_t)program.VertexStride;
    });

    const vk::VertexInputBindingDescription bindingDescription = vk::VertexInputBindingDescription
    (
        0,
        vertexStride,
        vk::VertexInputRate::eVertex
    );

    const uint32_t vertexInputCount = ILAMBDA(
    {
        if (!meshEnabled && program.MaterialMode == MaterialMode_BaseMesh)
        {
            ILRETURN shaderData->GetAttributeCount();
        }

        ILRETURN (uint32_t)program.VertexInputCount;
    });

    vk::VertexInputAttributeDescription* attributeDescription = ILAMBDA(
    {
        if (!meshEnabled && program.MaterialMode == MaterialMode_BaseMesh)
        {
            vk::VertexInputAttributeDescription* vals = a_tempAllocator->TAllocate<vk::VertexInputAttributeDescription>(vertexInputCount);

            for (uint32_t i = 0; i < vertexInputCount; ++i)
            {
                const VertexInputAttribute attrib = shaderData->GetAttribute(i);

                const vk::Format format = GetFormat(attrib);

                const vk::VertexInputAttributeDescription desc = vk::VertexInputAttributeDescription
                (
                    (uint32_t)attrib.Location,
                    0,
                    format,
                    (uint32_t)attrib.Offset
                );

                vals[i] = desc;
            }

            ILRETURN vals;
        }

        if (vertexInputCount > 0)
        {
            vk::VertexInputAttributeDescription* vals = a_tempAllocator->TAllocate<vk::VertexInputAttributeDescription>(vertexInputCount);

            for (uint32_t i = 0; i < vertexInputCount; ++i)
            {
                const VertexInputAttribute& attrib = program.VertexAttributes[i];

                const vk::Format format = GetFormat(attrib);

                const vk::VertexInputAttributeDescription desc = vk::VertexInputAttributeDescription
                (
                    (uint32_t)attrib.Location,
                    0,
                    format,
                    (uint32_t)attrib.Offset
                );

                vals[i] = desc;
            }

            ILRETURN vals;
        }

        ILRETURN (vk::VertexInputAttributeDescription*)nullptr;
    });
    IDEFER(
    {
        if (attributeDescription != nullptr)
        {
            a_tempAllocator->Free(attributeDescription);
        }
    });

    const vk::PipelineVertexInputStateCreateInfo vertexInputInfo = ILAMBDA(
    {
        if (vertexInputCount > 0)
        {
            ILRETURN vk::PipelineVertexInputStateCreateInfo
            (
                { },
                1,
                &bindingDescription,
                vertexInputCount,
                attributeDescription
            );
        }

        ILRETURN vk::PipelineVertexInputStateCreateInfo();
    });

    const vk::PrimitiveTopology primitiveMode = GetPrimitiveMode(program.PrimitiveMode);

    const vk::PipelineInputAssemblyStateCreateInfo inputAssembly = vk::PipelineInputAssemblyStateCreateInfo
    (
        { },
        primitiveMode,
        vk::False
    );

    const vk::PipelineViewportStateCreateInfo viewportState = vk::PipelineViewportStateCreateInfo
    (
        { },
        1,
        &Viewport,
        1,
        &Scissor
    );

    const vk::CullModeFlags cullFlags = GetCullingMode(program.CullingMode);

    const vk::PipelineRasterizationStateCreateInfo rasterizer = vk::PipelineRasterizationStateCreateInfo
    (
        { },
        vk::False,
        vk::False,
        vk::PolygonMode::eFill,
        cullFlags,
        vk::FrontFace::eClockwise,
        vk::False,
        0.0f,
        0.0f,
        0.0f,
        1.0f
    );

    const vk::Bool32 depthEnable = ILAMBDA(
    {
        switch (program.ColorBlendMode)
        {
        case MaterialBlendMode_One:
        case MaterialBlendMode_Alpha:
        case MaterialBlendMode_AlphaBlend:
        {
            ILRETURN vk::False;
        }
        default:
        {
            break;
        }
        }

        ILRETURN vk::True;
    });

    const vk::PipelineDepthStencilStateCreateInfo depthStencil = vk::PipelineDepthStencilStateCreateInfo
    (
        { },
        vk::True,
        depthEnable,
        vk::CompareOp::eLess,
        vk::False,
        vk::False
    );

    constexpr vk::ColorComponentFlags WriteMask =
        vk::ColorComponentFlagBits::eR |
        vk::ColorComponentFlagBits::eG |
        vk::ColorComponentFlagBits::eB |
        vk::ColorComponentFlagBits::eA;

    const vk::PipelineColorBlendAttachmentState colorBlendAttachment = ILAMBDA(
    {
        switch (program.ColorBlendMode)
        {
        case MaterialBlendMode_None:
        {
            ILRETURN vk::PipelineColorBlendAttachmentState
            (
                vk::False,
                vk::BlendFactor::eZero,
                vk::BlendFactor::eZero,
                vk::BlendOp::eAdd,
                vk::BlendFactor::eZero,
                vk::BlendFactor::eZero,
                vk::BlendOp::eAdd,
                WriteMask
            );
        }
        case MaterialBlendMode_One:
        {
            ILRETURN vk::PipelineColorBlendAttachmentState
            (
                vk::True,
                vk::BlendFactor::eOne,
                vk::BlendFactor::eOne,
                vk::BlendOp::eAdd,
                vk::BlendFactor::eOne,
                vk::BlendFactor::eOne,
                vk::BlendOp::eAdd,
                WriteMask
            );
        }
        case MaterialBlendMode_Alpha:
        {
            ILRETURN vk::PipelineColorBlendAttachmentState
            (
                vk::True,
                vk::BlendFactor::eSrcAlpha,
                vk::BlendFactor::eOneMinusSrcAlpha,
                vk::BlendOp::eAdd,
                vk::BlendFactor::eOne,
                vk::BlendFactor::eOne,
                vk::BlendOp::eAdd,
                WriteMask
            );
        }
        case MaterialBlendMode_AlphaBlend:
        {
            ILRETURN vk::PipelineColorBlendAttachmentState
            (
                vk::True,
                vk::BlendFactor::eSrcAlpha,
                vk::BlendFactor::eDstAlpha,
                vk::BlendOp::eAdd,
                vk::BlendFactor::eSrcAlpha,
                vk::BlendFactor::eDstAlpha,
                vk::BlendOp::eAdd,
                WriteMask
            );
        }
        default:
        {
            IERROR("Invalid MaterialBlendMode");

            break;
        }
        }

        ILRETURN vk::PipelineColorBlendAttachmentState();
    });

    vk::PipelineColorBlendAttachmentState* colorBlendAttachments = ILAMBDA(
    {
        vk::PipelineColorBlendAttachmentState* vals = a_tempAllocator->TAllocate<vk::PipelineColorBlendAttachmentState>(a_builder.TextureCount);

        for (uint32_t i = 0; i < a_builder.TextureCount; ++i)
        {
            vals[i] = colorBlendAttachment;
        }

        ILRETURN vals;
    });
    IDEFER(
    {
        if (colorBlendAttachments != nullptr)
        {
            a_tempAllocator->Free(colorBlendAttachments);
        }
    });

    const vk::PipelineColorBlendStateCreateInfo colorBlending = vk::PipelineColorBlendStateCreateInfo
    (
        { },
        vk::False,
        vk::LogicOp::eCopy,
        a_builder.TextureCount,
        colorBlendAttachments
    );

    const IcarianCore::Array<vk::PipelineShaderStageCreateInfo> shaderStages = GetStageInfo
    (
        program,
        meshEnabled,
        a_tempAllocator
    );

    const vk::PipelineLayout layout = shaderData->GetLayout();

    const vk::PipelineDepthStencilStateCreateInfo* depthInfo = ILAMBDA(
    {
        if (a_builder.Depth)
        {
            ILRETURN &depthStencil;
        }

        ILRETURN (const vk::PipelineDepthStencilStateCreateInfo*)nullptr;
    });

    const vk::GraphicsPipelineCreateInfo pipelineInfo = vk::GraphicsPipelineCreateInfo
    (
        { },
        shaderStages.Size(),
        shaderStages.Data(),
        &vertexInputInfo,
        &inputAssembly,
        nullptr,
        &viewportState,
        &rasterizer,
        &Multisampling,
        depthInfo,
        &colorBlending,
        &dynamicState,
        layout,
        a_builder.RenderPass
    );

    vk::Pipeline pipeline;
    VKRESERRMSG(device.createGraphicsPipelines
    (
        nullptr,
        1,
        &pipelineInfo,
        nullptr,
        &pipeline
    ), "Failed to create Vulkan Pipeline");

    new (a_out) VulkanPipeline
    (
        pipeline,
        a_builder.Engine,
        a_builder.GraphicsEngine,
        a_builder.ProgramAddr,
        VulkanPipelineType_Graphics
    );
}

void VulkanPipeline::CreateMeshComputePipeline(VulkanPipeline* a_out, const VulkanGraphicsComputePipelineBuilder& a_builder)
{
    IVERIFY(!a_builder.Engine->IsMeshEnabled());

    const vk::Device device = a_builder.Engine->GetLogicalDevice();
    const RenderProgram program = a_builder.GraphicsEngine->GetRenderProgram(a_builder.ProgramAddr);
    IVERIFY(program.Data != nullptr);
    IVERIFY(program.VertexShader != uint32_t(-1));

    const VulkanRenderProgramBlob* blob = (VulkanRenderProgramBlob*)program.Data;
    IVERIFY(blob->Secondary != nullptr);

    const VulkanShaderData* shaderData = blob->Secondary;

    IVERIFY(shaderData->GetShaderCount() == 1);

    const VulkanShader* shader = shaderData->GetShader(0);
    IVERIFY(shader != nullptr);

    const vk::ShaderModule module = shader->GetShaderModule();

    const vk::PipelineShaderStageCreateInfo computeStage = vk::PipelineShaderStageCreateInfo
    (
        { },
        vk::ShaderStageFlagBits::eCompute,
        module,
        "main"
    );

    const vk::PipelineLayout layout = shaderData->GetLayout();
    const vk::ComputePipelineCreateInfo createInfo = vk::ComputePipelineCreateInfo
    (
        { },
        computeStage,
        layout
    );

    vk::Pipeline pipeline;
    VKRESERRMSG(device.createComputePipelines
    (
        nullptr,
        1,
        &createInfo,
        nullptr,
        &pipeline
    ), "Failed to create Vulkan Graphics Compute Pipeline");

    new (a_out) VulkanPipeline
    (
        pipeline,
        a_builder.Engine,
        a_builder.GraphicsEngine,
        a_builder.ProgramAddr,
        VulkanPipelineType_EmulatedMesh
    );
}

void VulkanPipeline::CreateShadowPipeline(VulkanPipeline* a_out, const VulkanGraphicsPipelineBuilder& a_builder, IcarianCore::Allocator* a_tempAllocator)
{
    // TODO: Support using mesh shaders
    TRACE("Creating Vulkan Shadow Pipeline");
    const vk::Device device = a_builder.Engine->GetLogicalDevice();
    const RenderProgram program = a_builder.GraphicsEngine->GetRenderProgram(a_builder.ProgramAddr);
    IVERIFY(program.Data != nullptr);
    IVERIFY(program.ShadowVertexShader != uint32_t(-1));

    const VulkanRenderProgramBlob* blob = (VulkanRenderProgramBlob*)program.Data;
    IVERIFY(blob->Shadow != nullptr);

    const VulkanShaderData* shaderData = blob->Shadow;

    const vk::PipelineDynamicStateCreateInfo dynamicState = vk::PipelineDynamicStateCreateInfo
    (
        { },
        ShadowDynamicStateCount,
        ShadowDynamicStates
    );

    const vk::VertexInputBindingDescription bindingDescription = vk::VertexInputBindingDescription
    (
        0,
        program.VertexStride,
        vk::VertexInputRate::eVertex
    );

    vk::VertexInputAttributeDescription* attributeDescription = ILAMBDA(
    {
        vk::VertexInputAttributeDescription* vals = a_tempAllocator->TAllocate<vk::VertexInputAttributeDescription>(program.VertexInputCount);

        for (uint32_t i = 0; i < program.VertexInputCount; ++i)
        {
            const VertexInputAttribute& attrib = program.VertexAttributes[i];

            const vk::Format format = GetFormat(attrib);

            const vk::VertexInputAttributeDescription desc = vk::VertexInputAttributeDescription
            (
                attrib.Location,
                0,
                format,
                attrib.Offset
            );

            vals[i] = desc;
        }

        ILRETURN vals;
    });
    IDEFER(
    {
        if (attributeDescription != nullptr)
        {
            a_tempAllocator->Free(attributeDescription);
        }
    });

    const vk::PipelineVertexInputStateCreateInfo vertexInputInfo = ILAMBDA(
    {
        if (program.VertexInputCount > 0)
        {
            vk::PipelineVertexInputStateCreateInfo
            (
                { },
                1,
                &bindingDescription,
                program.VertexInputCount,
                attributeDescription
            );
        }

        ILRETURN vk::PipelineVertexInputStateCreateInfo();
    });

    const vk::PrimitiveTopology primitiveMode = GetPrimitiveMode(program.PrimitiveMode);
    const vk::PipelineInputAssemblyStateCreateInfo inputAssembly = vk::PipelineInputAssemblyStateCreateInfo
    (
        { },
        primitiveMode,
        vk::False
    );

    const vk::PipelineViewportStateCreateInfo viewportState = vk::PipelineViewportStateCreateInfo
    (
        { },
        1,
        &Viewport,
        1,
        &Scissor
    );

    const vk::CullModeFlags cullMode = GetCullingMode(program.CullingMode);
    const vk::PipelineRasterizationStateCreateInfo rasterizer = vk::PipelineRasterizationStateCreateInfo
    (
        { },
        vk::False,
        vk::False,
        vk::PolygonMode::eFill,
        cullMode,
        vk::FrontFace::eClockwise,
        vk::True,
        0.0f,
        0.0f,
        0.0f,
        1.0f
    );

    constexpr vk::PipelineColorBlendAttachmentState ColorBlendAttachment = vk::PipelineColorBlendAttachmentState
    (
        vk::False,
        vk::BlendFactor::eOne,
        vk::BlendFactor::eOne,
        vk::BlendOp::eAdd,
        vk::BlendFactor::eOne,
        vk::BlendFactor::eOne,
        vk::BlendOp::eAdd,
        vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
    );

    const vk::PipelineColorBlendStateCreateInfo colorBlending = vk::PipelineColorBlendStateCreateInfo
    (
        { },
        vk::False,
        vk::LogicOp::eCopy,
        1,
        &ColorBlendAttachment
    );

    const uint32_t shaderCount = shaderData->GetShaderCount();
    const VulkanVertexShader* vertexShader = ILAMBDA(
    {
        for (uint32_t i = 0; i < shaderCount; ++i)
        {
            const VulkanShader* shader = shaderData->GetShader(i);
            IVERIFY(shader != nullptr);

            const e_VulkanShaderType type = shader->GetShaderType();
            if (type == VulkanShaderType_Vertex)
            {
                ILRETURN (VulkanVertexShader*)shader;
            }
        }

        ILRETURN (VulkanVertexShader*)nullptr;
    });

    const vk::ShaderModule module = vertexShader->GetShaderModule();
    const vk::PipelineShaderStageCreateInfo vertexStage = vk::PipelineShaderStageCreateInfo
    (
        { },
        vk::ShaderStageFlagBits::eVertex,
        module,
        "main"
    );

    constexpr vk::PipelineDepthStencilStateCreateInfo DepthStencil = vk::PipelineDepthStencilStateCreateInfo
    (
        { },
        vk::True,
        vk::True,
        vk::CompareOp::eLess,
        vk::False,
        vk::False
    );

    const vk::PipelineLayout layout = shaderData->GetLayout();

    const vk::GraphicsPipelineCreateInfo pipelineInfo = vk::GraphicsPipelineCreateInfo
    (
        { },
        1,
        &vertexStage,
        &vertexInputInfo,
        &inputAssembly,
        nullptr,
        &viewportState,
        &rasterizer,
        &Multisampling,
        &DepthStencil,
        &colorBlending,
        &dynamicState,
        layout,
        a_builder.RenderPass
    );

    vk::Pipeline pipeline;
    VKRESERRMSG(device.createGraphicsPipelines
    (
        nullptr,
        1,
        &pipelineInfo,
        nullptr,
        &pipeline
    ), "Failed to create Vulkan Shadow Pipeline");

    new (a_out) VulkanPipeline
    (
        pipeline,
        a_builder.Engine,
        a_builder.GraphicsEngine,
        a_builder.ProgramAddr,
        VulkanPipelineType_Shadow
    );
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
