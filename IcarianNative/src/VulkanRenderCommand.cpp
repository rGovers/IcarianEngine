#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/VulkanRenderCommand.h"

#include "Flare/IcarianAssert.h"
#include "Logger.h"
#include "ObjectManager.h"
#include "Rendering/RenderEngine.h"
#include "Rendering/ShaderBuffers.h"
#include "Rendering/Vulkan/VulkanGraphicsEngine.h"
#include "Rendering/Vulkan/VulkanModel.h"
#include "Rendering/Vulkan/VulkanPipeline.h"
#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"
#include "Rendering/Vulkan/VulkanRenderTexture.h"
#include "Rendering/Vulkan/VulkanShaderData.h"
#include "Rendering/Vulkan/VulkanSwapchain.h"
#include "Rendering/Vulkan/VulkanUniformBuffer.h"

VulkanRenderCommand::VulkanRenderCommand(VulkanRenderEngineBackend* a_engine, VulkanGraphicsEngine* a_gEngine, VulkanSwapchain* a_swapchain, vk::CommandBuffer a_buffer, uint32_t a_camAddr, uint32_t a_bufferIndex)
{
    m_engine = a_engine;
    m_gEngine = a_gEngine;
    m_swapchain = a_swapchain;

    m_commandBuffer = a_buffer;

    m_bufferIndex = a_bufferIndex;

    m_cameraAddr = a_camAddr;
    m_renderTexAddr = -1;
    m_materialAddr = -1;
    
    m_flags = 0;

    SetFlushedState(true);
}
VulkanRenderCommand::~VulkanRenderCommand()
{
    
}

void VulkanRenderCommand::SetFlushedState(bool a_value)
{
    if (a_value)
    {
        m_flags |= 0b1 << FlushedBit;
    }
    else
    {
        m_flags &= ~(0b1 << FlushedBit);
    }
}

void VulkanRenderCommand::Flush()
{
    if (!IsFlushed())
    {
        m_commandBuffer.endRenderPass();
    }

    SetFlushedState(true);

    m_renderTexAddr = -1;
}

VulkanRenderTexture* VulkanRenderCommand::GetRenderTexture() const
{
    return m_gEngine->GetRenderTexture(m_renderTexAddr);
}
VulkanPipeline* VulkanRenderCommand::GetPipeline() const
{
    if (m_materialAddr == -1)
    {
        return nullptr;
    }

    return m_gEngine->GetPipeline(m_renderTexAddr, m_materialAddr);
}

VulkanPipeline* VulkanRenderCommand::BindMaterial(uint32_t a_materialAddr)
{
    const bool bind = m_materialAddr != a_materialAddr;

    m_materialAddr = a_materialAddr;
    if (m_materialAddr == -1)
    {
        return nullptr;
    }

    VulkanPipeline* pipeline = m_gEngine->GetPipeline(m_renderTexAddr, m_materialAddr);
    if (bind)
    {
        const VulkanShaderData* shaderData = pipeline->GetShaderData();
        const uint32_t currentFrame = m_engine->GetCurrentFrame();

        ShaderBufferInput camInput;
        if (shaderData->GetShaderBufferInput(ShaderBufferType_CameraBuffer, &camInput))
        {
            VulkanUniformBuffer* camBuffer = m_gEngine->GetCameraUniformBuffer(m_bufferIndex);

            shaderData->PushUniformBuffer(m_commandBuffer, camInput.Set, camBuffer, currentFrame);
        }

        pipeline->Bind(currentFrame, m_commandBuffer);
    }

    return pipeline;
}

void VulkanRenderCommand::PushTexture(uint32_t a_slot, const TextureSamplerBuffer& a_sampler) const
{
    ICARIAN_ASSERT_MSG_R(m_materialAddr != -1, "PushTexture Material not bound");

    const RenderProgram program = m_gEngine->GetRenderProgram(m_materialAddr);
    VulkanShaderData* data = (VulkanShaderData*)program.Data;

    data->PushTexture(m_commandBuffer, a_slot, a_sampler, m_engine->GetCurrentFrame());
}

