#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/VulkanPipeline.h"

#include "Flare/IcarianAssert.h"
#include "Flare/Vertices.h"
#include "Logger.h"
#include "Rendering/Vulkan/VulkanGraphicsEngine.h"
#include "Rendering/Vulkan/VulkanPixelShader.h"
#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"
#include "Rendering/Vulkan/VulkanShaderData.h"
#include "Rendering/Vulkan/VulkanVertexShader.h"
#include "Trace.h"

static std::vector<vk::PipelineShaderStageCreateInfo> GetStageInfo(const RenderProgram& a_program, VulkanGraphicsEngine* a_gEngine)
{
    std::vector<vk::PipelineShaderStageCreateInfo> stages;

    if (a_program.VertexShader != -1)
    {
        const VulkanShader* vertexShader = a_gEngine->GetVertexShader(a_program.VertexShader);
        ICARIAN_ASSERT_MSG(vertexShader != nullptr, "Failed to find vertex shader")

        stages.emplace_back(vk::PipelineShaderStageCreateInfo
        (
            vk::PipelineShaderStageCreateFlags(),
            vk::ShaderStageFlagBits::eVertex,
            vertexShader->GetShaderModule(),
            "main"
        ));
    }

    if (a_program.PixelShader != -1)
    {
        const VulkanShader* pixelShader = a_gEngine->GetPixelShader(a_program.PixelShader);
        ICARIAN_ASSERT_MSG(pixelShader != nullptr, "Failed to find pixel shader");

        stages.emplace_back(vk::PipelineShaderStageCreateInfo
        (
            vk::PipelineShaderStageCreateFlags(),
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
    }

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
    }

    return vk::PrimitiveTopology::eTriangleList;
}

constexpr static vk::Format GetFormat(const FlareBase::VertexInputAttrib& a_attrib) 
{
    switch (a_attrib.Type)
    {
    case FlareBase::VertexType_Float:
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
    case FlareBase::VertexType_Int:
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
    case FlareBase::VertexType_UInt:
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
    }

    return vk::Format::eUndefined;
}

VulkanPipeline::VulkanPipeline(VulkanRenderEngineBackend* a_engine, VulkanGraphicsEngine* a_gEngine, const vk::RenderPass& a_renderPass, bool a_depth, uint32_t a_textureCount, uint32_t a_programAddr)
{
    TRACE("Creating Vulkan Pipeline");
    m_engine = a_engine;
    m_gEngine = a_gEngine;

    m_programAddr = a_programAddr;

    const vk::Device device = m_engine->GetLogicalDevice();
    const RenderProgram program = m_gEngine->GetRenderProgram(m_programAddr);
    const VulkanShaderData* shaderData = (VulkanShaderData*)program.Data;
    ICARIAN_ASSERT(shaderData != nullptr);

    const std::vector<vk::DynamicState> dynamicStates = 
    {
        vk::DynamicState::eViewport,
        vk::DynamicState::eScissor
    };

    const vk::PipelineDynamicStateCreateInfo dynamicState = vk::PipelineDynamicStateCreateInfo
    (
        vk::PipelineDynamicStateCreateFlags(),
        (uint32_t)dynamicStates.size(),
        dynamicStates.data()
    );

    const vk::VertexInputBindingDescription bindingDescription = vk::VertexInputBindingDescription
    (
        0,
        program.VertexStride,
        vk::VertexInputRate::eVertex
    );

    std::vector<vk::VertexInputAttributeDescription> attributeDescription = std::vector<vk::VertexInputAttributeDescription>(program.VertexInputCount);
    for (uint16_t i = 0; i < program.VertexInputCount; ++i)
    {
        const FlareBase::VertexInputAttrib& attrib = program.VertexAttribs[i];
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
        (uint32_t)attributeDescription.size(),
        attributeDescription.data()
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

    constexpr vk::Viewport Viewport = vk::Viewport(0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f);
    constexpr vk::Rect2D Scissor = vk::Rect2D({ 0, 0 }, { 1, 1 });

    const vk::PipelineViewportStateCreateInfo viewportState = vk::PipelineViewportStateCreateInfo
    (
        vk::PipelineViewportStateCreateFlags(),
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

    constexpr vk::PipelineMultisampleStateCreateInfo Multisampling = vk::PipelineMultisampleStateCreateInfo
    (
        vk::PipelineMultisampleStateCreateFlags(),
        vk::SampleCountFlagBits::e1
    );

    vk::PipelineColorBlendAttachmentState colorBlendAttachment = vk::PipelineColorBlendAttachmentState
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
    if (program.EnableColorBlending)
    {
        colorBlendAttachment.blendEnable = VK_TRUE;
    }

    const std::vector<vk::PipelineColorBlendAttachmentState> colorBlendAttachments = std::vector<vk::PipelineColorBlendAttachmentState>(a_textureCount, colorBlendAttachment);

    const vk::PipelineColorBlendStateCreateInfo colorBlending = vk::PipelineColorBlendStateCreateInfo
    (
        { },
        VK_FALSE,
        vk::LogicOp::eCopy,
        a_textureCount,
        colorBlendAttachments.data()
    );
    
    const std::vector<vk::PipelineShaderStageCreateInfo> shaderStages = GetStageInfo(program, a_gEngine);

    constexpr vk::PipelineDepthStencilStateCreateInfo DepthStencil = vk::PipelineDepthStencilStateCreateInfo
    (
        { },
        VK_TRUE,
        VK_TRUE,
        vk::CompareOp::eLess,
        VK_FALSE,
        VK_FALSE
    );

    vk::GraphicsPipelineCreateInfo pipelineInfo = vk::GraphicsPipelineCreateInfo
    (
        { },
        (uint32_t)shaderStages.size(),
        shaderStages.data(),
        &vertexInputInfo,
        &inputAssembly,
        nullptr,
        &viewportState,
        &rasterizer,
        &Multisampling,
        nullptr,
        &colorBlending,
        &dynamicState,
        shaderData->GetLayout(),
        a_renderPass
    );

    if (a_depth)
    {
        pipelineInfo.pDepthStencilState = &DepthStencil;
    }

    TRACE("Creating Pipeline");
    ICARIAN_ASSERT_MSG_R(device.createGraphicsPipelines(nullptr, 1, &pipelineInfo, nullptr, &m_pipeline) == vk::Result::eSuccess, "Failed to create Vulkan Pipeline");
}
VulkanPipeline::~VulkanPipeline()
{
    TRACE("Destroying Pipeline");
    const vk::Device device = m_engine->GetLogicalDevice();

    device.destroyPipeline(m_pipeline);
}

VulkanShaderData* VulkanPipeline::GetShaderData() const
{
    const RenderProgram program = m_gEngine->GetRenderProgram(m_programAddr);
    ICARIAN_ASSERT(program.Data != nullptr);
    
    return (VulkanShaderData*)program.Data;
}
void VulkanPipeline::Bind(uint32_t a_index, vk::CommandBuffer a_commandBuffer) const
{
    const RenderProgram program = m_gEngine->GetRenderProgram(m_programAddr);

    const VulkanShaderData* data = (VulkanShaderData*)program.Data;
    ICARIAN_ASSERT(data != nullptr);

    data->Bind(a_index, a_commandBuffer);

    a_commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline);
}
#endif