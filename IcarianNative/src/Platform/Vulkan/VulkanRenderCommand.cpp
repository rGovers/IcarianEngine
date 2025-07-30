// Icarian Engine - C# Game Engine
// 
// License at end of file.

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/VulkanRenderCommand.h"

#include "Core/Bitfield.h"
#include "Core/IcarianLambda.h"
#include "Core/ShaderBuffers.h"
#include "IcarianError.h"
#include "ObjectManager.h"
#include "Rendering/RenderEngine.h"
#include "Rendering/Vulkan/Shaders/VulkanComputeShader.h"
#include "Rendering/Vulkan/Shaders/VulkanMeshShader.h"
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

    m_renderBindMode = RenderTextureBindMode_Null;

    m_cameraAddr = a_camAddr;
    m_renderTexAddr = -1;
    m_materialAddr = -1;
    
    m_flags = 0;
}
VulkanRenderCommand::~VulkanRenderCommand()
{
    
}

void VulkanRenderCommand::SetRenderTextureCompute()
{
    constexpr vk::ImageSubresourceRange SubResourceRange = vk::ImageSubresourceRange
    (
        vk::ImageAspectFlagBits::eColor, 
        0, 
        1, 
        0, 
        1
    );

    if (!IISBITSET(m_flags, ComputeLayoutBit))
    {
        const VulkanRenderTexture* renderTexture = m_gEngine->GetRenderTexture(m_renderTexAddr);
        if (renderTexture != nullptr)
        {
            const uint32_t textureCount = renderTexture->GetTextureCount();

            for (uint32_t i = 0; i < textureCount; ++i)
            {
                const vk::Image image = renderTexture->GetTexture(i);

                const vk::ImageMemoryBarrier barrier = vk::ImageMemoryBarrier
                (
                    vk::AccessFlagBits::eColorAttachmentWrite,
                    vk::AccessFlagBits::eShaderRead,
                    vk::ImageLayout::eShaderReadOnlyOptimal,
                    vk::ImageLayout::eGeneral,
                    vk::QueueFamilyIgnored,
                    vk::QueueFamilyIgnored,
                    image,
                    SubResourceRange
                );

                m_commandBuffer.pipelineBarrier
                (
                    vk::PipelineStageFlagBits::eColorAttachmentOutput,
                    vk::PipelineStageFlagBits::eComputeShader,
                    vk::DependencyFlagBits::eByRegion,
                    0,
                    nullptr,
                    0,
                    nullptr,
                    1,
                    &barrier
                );
            }

            const bool hasDepthTexture = renderTexture->HasDepthTexture();
            if (hasDepthTexture)
            {
                constexpr vk::ImageSubresourceRange DepthSubResourceRange = vk::ImageSubresourceRange
                (
                    vk::ImageAspectFlagBits::eDepth, 
                    0, 
                    1, 
                    0, 
                    1
                );

                const vk::Image image = renderTexture->GetDepthTexture();

                const vk::ImageMemoryBarrier barrier = vk::ImageMemoryBarrier
                (
                    vk::AccessFlagBits::eDepthStencilAttachmentWrite,
                    vk::AccessFlagBits::eShaderRead,
                    vk::ImageLayout::eDepthStencilReadOnlyOptimal,
                    vk::ImageLayout::eGeneral,
                    vk::QueueFamilyIgnored,
                    vk::QueueFamilyIgnored,
                    image,
                    DepthSubResourceRange
                );

                m_commandBuffer.pipelineBarrier
                (
                    vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests,
                    vk::PipelineStageFlagBits::eComputeShader,
                    vk::DependencyFlagBits::eByRegion,
                    0,
                    nullptr,
                    0,
                    nullptr,
                    1,
                    &barrier
                );
            }
        }
        else
        {
            const vk::Image image = m_swapchain->GetTexture();
            const vk::ImageLayout swapLayout = m_swapchain->GetImageLayout();

            const vk::ImageMemoryBarrier barrier = vk::ImageMemoryBarrier
            (
                vk::AccessFlagBits::eColorAttachmentWrite,
                vk::AccessFlagBits::eShaderRead,
                swapLayout,
                vk::ImageLayout::eGeneral,
                vk::QueueFamilyIgnored,
                vk::QueueFamilyIgnored,
                image,
                SubResourceRange
            );
    
            m_commandBuffer.pipelineBarrier
            (
                vk::PipelineStageFlagBits::eColorAttachmentOutput, 
                vk::PipelineStageFlagBits::eComputeShader, 
                vk::DependencyFlagBits::eByRegion, 
                0, 
                nullptr,
                0,
                nullptr,
                1,
                &barrier
            );
        }

        ISETBIT(m_flags, ComputeLayoutBit);
    }
}
void VulkanRenderCommand::ClearRenderTextureCompute()
{
    constexpr vk::ImageSubresourceRange SubResourceRange = vk::ImageSubresourceRange
    (
        vk::ImageAspectFlagBits::eColor, 
        0, 
        1, 
        0, 
        1
    );

    if (IISBITSET(m_flags, ComputeLayoutBit))
    {
        const VulkanRenderTexture* renderTexture = m_gEngine->GetRenderTexture(m_renderTexAddr);
        if (renderTexture != nullptr)
        {
            const uint32_t textureCount = renderTexture->GetTextureCount();

            for (uint32_t i = 0; i < textureCount; ++i)
            {
                const vk::Image image = renderTexture->GetTexture(i);

                const vk::ImageMemoryBarrier barrier = vk::ImageMemoryBarrier
                (
                    vk::AccessFlagBits::eShaderWrite,
                    vk::AccessFlagBits::eShaderRead,
                    vk::ImageLayout::eGeneral,
                    vk::ImageLayout::eShaderReadOnlyOptimal,
                    vk::QueueFamilyIgnored,
                    vk::QueueFamilyIgnored,
                    image,
                    SubResourceRange
                );
        
                m_commandBuffer.pipelineBarrier
                (
                    vk::PipelineStageFlagBits::eComputeShader, 
                    vk::PipelineStageFlagBits::eFragmentShader, 
                    vk::DependencyFlagBits::eByRegion, 
                    0, 
                    nullptr,
                    0,
                    nullptr,
                    1,
                    &barrier
                );
            }

            if (renderTexture->HasDepthTexture())
            {
                constexpr vk::ImageSubresourceRange DepthSubResourceRange = vk::ImageSubresourceRange
                (
                    vk::ImageAspectFlagBits::eDepth, 
                    0, 
                    1, 
                    0, 
                    1
                );

                const vk::Image image = renderTexture->GetDepthTexture();

                const vk::ImageMemoryBarrier barrier = vk::ImageMemoryBarrier
                (
                    vk::AccessFlagBits::eShaderWrite,
                    vk::AccessFlagBits::eDepthStencilAttachmentRead,
                    vk::ImageLayout::eGeneral,
                    vk::ImageLayout::eDepthStencilReadOnlyOptimal,
                    vk::QueueFamilyIgnored,
                    vk::QueueFamilyIgnored,
                    image,
                    DepthSubResourceRange
                );

                m_commandBuffer.pipelineBarrier
                (
                    vk::PipelineStageFlagBits::eComputeShader,
                    vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests,
                    vk::DependencyFlagBits::eByRegion,
                    0,
                    nullptr,
                    0,
                    nullptr,
                    1,
                    &barrier
                );
            }
        }
        else
        {
            const vk::Image image = m_swapchain->GetTexture();
            const vk::ImageLayout swapLayout = m_swapchain->GetImageLayout();

            const vk::ImageMemoryBarrier barrier = vk::ImageMemoryBarrier
            (
                vk::AccessFlagBits::eShaderWrite,
                vk::AccessFlagBits::eShaderRead,
                vk::ImageLayout::eGeneral,
                swapLayout,
                vk::QueueFamilyIgnored,
                vk::QueueFamilyIgnored,
                image,
                SubResourceRange
            );
    
            m_commandBuffer.pipelineBarrier
            (
                vk::PipelineStageFlagBits::eComputeShader, 
                vk::PipelineStageFlagBits::eFragmentShader, 
                vk::DependencyFlagBits::eByRegion, 
                0, 
                nullptr,
                0,
                nullptr,
                1,
                &barrier
            );
        }

        ICLEARBIT(m_flags, ComputeLayoutBit);
    }
}