void VulkanRenderCommand::BindRenderTexture(uint32_t a_renderTexAddr)
{
    const RenderEngine* renderEngine = m_engine->GetRenderEngine();

    Flush();

    SetFlushedState(false);

    m_renderTexAddr = a_renderTexAddr;

    glm::vec2 screenSize;

    const VulkanRenderTexture* renderTexture = m_gEngine->GetRenderTexture(m_renderTexAddr);

    if (renderTexture == nullptr)
    {
        screenSize = m_swapchain->GetSize();

        constexpr vk::ClearValue ClearColor = vk::ClearValue(vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f));

        const vk::RenderPassBeginInfo renderPassInfo = vk::RenderPassBeginInfo
        (
            m_swapchain->GetRenderPass(),
            m_swapchain->GetFramebuffer(m_engine->GetImageIndex()),
            vk::Rect2D({ 0, 0 }, { (uint32_t)screenSize.x, (uint32_t)screenSize.y }),
            1,
            &ClearColor
        );

        m_commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
    }
    else
    {
        screenSize = glm::vec2(renderTexture->GetWidth(), renderTexture->GetHeight());

        const vk::RenderPassBeginInfo renderPassInfo = vk::RenderPassBeginInfo
        (
            renderTexture->GetRenderPass(),
            renderTexture->GetFramebuffer(),
            vk::Rect2D({ 0, 0 }, { (uint32_t)screenSize.x, (uint32_t)screenSize.y }),
            renderTexture->GetTotalTextureCount(),
            renderTexture->GetClearValues()
        );

        m_commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
    }

    if (m_cameraAddr == -1)
    {
        return;
    }
    
    const CameraBuffer camBuffer = m_gEngine->GetCameraBuffer(m_cameraAddr);
    const glm::vec2 viewPos = camBuffer.View.Position * screenSize;
    const glm::vec2 viewSize = camBuffer.View.Size * screenSize;

    const vk::Rect2D scissor = vk::Rect2D({ (int32_t)viewPos.x, (int32_t)viewPos.y }, { (uint32_t)viewSize.x, (uint32_t)viewSize.y });
    m_commandBuffer.setScissor(0, 1, &scissor);

    const vk::Viewport viewport = vk::Viewport(viewPos.x, viewPos.y, viewSize.x, viewSize.y, camBuffer.View.MinDepth, camBuffer.View.MaxDepth);
    m_commandBuffer.setViewport(0, 1, &viewport);

    CameraShaderBuffer cameraShaderData;
    cameraShaderData.InvView = ObjectManager::GetGlobalMatrix(camBuffer.TransformAddr);
    cameraShaderData.View = glm::inverse(cameraShaderData.InvView);
    cameraShaderData.Proj = camBuffer.ToProjection(viewSize);
    cameraShaderData.InvProj = glm::inverse(cameraShaderData.Proj);
    cameraShaderData.ViewProj = cameraShaderData.Proj * cameraShaderData.View;

    VulkanUniformBuffer* cameraUniformBuffer = m_gEngine->GetCameraUniformBuffer(m_bufferIndex);
    cameraUniformBuffer->SetData(m_engine->GetCurrentFrame(), &cameraShaderData);
}

