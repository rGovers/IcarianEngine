// Icarian Engine - C# Game Engine
// 
// License at end of file.

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/VulkanPipeline.h"

#include "Core/IcarianAssert.h"
#include "Core/IcarianDefer.h"
#include "Rendering/Vulkan/VulkanGraphicsEngine.h"
#include "Rendering/Vulkan/VulkanPixelShader.h"
#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"
#include "Rendering/Vulkan/VulkanShaderData.h"
#include "Rendering/Vulkan/VulkanVertexShader.h"
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

static Array<vk::PipelineShaderStageCreateInfo> GetStageInfo(const RenderProgram& a_program, VulkanGraphicsEngine* a_gEngine)
{
    Array<vk::PipelineShaderStageCreateInfo> stages;

    if (a_program.VertexShader != -1)
    {
        const VulkanShader* vertexShader = a_gEngine->GetVertexShader(a_program.VertexShader);
        IVERIFY(vertexShader != nullptr);

        stages.Push(vk::PipelineShaderStageCreateInfo
        (
            { },
            vk::ShaderStageFlagBits::eVertex,
            vertexShader->GetShaderModule(),
            "main"
        ));
    }

    if (a_program.PixelShader != -1)
    {
        const VulkanShader* pixelShader = a_gEngine->GetPixelShader(a_program.PixelShader);
        IVERIFY(pixelShader != nullptr);

        stages.Push(vk::PipelineShaderStageCreateInfo
        (
            { },
            vk::ShaderStageFlagBits::eFragment,
            pixelShader->GetShaderModule(),
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

    ICARIAN_ASSERT(0);

    return vk::CullModeFlagBits::eNone;
}
constexpr static vk::CullModeFlags GetInvCullingMode(e_CullMode a_mode)
{
    switch (a_mode)
    {
    case CullMode_Front:
    {
        return vk::CullModeFlagBits::eBack;
    }
    case CullMode_Back:
    {
        return vk::CullModeFlagBits::eFront;
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
    
    ICARIAN_ASSERT(0);

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

    ICARIAN_ASSERT(0);

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

        ICARIAN_ASSERT(0);

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

        ICARIAN_ASSERT(0);

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

        ICARIAN_ASSERT(0);

        break;
    }
    default:
    {
        ICARIAN_ASSERT(0);

        break;
    }
    }

    return vk::Format::eUndefined;
}

VulkanPipeline::VulkanPipeline(vk::Pipeline a_pipeline, VulkanRenderEngineBackend* a_engine, VulkanGraphicsEngine* a_gEngine, uint32_t a_programAddr, e_VulkanPipelineType a_type)
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
    m_engine->PushDeletionObject(new VulkanPipelineDeletionObject(m_engine, m_pipeline));
}

VulkanShaderData* VulkanPipeline::GetShaderData() const
{
    const RenderProgram program = m_gEngine->GetRenderProgram(m_programAddr);
    IVERIFY(program.Data != nullptr);
    
    return (VulkanShaderData*)program.Data;
}
void VulkanPipeline::Bind(uint32_t a_index, vk::CommandBuffer a_commandBuffer) const
{
    const RenderProgram program = m_gEngine->GetRenderProgram(m_programAddr);
    IVERIFY(program.Data != nullptr);

    const VulkanShaderData* data = (VulkanShaderData*)program.Data;

    switch (m_type)
    {
    case VulkanPipelineType_Graphics:
    {
        data->Bind(a_index, a_commandBuffer);

        break;
    }
    case VulkanPipelineType_Shadow:
    {
        data->BindShadow(a_index, a_commandBuffer);

        break;
    }
    }

    a_commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline);
}

VulkanPipeline* VulkanPipeline::CreatePipeline(VulkanRenderEngineBackend* a_engine, VulkanGraphicsEngine* a_gEngine, const vk::RenderPass& a_renderPass, bool a_depth, uint32_t a_textureCount, uint32_t a_programAddr)
{
    TRACE("Creating Vulkan Pipeline");
    const vk::Device device = a_engine->GetLogicalDevice();
    const RenderProgram program = a_gEngine->GetRenderProgram(a_programAddr);
    IVERIFY(program.Data != nullptr);
    const VulkanShaderData* shaderData = (VulkanShaderData*)program.Data;

    const vk::PipelineDynamicStateCreateInfo dynamicState = vk::PipelineDynamicStateCreateInfo
    (
        { },
        DynamicStateCount,
        DynamicStates
    );

    const vk::VertexInputBindingDescription bindingDescription = vk::VertexInputBindingDescription
    (
        0,
        program.VertexStride,
        vk::VertexInputRate::eVertex
    );

    vk::VertexInputAttributeDescription* attributeDescription = new vk::VertexInputAttributeDescription[program.VertexInputCount];
    IDEFER(delete[] attributeDescription);

    for (uint16_t i = 0; i < program.VertexInputCount; ++i)
    {
        const VertexInputAttribute& attrib = program.VertexAttributes[i];

        attributeDescription[i].binding = 0;
        attributeDescription[i].location = attrib.Location;
        attributeDescription[i].offset = attrib.Offset;
        attributeDescription[i].format = GetFormat(attrib);
    }

    vk::PipelineVertexInputStateCreateInfo vertexInputInfo = vk::PipelineVertexInputStateCreateInfo
    (
        { },
        0,
        nullptr,
        (uint32_t)program.VertexInputCount,
        attributeDescription
    );

    if (program.VertexInputCount > 0)
    {
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    }

    const vk::PipelineInputAssemblyStateCreateInfo inputAssembly = vk::PipelineInputAssemblyStateCreateInfo
    (
        { },
        GetPrimitiveMode(program.PrimitiveMode),
        VK_FALSE
    );

    const vk::PipelineViewportStateCreateInfo viewportState = vk::PipelineViewportStateCreateInfo
    (
        { },
        1,
        &Viewport,
        1,
        &Scissor
    );

    const vk::PipelineRasterizationStateCreateInfo rasterizer = vk::PipelineRasterizationStateCreateInfo
    (
        { },
        VK_FALSE,
        VK_FALSE,
        vk::PolygonMode::eFill,
        GetCullingMode(program.CullingMode),
        vk::FrontFace::eClockwise,
        VK_FALSE,
        0.0f,
        0.0f,
        0.0f,
        1.0f
    );

    vk::PipelineDepthStencilStateCreateInfo depthStencil = vk::PipelineDepthStencilStateCreateInfo
    (
        { },
        VK_TRUE,
        VK_TRUE,
        vk::CompareOp::eLess,
        VK_FALSE,
        VK_FALSE
    );

    vk::PipelineColorBlendAttachmentState colorBlendAttachment;
    // Even if we are not blending we need a write mask
    colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
    switch (program.ColorBlendMode)
    {
    case MaterialBlendMode_None:
    {
        colorBlendAttachment.blendEnable = VK_FALSE;

        break;
    }
    case MaterialBlendMode_One:
    {
        depthStencil.depthWriteEnable = VK_FALSE;

        colorBlendAttachment.blendEnable = VK_TRUE;

        colorBlendAttachment.srcColorBlendFactor = vk::BlendFactor::eOne;
        colorBlendAttachment.dstColorBlendFactor = vk::BlendFactor::eOne;
        colorBlendAttachment.colorBlendOp = vk::BlendOp::eAdd;

        colorBlendAttachment.srcAlphaBlendFactor = vk::BlendFactor::eOne;
        colorBlendAttachment.dstAlphaBlendFactor = vk::BlendFactor::eOne;
        colorBlendAttachment.alphaBlendOp = vk::BlendOp::eAdd;

        break;
    }
    case MaterialBlendMode_Alpha:
    {
        depthStencil.depthWriteEnable = VK_FALSE;

        colorBlendAttachment.blendEnable = VK_TRUE;

        colorBlendAttachment.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
        colorBlendAttachment.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
        colorBlendAttachment.colorBlendOp = vk::BlendOp::eAdd;

        colorBlendAttachment.srcAlphaBlendFactor = vk::BlendFactor::eOne;
        colorBlendAttachment.dstAlphaBlendFactor = vk::BlendFactor::eOne;
        colorBlendAttachment.alphaBlendOp = vk::BlendOp::eAdd;

        break;
    }
    case MaterialBlendMode_AlphaBlend:
    {
        depthStencil.depthWriteEnable = VK_FALSE;

        colorBlendAttachment.blendEnable = VK_TRUE;

        colorBlendAttachment.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
        colorBlendAttachment.dstColorBlendFactor = vk::BlendFactor::eDstAlpha;
        colorBlendAttachment.colorBlendOp = vk::BlendOp::eAdd;

        colorBlendAttachment.srcAlphaBlendFactor = vk::BlendFactor::eSrcAlpha;
        colorBlendAttachment.dstAlphaBlendFactor = vk::BlendFactor::eDstAlpha;
        colorBlendAttachment.alphaBlendOp = vk::BlendOp::eAdd;

        break;
    }
    default:
    {
        ICARIAN_ASSERT(0);

        break;
    }
    }

    vk::PipelineColorBlendAttachmentState* colorBlendAttachments = new vk::PipelineColorBlendAttachmentState[a_textureCount];
    IDEFER(delete[] colorBlendAttachments);
    for (uint32_t i = 0; i < a_textureCount; ++i)
    {
        colorBlendAttachments[i] = colorBlendAttachment;
    }

    const vk::PipelineColorBlendStateCreateInfo colorBlending = vk::PipelineColorBlendStateCreateInfo
    (
        { },
        VK_FALSE,
        vk::LogicOp::eCopy,
        a_textureCount,
        colorBlendAttachments
    );
    
    const Array<vk::PipelineShaderStageCreateInfo> shaderStages = GetStageInfo(program, a_gEngine);

    const vk::PipelineLayout layout = shaderData->GetLayout();

    vk::GraphicsPipelineCreateInfo pipelineInfo = vk::GraphicsPipelineCreateInfo
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
        nullptr,
        &colorBlending,
        &dynamicState,
        layout,
        a_renderPass
    );
    if (a_depth)
    {   
        pipelineInfo.pDepthStencilState = &depthStencil;
    }

    vk::Pipeline pipeline;
    VKRESERRMSG(device.createGraphicsPipelines(nullptr, 1, &pipelineInfo, nullptr, &pipeline), "Failed to create Vulkan Pipeline");

    return new VulkanPipeline(pipeline, a_engine, a_gEngine, a_programAddr, VulkanPipelineType_Graphics);
}

