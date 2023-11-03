#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/VulkanPipeline.h"

#include "Flare/IcarianAssert.h"
#include "Flare/IcarianDefer.h"
#include "Logger.h"
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

static constexpr vk::Viewport Viewport = vk::Viewport(0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f);
static constexpr vk::Rect2D Scissor = vk::Rect2D({ 0, 0 }, { 1, 1 });

static constexpr vk::PipelineMultisampleStateCreateInfo Multisampling = vk::PipelineMultisampleStateCreateInfo
(
    { },
    vk::SampleCountFlagBits::e1
);

static constexpr vk::PipelineDepthStencilStateCreateInfo DepthStencil = vk::PipelineDepthStencilStateCreateInfo
(
    { },
    VK_TRUE,
    VK_TRUE,
    vk::CompareOp::eLess,
    VK_FALSE,
    VK_FALSE
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

static std::vector<vk::PipelineShaderStageCreateInfo> GetStageInfo(const RenderProgram& a_program, VulkanGraphicsEngine* a_gEngine)
{
    std::vector<vk::PipelineShaderStageCreateInfo> stages;

    if (a_program.VertexShader != -1)
    {
        const VulkanShader* vertexShader = a_gEngine->GetVertexShader(a_program.VertexShader);
        ICARIAN_ASSERT_MSG(vertexShader != nullptr, "Failed to find vertex shader")

        stages.emplace_back(vk::PipelineShaderStageCreateInfo
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
        ICARIAN_ASSERT_MSG(pixelShader != nullptr, "Failed to find pixel shader");

        stages.emplace_back(vk::PipelineShaderStageCreateInfo
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
    }

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
    }

    return vk::Format::eUndefined;
}

VulkanPipeline::VulkanPipeline(VulkanRenderEngineBackend* a_engine, VulkanGraphicsEngine* a_gEngine, uint32_t a_programAddr, e_VulkanPipelineType a_type)
{
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
    ICARIAN_ASSERT(program.Data != nullptr);
    
    return (VulkanShaderData*)program.Data;
}
void VulkanPipeline::Bind(uint32_t a_index, vk::CommandBuffer a_commandBuffer) const
{
    const RenderProgram program = m_gEngine->GetRenderProgram(m_programAddr);
    ICARIAN_ASSERT(program.Data != nullptr);

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
    VulkanPipeline* pipeline = new VulkanPipeline(a_engine, a_gEngine, a_programAddr, VulkanPipelineType_Graphics);

    const vk::Device device = a_engine->GetLogicalDevice();
    const RenderProgram program = a_gEngine->GetRenderProgram(a_programAddr);
    ICARIAN_ASSERT(program.Data != nullptr);
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

    vk::PipelineLayout layout = shaderData->GetLayout();

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
        layout,
        a_renderPass
    );

    if (a_depth)
    {
        pipelineInfo.pDepthStencilState = &DepthStencil;
    }

    ICARIAN_ASSERT_MSG_R(device.createGraphicsPipelines(nullptr, 1, &pipelineInfo, nullptr, &pipeline->m_pipeline) == vk::Result::eSuccess, "Failed to create Vulkan Pipeline");

    return pipeline;
}

VulkanPipeline* VulkanPipeline::CreateShadowPipeline(VulkanRenderEngineBackend* a_engine, VulkanGraphicsEngine* a_gEngine, const vk::RenderPass& a_renderPass, uint32_t a_programAddr)
{
    const vk::Device device = a_engine->GetLogicalDevice();
    const RenderProgram program = a_gEngine->GetRenderProgram(a_programAddr);
    ICARIAN_ASSERT(program.Data != nullptr);
    ICARIAN_ASSERT(program.ShadowVertexShader != -1);

    TRACE("Creating Vulkan Shadow Pipeline");
    VulkanPipeline* pipeline = new VulkanPipeline(a_engine, a_gEngine, a_programAddr, VulkanPipelineType_Shadow);
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
    ICARIAN_ASSERT_MSG(vertexShader != nullptr, "Failed to find vertex shader")

    const vk::PipelineShaderStageCreateInfo vertexStage = vk::PipelineShaderStageCreateInfo
    (
        { },
        vk::ShaderStageFlagBits::eVertex,
        vertexShader->GetShaderModule(),
        "main"
    );

    vk::PipelineLayout layout = shaderData->GetShadowLayout();

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

    ICARIAN_ASSERT_MSG_R(device.createGraphicsPipelines(nullptr, 1, &pipelineInfo, nullptr, &pipeline->m_pipeline) == vk::Result::eSuccess, "Failed to create Vulkan Shadow Pipeline");

    return pipeline;
}
#endif