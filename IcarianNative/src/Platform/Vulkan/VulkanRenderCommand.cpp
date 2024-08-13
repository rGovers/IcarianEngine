// Icarian Engine - C# Game Engine
// 
// License at end of file.

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/VulkanRenderCommand.h"

#include "Core/IcarianAssert.h"
#include "Core/ShaderBuffers.h"
#include "ObjectManager.h"
#include "Rendering/RenderEngine.h"
#include "Rendering/Vulkan/VulkanGraphicsEngine.h"
#include "Rendering/Vulkan/VulkanLightBuffer.h"
#include "Rendering/Vulkan/VulkanModel.h"
#include "Rendering/Vulkan/VulkanPipeline.h"
#include "Rendering/Vulkan/VulkanPushPool.h"
#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"
#include "Rendering/Vulkan/VulkanRenderTexture.h"
#include "Rendering/Vulkan/VulkanShaderData.h"
#include "Rendering/Vulkan/VulkanShaderStorageObject.h"
#include "Rendering/Vulkan/VulkanSwapchain.h"
#include "Rendering/Vulkan/VulkanTextureSampler.h"
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
    ITOGGLEBIT(a_value, m_flags, FlushedBit);
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

            shaderData->PushUniformBuffer(m_commandBuffer, camInput.Slot, camBuffer, currentFrame);
        }

        ShaderBufferInput timeInput;
        if (shaderData->GetShaderBufferInput(ShaderBufferType_TimeBuffer, &timeInput))
        {
            VulkanUniformBuffer* timeBuffer = m_gEngine->GetTimeUniformBuffer();

            shaderData->PushUniformBuffer(m_commandBuffer, timeInput.Slot, timeBuffer, currentFrame);
        }

        pipeline->Bind(currentFrame, m_commandBuffer);
    }

    return pipeline;
}