void VulkanRenderCommand::BindRenderTexturePass()
{
    const VulkanRenderTexture* renderTexture = m_gEngine->GetRenderTexture(m_renderTexAddr);

    const uint32_t screenWidth = ILAMBDA(
    {
        if (renderTexture != nullptr)
        {
            ILRETURN renderTexture->GetWidth();
        }

        ILRETURN m_swapchain->GetWidth();
    });
    const uint32_t screenHeight = ILAMBDA(
    {
        if (renderTexture != nullptr)
        {
            ILRETURN renderTexture->GetHeight();
        }

        ILRETURN m_swapchain->GetHeight();
    });

    if (renderTexture == nullptr)
    {
        constexpr vk::ClearValue ClearColor = vk::ClearValue(vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f));

        const uint32_t imageIndex = m_engine->GetImageIndex();

        const vk::Rect2D rect = vk::Rect2D({ 0, 0 }, { screenWidth, screenHeight });
        const vk::RenderPass renderPass = m_swapchain->GetRenderPass();
        const vk::Framebuffer framebuffer = m_swapchain->GetFramebuffer(imageIndex);

        const vk::RenderPassBeginInfo renderPassInfo = vk::RenderPassBeginInfo
        (
            renderPass,
            framebuffer,
            rect,
            1,
            &ClearColor
        );
    
        m_commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
    }
    else
    {
        // TODO: There are edge cases that may get users and feels like bad design
        // Might switch to using no clear and doing a software clear in RenderCommands to make behaviour more predictable
        // Would not have this issue if we did not have to figure out if will be used in a Compute or Graphics context
        // Could also track more state but more surface area for bugs
        const vk::RenderPass renderPass = ILAMBDA
        (
            switch (m_renderBindMode) 
            {
            case RenderTextureBindMode_Clear:
            {
                ILRETURN renderTexture->GetRenderPass();
            }
            case RenderTextureBindMode_ClearColor:
            {
                ILRETURN renderTexture->GetRenderPassColorClear();
            }
            case RenderTextureBindMode_NoClear:
            {
                ILRETURN renderTexture->GetRenderPassNoClear();
            }
            default:
            {
                break;
            }
            }

            IERROR("Invalid RenderTextureBindMode");

            ILRETURN vk::RenderPass(nullptr);
        );

        const vk::Rect2D rect = vk::Rect2D({ 0, 0 }, { screenWidth, screenHeight });
        const vk::Framebuffer framebuffer = renderTexture->GetFramebuffer();

        const uint32_t clearCount = renderTexture->GetTotalTextureCount();
        const vk::ClearValue* clearValues = renderTexture->GetClearValues();

        const vk::RenderPassBeginInfo renderPassInfo = vk::RenderPassBeginInfo
        (
            renderPass,
            framebuffer,
            rect,
            clearCount,
            clearValues
        );

        m_commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
    }

    if (m_cameraAddr != uint32_t(-1))
    {
        const CameraBuffer camBuffer = m_gEngine->GetCameraBuffer(m_cameraAddr);

        const uint32_t screenWidth = ILAMBDA(
        {
            if (renderTexture != nullptr)
            {
                ILRETURN renderTexture->GetWidth();
            }

            ILRETURN m_swapchain->GetWidth();
        });
        const uint32_t screenHeight = ILAMBDA(
        {
            if (renderTexture != nullptr)
            {
                ILRETURN renderTexture->GetHeight();
            }

            ILRETURN m_swapchain->GetHeight();
        });

        const glm::vec2 screenSize = glm::vec2((float)screenWidth, (float)screenHeight);
        const glm::vec2 viewPos = camBuffer.View.Position * screenSize;
        const glm::vec2 viewSize = camBuffer.View.Size * screenSize;

        const vk::Rect2D scissor = vk::Rect2D({ (int32_t)viewPos.x, (int32_t)viewPos.y }, { (uint32_t)viewSize.x, (uint32_t)viewSize.y });
        m_commandBuffer.setScissor(0, 1, &scissor);

        const vk::Viewport viewport = vk::Viewport(viewPos.x, viewPos.y, viewSize.x, viewSize.y, camBuffer.View.MinDepth, camBuffer.View.MaxDepth);
        m_commandBuffer.setViewport(0, 1, &viewport);
    }
}