VulkanPipeline* VulkanPipeline::CreateShadowPipeline(VulkanRenderEngineBackend* a_engine, VulkanGraphicsEngine* a_gEngine, const vk::RenderPass& a_renderPass, uint32_t a_programAddr)
{
    const vk::Device device = a_engine->GetLogicalDevice();
    const RenderProgram program = a_gEngine->GetRenderProgram(a_programAddr);
    IVERIFY(program.Data != nullptr);
    IVERIFY(program.ShadowVertexShader != -1);

    TRACE("Creating Vulkan Shadow Pipeline");
    const VulkanShaderData* shaderData = (VulkanShaderData*)program.Data;

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

    vk::VertexInputAttributeDescription* attributeDescription = new vk::VertexInputAttributeDescription[program.VertexInputCount];
    IDEFER(delete[] attributeDescription);

    for (uint16_t i = 0; i < program.VertexInputCount; ++i)
    {
        const VertexInputAttribute& attrib = program.VertexAttributes[i];

        attributeDescription[i].binding = 0;
        attributeDescription[i].location = attrib.Location;
        attributeDescription[i].offset = attrib.Offset;
        attributeDescription[i].format = GetFormat(attrib);
    }

    vk::PipelineVertexInputStateCreateInfo vertexInputInfo = vk::PipelineVertexInputStateCreateInfo
    (
        { },
        0,
        nullptr,
        (uint32_t)program.VertexInputCount,
        attributeDescription
    );
    if (program.VertexInputCount > 0)
    {
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    }

    const vk::PrimitiveTopology primitiveMode = GetPrimitiveMode(program.PrimitiveMode);
    const vk::PipelineInputAssemblyStateCreateInfo inputAssembly = vk::PipelineInputAssemblyStateCreateInfo
    (
        { },
        primitiveMode,
        VK_FALSE
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
        VK_FALSE,
        VK_FALSE,
        vk::PolygonMode::eFill,
        cullMode,
        vk::FrontFace::eClockwise,
        VK_TRUE,
        0.0f,
        0.0f,
        0.0f,
        1.0f
    );

    constexpr vk::PipelineColorBlendAttachmentState ColorBlendAttachment = vk::PipelineColorBlendAttachmentState
    (
        VK_FALSE,
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
        VK_FALSE,
        vk::LogicOp::eCopy,
        1,
        &ColorBlendAttachment
    );

    const VulkanShader* vertexShader = a_gEngine->GetVertexShader(program.ShadowVertexShader);
    IVERIFY(vertexShader != nullptr);

    const vk::PipelineShaderStageCreateInfo vertexStage = vk::PipelineShaderStageCreateInfo
    (
        { },
        vk::ShaderStageFlagBits::eVertex,
        vertexShader->GetShaderModule(),
        "main"
    );

    const vk::PipelineLayout layout = shaderData->GetShadowLayout();

    constexpr vk::PipelineDepthStencilStateCreateInfo DepthStencil = vk::PipelineDepthStencilStateCreateInfo
    (
        { },
        VK_TRUE,
        VK_TRUE,
        vk::CompareOp::eLess,
        VK_FALSE,
        VK_FALSE
    );

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
        a_renderPass
    );

    vk::Pipeline pipeline;
    VKRESERRMSG(device.createGraphicsPipelines(nullptr, 1, &pipelineInfo, nullptr, &pipeline), "Failed to create Vulkan Shadow Pipeline");

    return new VulkanPipeline(pipeline, a_engine, a_gEngine, a_programAddr, VulkanPipelineType_Shadow);
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