void VulkanRenderCommand::Blit(const VulkanRenderTexture* a_src, const VulkanRenderTexture* a_dst)
{
    // TODO: Fix this temp fix for bliting
    // Probably better to copy or redraw when not flushed
    Flush();

    if (a_src == nullptr)
    {
        Logger::Error("IcarianEngine: Cannot Blit Swapchain as Source");

        return;
    }

    const glm::ivec2 swapSize = m_swapchain->GetSize();

    vk::Image dstImage = m_swapchain->GetTexture();
    vk::Offset3D dstOffset = vk::Offset3D((int32_t)swapSize.x, (int32_t)swapSize.y, 1);
    vk::ImageLayout dstLayout = m_swapchain->GetImageLayout();
    if (a_dst != nullptr)
    {
        dstImage = a_dst->GetTexture(0);
        dstOffset = vk::Offset3D((int32_t)a_dst->GetWidth(), (int32_t)a_dst->GetHeight(), 1);
        dstLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    }

    const vk::Image srcImage = a_src->GetTexture(0);

    const vk::Offset3D srcOffset = vk::Offset3D((int32_t)a_src->GetWidth(), (int32_t)a_src->GetHeight(), 1);

    constexpr vk::Offset3D ZeroOffset;

    constexpr vk::ImageSubresourceLayers ImageSubResource = vk::ImageSubresourceLayers
    (
        vk::ImageAspectFlagBits::eColor, 
        0,
        0, 
        1
    );

    const vk::ImageBlit blitRegion = vk::ImageBlit
    (
        ImageSubResource, 
        { ZeroOffset, srcOffset }, 
        ImageSubResource, 
        { ZeroOffset, dstOffset }
    );

    constexpr vk::ImageSubresourceRange SubResourceRange = vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);

    // const vk::ImageLayout srcLayout = a_src->GetImageLayout();
    const vk::ImageLayout srcLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

    const vk::ImageMemoryBarrier srcMemoryBarrier = vk::ImageMemoryBarrier
    (
        vk::AccessFlags(),
        vk::AccessFlagBits::eTransferRead,
        srcLayout,
        vk::ImageLayout::eTransferSrcOptimal,
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED,
        srcImage,
        SubResourceRange
    );
    const vk::ImageMemoryBarrier dstMemoryBarrier = vk::ImageMemoryBarrier
    (
        vk::AccessFlags(),
        vk::AccessFlagBits::eTransferWrite,
        dstLayout,
        vk::ImageLayout::eTransferDstOptimal,
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED,
        dstImage,
        SubResourceRange
    );

    m_commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer, vk::DependencyFlags(), 0, nullptr, 0, nullptr, 1, &srcMemoryBarrier);
    m_commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer, vk::DependencyFlags(), 0, nullptr, 0, nullptr, 1, &dstMemoryBarrier);

    m_commandBuffer.blitImage(a_src->GetTexture(0), vk::ImageLayout::eTransferSrcOptimal, dstImage, vk::ImageLayout::eTransferDstOptimal, 1, &blitRegion, vk::Filter::eNearest);

    const vk::ImageMemoryBarrier srcFinalMemoryBarrier = vk::ImageMemoryBarrier
    (
        vk::AccessFlagBits::eTransferRead,
        vk::AccessFlagBits::eMemoryRead,
        vk::ImageLayout::eTransferSrcOptimal,
        srcLayout,
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED,
        srcImage,
        SubResourceRange
    );
    const vk::ImageMemoryBarrier dstFinalMemoryBarrier = vk::ImageMemoryBarrier
    (
        vk::AccessFlagBits::eTransferWrite,
        vk::AccessFlagBits::eMemoryRead,
        vk::ImageLayout::eTransferDstOptimal,
        dstLayout,
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED,
        dstImage,
        SubResourceRange
    );

    m_commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer, vk::DependencyFlags(), 0, nullptr, 0, nullptr, 1, &srcFinalMemoryBarrier);
    m_commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer, vk::DependencyFlags(), 0, nullptr, 0, nullptr, 1, &dstFinalMemoryBarrier);
}

void VulkanRenderCommand::DrawMaterial()
{
    m_commandBuffer.draw(4, 1, 0, 0);
}
void VulkanRenderCommand::DrawModel(const glm::mat4& a_transform, uint32_t a_addr)
{
    const VulkanModel* model = m_gEngine->GetModel(a_addr);

    model->Bind(m_commandBuffer);

    const uint32_t indexCount = model->GetIndexCount();

    const VulkanPipeline* pipeline = GetPipeline();
    const VulkanShaderData* shaderData = pipeline->GetShaderData();
    shaderData->UpdateTransformBuffer(m_commandBuffer, a_transform);

    m_commandBuffer.drawIndexed(indexCount, 1, 0, 0, 0);
}
#endif