void VulkanRenderCommand::BindResources()
{
    IVERIFY(m_materialAddr != uint32_t(-1));

    const VulkanPipeline* pipeline = m_gEngine->GetPipeline(m_renderTexAddr, m_materialAddr);
    const VulkanShaderData* shaderData = pipeline->GetShaderData();
    const e_MaterialMode materialMode = shaderData->GetMaterialMode();
    const VulkanRenderTexture* renderTexture = m_gEngine->GetRenderTexture(m_renderTexAddr);

    const bool meshEnabled = m_engine->IsMeshEnabled();

    const bool isEmulatedMesh = materialMode == MaterialMode_BaseMesh && !meshEnabled;
    const bool isCompute = materialMode == MaterialMode_Compute;

    switch (materialMode)
    {
    case MaterialMode_BaseVertex:
    {
        if (IISBITSET(m_flags, ComputeLayoutBit))
        {
            ClearRenderTextureCompute();
        }

        if (!IISBITSET(m_flags, RenderTextureBoundBit))
        {
            BindRenderTexturePass();

            ISETBIT(m_flags, RenderTextureBoundBit);
        }

        break;
    }
    case MaterialMode_BaseMesh:
    {
        if (IISBITSET(m_flags, ComputeLayoutBit))
        {
            ClearRenderTextureCompute();
        }

        if (!isEmulatedMesh)
        {
            if (!IISBITSET(m_flags, RenderTextureBoundBit))
            {
                BindRenderTexturePass();

                ISETBIT(m_flags, RenderTextureBoundBit);
            }
        }
        else
        {
            if (IISBITSET(m_flags, RenderTextureBoundBit))
            {
                // We are in emulated mode and Vulkan is annoying as we cannot run Compute in a Render Pass
                // Because of this we will bind it as needed
                // Yes this is a lot of over head and state changes however Vulkan does not give us much choice
                // That went out the window when we had to do 1-2 dispatches and 1 draw call to achieve the same functionality
                m_commandBuffer.endRenderPass();

                ICLEARBIT(m_flags, RenderTextureBoundBit);
            }
        }

        break;
    }
    case MaterialMode_Compute:
    {
        if (IISBITSET(m_flags, RenderTextureBoundBit))
        {
            m_commandBuffer.endRenderPass();

            ICLEARBIT(m_flags, RenderTextureBoundBit);
        }

        if (!IISBITSET(m_flags, ComputeLayoutBit))
        {
            SetRenderTextureCompute();
        }

        break;
    }
    default:
    {
        IERROR("Invalid material mode");

        break;
    }
    }

    if (m_cameraAddr != uint32_t(-1))
    {
        const CameraBuffer camBuffer = m_gEngine->GetCameraBuffer(m_cameraAddr);

        const uint32_t screenWidth = ILAMBDA(
        {
            if (renderTexture != nullptr)
            {
                ILRETURN renderTexture->GetWidth();
            }

            ILRETURN m_swapchain->GetWidth();
        });
        const uint32_t screenHeight = ILAMBDA(
        {
            if (renderTexture != nullptr)
            {
                ILRETURN renderTexture->GetHeight();
            }

            ILRETURN m_swapchain->GetHeight();
        });

        const glm::vec2 screenSize = glm::vec2((float)screenWidth, (float)screenHeight);
        const glm::vec2 viewSize = camBuffer.View.Size * screenSize;

        const glm::mat4 mat = ObjectManager::GetGlobalMatrix(camBuffer.TransformAddr);

        const IcarianCore::ShaderCameraBuffer cameraShaderData =
        {
            .View = glm::inverse(mat),
            .Proj = camBuffer.ToProjection(viewSize),
            .InvView = mat,
            .InvProj = glm::inverse(cameraShaderData.Proj),
            .ViewProj = cameraShaderData.Proj * cameraShaderData.View
        };

        const uint32_t curFrame = m_engine->GetCurrentFrame();

        VulkanUniformBuffer* cameraUniformBuffer = m_gEngine->GetCameraUniformBuffer(m_bufferIndex);
        cameraUniformBuffer->SetData(curFrame, &cameraShaderData);
    }

    if (!IISBITSET(m_flags, MaterialBoundBit))
    {
        RENDERSCRATCHFRAME;

        const uint32_t currentFrame = m_engine->GetCurrentFrame();

        ShaderBufferInput camInput;
        if (shaderData->GetShaderBufferInput(ShaderBufferType_CameraBuffer, &camInput))
        {
            const VulkanUniformBuffer* camBuffer = m_gEngine->GetCameraUniformBuffer(m_bufferIndex);

            shaderData->PushUniformBuffer(m_commandBuffer, camInput.Slot, camBuffer, currentFrame);
        }

        ShaderBufferInput timeInput;
        if (shaderData->GetShaderBufferInput(ShaderBufferType_TimeBuffer, &timeInput))
        {
            const VulkanUniformBuffer* timeBuffer = m_gEngine->GetTimeUniformBuffer();

            shaderData->PushUniformBuffer(m_commandBuffer, timeInput.Slot, timeBuffer, currentFrame);
        }

        if (isCompute)
        {
            const Array<ShaderBufferInput, RenderScratchAlloc> bufferInputs = shaderData->GetShaderBufferInputs(ShaderBufferType_BufferTexture);
            for (const ShaderBufferInput s : bufferInputs) 
            {
                shaderData->PushComputeBufferTexture(m_commandBuffer, s.Slot, renderTexture, currentFrame);
            }

            ISETBIT(m_flags, ComputeResourceBit);
        }

        pipeline->Bind(currentFrame, m_commandBuffer);

        ISETBIT(m_flags, MaterialBoundBit);
    }
}