void VulkanRenderCommand::PushTexture(uint32_t a_slot, const TextureSamplerBuffer& a_sampler) const
{
    IVERIFY(m_materialAddr != -1);

    const RenderProgram program = m_gEngine->GetRenderProgram(m_materialAddr);
    IVERIFY(program.Data != nullptr);
    VulkanShaderData* data = (VulkanShaderData*)program.Data;

    data->PushTexture(m_commandBuffer, a_slot, a_sampler, m_engine->GetCurrentFrame());
}
void VulkanRenderCommand::PushLight(uint32_t a_slot, e_LightType a_lightType, uint32_t a_lightAddr) const
{
    const RenderProgram program = m_gEngine->GetRenderProgram(m_materialAddr);
    IVERIFY(program.Data != nullptr);
    VulkanShaderData* data = (VulkanShaderData*)program.Data;

    VulkanPushPool* pushPool = m_engine->GetPushPool();

    const uint32_t index = m_engine->GetCurrentFrame();

    switch (a_lightType)
    {
    case LightType_Ambient:
    {
        const AmbientLightBuffer light = m_gEngine->GetAmbientLight(a_lightAddr);

        const IcarianCore::ShaderAmbientLightBuffer buffer =
        {
            .LightColor = glm::vec4(light.Color.xyz(), light.Intensity)
        };

        VulkanUniformBuffer* lightBuffer = pushPool->AllocateAmbientLightUniformBuffer();
        lightBuffer->SetData(index, &buffer);

        data->PushUniformBuffer(m_commandBuffer, a_slot, lightBuffer, index);

        break;
    }
    case LightType_Directional:
    {
        const DirectionalLightBuffer light = m_gEngine->GetDirectionalLight(a_lightAddr);

        const glm::mat4 tMat = ObjectManager::GetGlobalMatrix(light.TransformAddr);
        const glm::vec3 forward = glm::normalize(tMat[2].xyz());

        const IcarianCore::ShaderDirectionalLightBuffer buffer = 
        {
            .LightDir = glm::vec4(forward, light.Intensity),
            .LightColor = light.Color
        };

        VulkanUniformBuffer* lightBuffer = pushPool->AllocateDirectionalLightUniformBuffer();
        lightBuffer->SetData(index, &buffer);

        data->PushUniformBuffer(m_commandBuffer, a_slot, lightBuffer, index);

        break;
    }
    case LightType_Point:
    {
        const PointLightBuffer light = m_gEngine->GetPointLight(a_lightAddr);

        const glm::mat4 tMat = ObjectManager::GetGlobalMatrix(light.TransformAddr);
        const glm::vec3 position = tMat[3].xyz();

        const IcarianCore::ShaderPointLightBuffer buffer = 
        {
            .LightPos = glm::vec4(position, light.Intensity),
            .LightColor = light.Color,
            .Radius = light.Radius
        };

        VulkanUniformBuffer* lightBuffer = pushPool->AllocatePointLightUniformBuffer();
        lightBuffer->SetData(index, &buffer);

        data->PushUniformBuffer(m_commandBuffer, a_slot, lightBuffer, index);

        break;
    }
    case LightType_Spot:
    {
        const SpotLightBuffer light = m_gEngine->GetSpotLight(a_lightAddr);

        const glm::mat4 tMat = ObjectManager::GetGlobalMatrix(light.TransformAddr);
        const glm::vec3 position = tMat[3].xyz();
        const glm::vec3 forward = glm::normalize(tMat[2].xyz());

        const IcarianCore::ShaderSpotLightBuffer buffer =
        {
            .LightPos = position,
            .LightDir = glm::vec4(forward, light.Intensity),
            .LightColor = light.Color,
            .CutoffAngle = glm::vec3(light.CutoffAngle, light.Radius)
        };

        VulkanUniformBuffer* lightBuffer = pushPool->AllocateSpotLightUniformBuffer();
        lightBuffer->SetData(index, &buffer);

        data->PushUniformBuffer(m_commandBuffer, a_slot, lightBuffer, index);

        break;
    }
    default:
    {
        IERROR("Invalid light type");

        break;
    }
    }
}
void VulkanRenderCommand::PushLightSplits(uint32_t a_slot, const LightShadowSplit* a_splits, uint32_t a_splitCount) const
{
    const RenderProgram program = m_gEngine->GetRenderProgram(m_materialAddr);
    IVERIFY(program.Data != nullptr);
    VulkanShaderData* data = (VulkanShaderData*)program.Data;

    VulkanPushPool* pushPool = m_engine->GetPushPool();

    IcarianCore::ShaderShadowLightBuffer* shadowLightBuffer = new IcarianCore::ShaderShadowLightBuffer[a_splitCount];
    IDEFER(delete[] shadowLightBuffer);

    for (uint32_t i = 0; i < a_splitCount; ++i)
    {
        shadowLightBuffer[i].LVP = a_splits[i].LVP;
        shadowLightBuffer[i].Split = a_splits[i].Split;
    }

    const VulkanShaderStorageObject* storage = new VulkanShaderStorageObject(m_engine, sizeof(IcarianCore::ShaderShadowLightBuffer) * a_splitCount, a_splitCount, shadowLightBuffer);
    IDEFER(delete storage);

    data->PushShaderStorageObject(m_commandBuffer, a_slot, storage, m_engine->GetCurrentFrame());
}
void VulkanRenderCommand::PushShadowTextureArray(uint32_t a_slot, uint32_t a_dirLightAddr) const
{
    const RenderProgram program = m_gEngine->GetRenderProgram(m_materialAddr);
    IVERIFY(program.Data != nullptr);
    VulkanShaderData* data = (VulkanShaderData*)program.Data;

    const DirectionalLightBuffer dirLight = m_gEngine->GetDirectionalLight(a_dirLightAddr);
    IVERIFY(dirLight.Data != nullptr);

    const VulkanLightBuffer* lightBuffer = (VulkanLightBuffer*)dirLight.Data;
    const uint32_t count = lightBuffer->LightRenderTextureCount;

    TextureSamplerBuffer* buffer = new TextureSamplerBuffer[count];
    IDEFER(
    {
        for (uint32_t i = 0; i < count; ++i)
        {
            delete (VulkanTextureSampler*)buffer[i].Data;
        }

        delete[] buffer;
    });

    for (uint32_t i = 0; i < count; ++i)
    {
        const uint32_t renderTex = lightBuffer->LightRenderTextures[i];

        buffer[i].Addr = renderTex;
        buffer[i].Slot = 0;
        buffer[i].TextureMode = TextureMode_DepthRenderTexture;
        buffer[i].FilterMode = TextureFilter_Linear;
        buffer[i].AddressMode = TextureAddress_ClampToEdge;
        buffer[i].Data = VulkanTextureSampler::GenerateFromBuffer(m_engine, m_gEngine, buffer[i]);
    }

    data->PushTextures(m_commandBuffer, a_slot, buffer, count, m_engine->GetCurrentFrame());
}

void VulkanRenderCommand::BindRenderTexture(uint32_t a_renderTexAddr, e_RenderTextureBindMode a_bindMode)
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

        vk::RenderPass renderPass;

        switch (a_bindMode)
        {
        case RenderTextureBindMode_Clear:
        {
            renderPass = renderTexture->GetRenderPass();

            break;
        }
        case RenderTextureBindMode_ClearColor:
        {
            renderPass = renderTexture->GetRenderPassColorClear();

            break;
        }
        case RenderTextureBindMode_NoClear:
        {
            renderPass = renderTexture->GetRenderPassNoClear();

            break;
        }
        default:
        {
            ICARIAN_ASSERT(0);
        }
        }

        const vk::RenderPassBeginInfo renderPassInfo = vk::RenderPassBeginInfo
        (
            renderPass,
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

    const glm::mat4 mat = ObjectManager::GetGlobalMatrix(camBuffer.TransformAddr);

    const IcarianCore::ShaderCameraBuffer cameraShaderData =
    {
        .View = glm::inverse(mat),
        .Proj = camBuffer.ToProjection(viewSize),
        .InvView = mat,
        .InvProj = glm::inverse(cameraShaderData.Proj),
        .ViewProj = cameraShaderData.Proj * cameraShaderData.View
    };

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
        IERROR("Cannot blit Swapchain as source");

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