void VulkanRenderCommand::Flush()
{
    ClearRenderTextureCompute();

    if (m_materialAddr != uint32_t(-1) && IISBITSET(m_flags, ComputeResourceBit))
    {
        const RenderProgram program = m_gEngine->GetRenderProgram(m_materialAddr);
        IVERIFY(program.Data != nullptr);

        if (program.MaterialMode == MaterialMode_Compute)
        {
            const VulkanShaderData* data = (VulkanShaderData*)program.Data;

            const uint32_t frame = m_engine->GetCurrentFlightFrame();
            data->UnbindCompute(frame, m_commandBuffer);
        }

        ICLEARBIT(m_flags, ComputeResourceBit);
    }

    if (IISBITSET(m_flags, RenderTextureBoundBit))
    {
        m_commandBuffer.endRenderPass();

        ICLEARBIT(m_flags, RenderTextureBoundBit);
    }

    ICLEARBIT(m_flags, MaterialBoundBit);

    m_renderTexAddr = -1;
    m_materialAddr = -1;
}

VulkanRenderTexture* VulkanRenderCommand::GetRenderTexture() const
{
    return m_gEngine->GetRenderTexture(m_renderTexAddr);
}
VulkanPipeline* VulkanRenderCommand::GetPipeline() const
{
    if (m_materialAddr == uint32_t(-1))
    {
        return nullptr;
    }

    return m_gEngine->GetPipeline(m_renderTexAddr, m_materialAddr);
}

VulkanPipeline* VulkanRenderCommand::BindMaterial(uint32_t a_materialAddr, bool a_immediate)
{
    if (m_materialAddr != a_materialAddr)
    {
        ICLEARBIT(m_flags, MaterialBoundBit);
    }

    if (m_materialAddr != uint32_t(-1) && IISBITSET(m_flags, ComputeResourceBit))
    {
        const RenderProgram program = m_gEngine->GetRenderProgram(m_materialAddr);
        IVERIFY(program.Data != nullptr);

        if (program.MaterialMode == MaterialMode_Compute)
        {
            const VulkanShaderData* data = (VulkanShaderData*)program.Data;

            const uint32_t frame = m_engine->GetCurrentFlightFrame();
            data->UnbindCompute(frame, m_commandBuffer);
        }

        ICLEARBIT(m_flags, ComputeResourceBit);
    }

    m_materialAddr = a_materialAddr;

    if (m_materialAddr == uint32_t(-1))
    {
        ICLEARBIT(m_flags, MaterialBoundBit);

        return nullptr;
    }

    if (a_immediate)
    {
        BindResources();
    }

    return m_gEngine->GetPipeline(m_renderTexAddr, m_materialAddr);
}

void VulkanRenderCommand::PushTexture(uint32_t a_slot, const TextureSamplerBuffer& a_sampler)
{
    IVERIFY(m_materialAddr != uint32_t(-1));

    BindResources();

    const RenderProgram program = m_gEngine->GetRenderProgram(m_materialAddr);
    IVERIFY(program.Data != nullptr);
    VulkanShaderData* data = (VulkanShaderData*)program.Data;

    data->PushTexture(m_commandBuffer, a_slot, a_sampler, m_engine->GetCurrentFrame());
}
void VulkanRenderCommand::PushLight(uint32_t a_slot, e_LightType a_lightType, uint32_t a_lightAddr)
{
    IVERIFY(m_materialAddr != uint32_t(-1));

    BindResources();

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
void VulkanRenderCommand::PushLightSplits(uint32_t a_slot, const LightShadowSplit* a_splits, uint32_t a_splitCount)
{
    RENDERSCRATCHFRAME;

    BindResources();

    const RenderProgram program = m_gEngine->GetRenderProgram(m_materialAddr);
    IVERIFY(program.Data != nullptr);
    VulkanShaderData* data = (VulkanShaderData*)program.Data;

    IcarianCore::ShaderShadowLightBuffer* shadowLightBuffer = RenderScratchAlloc::TAllocate<IcarianCore::ShaderShadowLightBuffer>(a_splitCount);

    for (uint32_t i = 0; i < a_splitCount; ++i)
    {
        shadowLightBuffer[i].LVP = a_splits[i].LVP;
        shadowLightBuffer[i].Split = a_splits[i].Split;
    }

    VulkanShaderStorageObject* storage = RenderScratchAlloc::TAllocate<VulkanShaderStorageObject>();
    new (storage) VulkanShaderStorageObject(m_engine, sizeof(IcarianCore::ShaderShadowLightBuffer) * a_splitCount, a_splitCount, shadowLightBuffer);
    IDEFER(storage->~VulkanShaderStorageObject());

    data->PushShaderStorageObject(m_commandBuffer, a_slot, storage, m_engine->GetCurrentFrame());
}
void VulkanRenderCommand::PushShadowTextureArray(uint32_t a_slot, uint32_t a_dirLightAddr)
{
    RENDERSCRATCHFRAME;

    BindResources();

    const RenderProgram program = m_gEngine->GetRenderProgram(m_materialAddr);
    IVERIFY(program.Data != nullptr);
    VulkanShaderData* data = (VulkanShaderData*)program.Data;

    const DirectionalLightBuffer dirLight = m_gEngine->GetDirectionalLight(a_dirLightAddr);
    IVERIFY(dirLight.Data != nullptr);

    const VulkanLightBuffer* lightBuffer = (VulkanLightBuffer*)dirLight.Data;
    const uint32_t count = lightBuffer->LightRenderTextureCount;

    TextureSamplerBuffer* buffer = RenderScratchAlloc::TAllocate<TextureSamplerBuffer>(count);
    IDEFER(
    for (uint32_t i = 0; i < count; ++i)
    {
        ((VulkanTextureSampler*)buffer[i].Data)->~VulkanTextureSampler();
    });

    for (uint32_t i = 0; i < count; ++i)
    {
        const uint32_t renderTex = lightBuffer->LightRenderTextures[i];

        VulkanTextureSampler* sampler = RenderScratchAlloc::TAllocate<VulkanTextureSampler>();

        buffer[i].Addr = renderTex;
        buffer[i].Slot = 0;
        buffer[i].TextureMode = TextureMode_DepthRenderTexture;
        buffer[i].FilterMode = TextureFilter_Linear;
        buffer[i].AddressMode = TextureAddress_ClampToEdge;
        buffer[i].Data = sampler;

        const VulkanTextureSamplerBuilder builder =
        {
            .Engine = m_engine,
            .Sampler = buffer[i]
        };

        VulkanTextureSampler::GenerateFromBuffer(sampler, builder);
    }

    data->PushTextures(m_commandBuffer, a_slot, buffer, count, m_engine->GetCurrentFrame());
}

void VulkanRenderCommand::BindRenderTexture(uint32_t a_renderTexAddr, e_RenderTextureBindMode a_bindMode)
{
    if (m_renderTexAddr != a_renderTexAddr)
    {
        ClearRenderTextureCompute();
    }

    if (IISBITSET(m_flags, RenderTextureBoundBit))
    {
        m_commandBuffer.endRenderPass();

        ICLEARBIT(m_flags, RenderTextureBoundBit);
    }

    m_renderBindMode = a_bindMode;
    m_renderTexAddr = a_renderTexAddr;
}

void VulkanRenderCommand::Blit(const VulkanRenderTexture* a_src, const VulkanRenderTexture* a_dst)
{
    Blit(a_src, 0, a_dst);
}
void VulkanRenderCommand::Blit(const VulkanRenderTexture* a_src, uint32_t a_index, const VulkanRenderTexture* a_dst)
{
    if (a_src == nullptr)
    {
        IERROR("Cannot blit Swapchain as source");

        return;
    }

    if (IISBITSET(m_flags, RenderTextureBoundBit))
    {
        m_commandBuffer.endRenderPass();

        ICLEARBIT(m_flags, RenderTextureBoundBit);
    }

    ICLEARBIT(m_flags, MaterialBoundBit);

    const uint32_t width = ILAMBDA(
    {
        if (a_dst != nullptr)
        {
            ILRETURN a_dst->GetWidth();
        }

        ILRETURN m_swapchain->GetWidth();
    });
    const uint32_t height = ILAMBDA(
    {
        if (a_dst != nullptr)
        {
            ILRETURN a_dst->GetHeight();
        }

        ILRETURN m_swapchain->GetHeight();
    });

    const vk::Image dstImage = ILAMBDA(
    {
        if (a_dst != nullptr)
        {
            ILRETURN a_dst->GetTexture(0);
        }

        ILRETURN m_swapchain->GetTexture();
    });

    const vk::ImageLayout dstLayout = ILAMBDA(
    {
        if (a_dst != nullptr)
        {
            ILRETURN vk::ImageLayout::eShaderReadOnlyOptimal;
        }

        ILRETURN m_swapchain->GetImageLayout();
    });
    
    const vk::Offset3D dstOffset = vk::Offset3D((int32_t)width, (int32_t)height, 1);

    const vk::Image srcImage = a_src->GetTexture(a_index);

    const uint32_t srcWidth = a_src->GetWidth();
    const uint32_t srcHeight = a_src->GetHeight();
    const vk::Offset3D srcOffset = vk::Offset3D((int32_t)srcWidth, (int32_t)srcHeight, 1);

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
        { },
        vk::AccessFlagBits::eTransferRead,
        srcLayout,
        vk::ImageLayout::eTransferSrcOptimal,
        vk::QueueFamilyIgnored,
        vk::QueueFamilyIgnored,
        srcImage,
        SubResourceRange
    );
    const vk::ImageMemoryBarrier dstMemoryBarrier = vk::ImageMemoryBarrier
    (
        { },
        vk::AccessFlagBits::eTransferWrite,
        dstLayout,
        vk::ImageLayout::eTransferDstOptimal,
        vk::QueueFamilyIgnored,
        vk::QueueFamilyIgnored,
        dstImage,
        SubResourceRange
    );

    m_commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer, vk::DependencyFlags(), 0, nullptr, 0, nullptr, 1, &srcMemoryBarrier);
    m_commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer, vk::DependencyFlags(), 0, nullptr, 0, nullptr, 1, &dstMemoryBarrier);

    m_commandBuffer.blitImage(srcImage, vk::ImageLayout::eTransferSrcOptimal, dstImage, vk::ImageLayout::eTransferDstOptimal, 1, &blitRegion, vk::Filter::eLinear);

    const vk::ImageMemoryBarrier srcFinalMemoryBarrier = vk::ImageMemoryBarrier
    (
        vk::AccessFlagBits::eTransferRead,
        vk::AccessFlagBits::eShaderRead,
        vk::ImageLayout::eTransferSrcOptimal,
        srcLayout,
        vk::QueueFamilyIgnored,
        vk::QueueFamilyIgnored,
        srcImage,
        SubResourceRange
    );
    const vk::ImageMemoryBarrier dstFinalMemoryBarrier = vk::ImageMemoryBarrier
    (
        vk::AccessFlagBits::eTransferWrite,
        vk::AccessFlagBits::eShaderRead,
        vk::ImageLayout::eTransferDstOptimal,
        dstLayout,
        vk::QueueFamilyIgnored,
        vk::QueueFamilyIgnored,
        dstImage,
        SubResourceRange
    );

    m_commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, vk::DependencyFlags(), 0, nullptr, 0, nullptr, 1, &srcFinalMemoryBarrier);
    m_commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, vk::DependencyFlags(), 0, nullptr, 0, nullptr, 1, &dstFinalMemoryBarrier);
}

void VulkanRenderCommand::DrawMaterial()
{
    if (m_materialAddr == uint32_t(-1))
    {
        IERROR("Drawing Material with no Material");

        return;
    }

    BindResources();

    const RenderProgram program = m_gEngine->GetRenderProgram(m_materialAddr);
    IVERIFY(program.Data != nullptr);

    switch (program.MaterialMode) 
    {
    case MaterialMode_BaseVertex:
    {
        // TODO: Probably going to deprecate this down the line but good for now
        m_commandBuffer.draw(4, 1, 0, 0);

        break;
    }
    case MaterialMode_Compute:
    {
        VulkanComputeShader* shader = m_gEngine->GetComputeShader(program.ExtraShader);

        const VulkanRenderTexture* renderTexture = m_gEngine->GetRenderTexture(m_renderTexAddr);

        const uint32_t screenWidth = ILAMBDA(
        {
            if (renderTexture != nullptr)
            {
                ILRETURN renderTexture->GetWidth();
            }

            ILRETURN m_swapchain->GetWidth();
        });
        const uint32_t screenHeight = ILAMBDA(
        {
            if (renderTexture != nullptr)
            {
                ILRETURN renderTexture->GetHeight();
            }

            ILRETURN m_swapchain->GetHeight();
        });

        const uint32_t groupX = shader->GetWorkgroupX();
        const uint32_t groupY = shader->GetWorkgroupY();

        const uint32_t dispatchX = screenWidth / groupX + 1;
        const uint32_t dispatchY = screenHeight / groupY + 1;

        m_commandBuffer.dispatch(dispatchX, dispatchY, 1);

        break;
    }
    default:
    {
        IERROR("Draw Material with bad Material Mode");

        break;
    }
    }
}
void VulkanRenderCommand::DrawModel(const glm::mat4& a_transform, uint32_t a_modelAddr)
{
    if (m_materialAddr == uint32_t(-1))
    {
        IERROR("Drawing Model with no Material");

        return;
    }

    BindResources();

    const VulkanModel* model = m_gEngine->GetModel(a_modelAddr);
    model->Bind(m_commandBuffer);

    const uint32_t indexCount = model->GetIndexCount();

    const VulkanPipeline* pipeline = GetPipeline();
    IVERIFY(pipeline != nullptr);
    const VulkanShaderData* shaderData = pipeline->GetShaderData();
    const e_MaterialMode materialMode = shaderData->GetMaterialMode();
    if (materialMode != MaterialMode_BaseVertex)
    {
        IERROR("Drawing non vertex Material with DrawModel");

        return;
    }

    ShaderBufferInput input;
    if (shaderData->GetShaderBufferInput(ShaderBufferType_SSModelBuffer, &input))
    {
        // TODO: I can probably batch these calls need to investigate if it is even worthwhile
        RENDERSCRATCHFRAME;

        const IcarianCore::ShaderModelBuffer buffer = 
        {
            .Model = a_transform,
            .InvModel = glm::inverse(a_transform)
        };

        void* storagePtr = RenderScratchAlloc::TAllocate<VulkanShaderStorageObject>();
        const VulkanShaderStorageObject* storage = new (storagePtr) VulkanShaderStorageObject(m_engine, sizeof(IcarianCore::ShaderModelBuffer), 1, &buffer);
        IDEFER(storage->~VulkanShaderStorageObject());

        const uint32_t frameIndex = m_engine->GetCurrentFlightFrame();

        shaderData->PushShaderStorageObject(m_commandBuffer, input.Slot, storage, frameIndex);
    }
    else
    {
        shaderData->UpdateTransformBuffer(m_commandBuffer, a_transform);
    }

    m_commandBuffer.drawIndexed(indexCount, 1, 0, 0, 0);
}
void VulkanRenderCommand::DrawMesh(const glm::mat4& a_transform, uint32_t a_meshAddr, uint32_t a_indexCount)
{
    if (m_materialAddr == uint32_t(-1))
    {
        IERROR("Drawing Mesh with no Material");

        return;
    }

    BindResources();

    const VulkanPipeline* pipeline = GetPipeline();
    IVERIFY(pipeline != nullptr);
    const VulkanShaderData* shaderData = pipeline->GetShaderData();
    const e_MaterialMode materialMode = shaderData->GetMaterialMode();
    if (materialMode != MaterialMode_BaseMesh)
    {
        IERROR("Drawing non Mesh Material with DrawMesh");

        return;
    }

    // TODO: Implement Mesh binding support

    ShaderBufferInput input;
    if (shaderData->GetShaderBufferInput(ShaderBufferType_SSModelBuffer, &input))
    {
        RENDERSCRATCHFRAME;

        const IcarianCore::ShaderModelBuffer buffer =
        {
            .Model = a_transform,
            .InvModel = glm::inverse(a_transform)
        };

        void* storagePtr = RenderScratchAlloc::TAllocate<VulkanShaderStorageObject>();
        const VulkanShaderStorageObject* storage = new (storagePtr) VulkanShaderStorageObject(m_engine, sizeof(IcarianCore::ShaderModelBuffer), 1, &buffer);
        IDEFER(storage->~VulkanShaderStorageObject());

        const uint32_t frameIndex = m_engine->GetCurrentFlightFrame();

        shaderData->PushShaderStorageObject(m_commandBuffer, input.Slot, storage, frameIndex);
    }
    else
    {
        shaderData->UpdateTransformBuffer(m_commandBuffer, a_transform);
    }

    const bool isMeshEnabled = m_engine->IsMeshEnabled();
    if (isMeshEnabled)
    {
        // We do have native mesh support so we can just directly dispatch the call no questions asked
        m_commandBuffer.drawMeshTasksEXT(a_indexCount, 1, 1);
    }
    else
    {
        const RenderProgram program = m_gEngine->GetRenderProgram(m_materialAddr);
        IVERIFY(program.Data != nullptr);

        // We do not have native mesh support so we need to emulate the functionality
        // Note Vulkan is annoying as you cannot dispatch Compute in an active RenderPass
        // To get around this when RenderCommands are in emulated mode we have to manually bind the RenderPass
        // when it is a Mesh Material
        const VulkanMeshEmulationData* emulationData = m_gEngine->GetMeshEmulationData();
        IVERIFY(emulationData != nullptr);

        const bool hasTask = program.ExtraShader != uint32_t(-1);

        // Start the process of zero filling the old buffer
        // We do not need a barrier yet will insert a barrier just before we start writing to it
        // Starting the process now as we know we will need to clear it as we are drawing now
        // If the GPU decides to actually start filling the buffer with zeros before the barrier is a seperate matter
        // We have presented the opportunity to fill the buffer in the backgroud so we have done our job
        m_commandBuffer.fillBuffer(emulationData->IndexBuffer, 0, vk::WholeSize, 0);

        // const VulkanMeshShader* meshShader = m_gEngine->GetMeshShader(program.VertexShader);

        // TODO: We need to properly handle Task Shaders
        // This will likely end up a Dispatch->IndirectMultiDispatch still figuring out details however
        if (hasTask)
        {

        }

        const vk::BufferMemoryBarrier barrier = vk::BufferMemoryBarrier
        (
            vk::AccessFlagBits::eTransferWrite,
            vk::AccessFlagBits::eShaderWrite,
            vk::QueueFamilyIgnored,
            vk::QueueFamilyIgnored,
            emulationData->IndexBuffer,
            0,
            vk::WholeSize
        );

        m_commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eComputeShader, { }, 0, nullptr, 1, &barrier, 0, nullptr);

        if (hasTask)
        {
            // TODO: Implement me!~
        }
        else
        {
            // const uint32_t computeAddr =
            // m_commandBuffer.dispatch()
        }
    }
}

void VulkanRenderCommand::MarkerStart(const std::string_view& a_name)
{
#ifdef ICARIANNATIVE_ENABLE_MARKERS
    const bool markersEnabled = m_engine->IsExtensionEnabled(VK_EXT_DEBUG_MARKER_EXTENSION_NAME); 
    if (markersEnabled)
    { 
        const vk::DebugMarkerMarkerInfoEXT markerInfo = vk::DebugMarkerMarkerInfoEXT 
        ( 
            a_name.data()
        ); 

        m_commandBuffer.debugMarkerBeginEXT(markerInfo); 
    }
#endif
}
void VulkanRenderCommand::MarkerEnd()
{
#ifdef ICARIANNATIVE_ENABLE_MARKERS
    const bool markersEnabled = m_engine->IsExtensionEnabled(VK_EXT_DEBUG_MARKER_EXTENSION_NAME); 
    if (markersEnabled)
    {
        m_commandBuffer.debugMarkerEndEXT(); 
    }
#endif
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
