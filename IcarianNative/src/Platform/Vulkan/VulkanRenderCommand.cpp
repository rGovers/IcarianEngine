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
#include "Rendering/Vulkan/VulkanMesh.h"
#include "Rendering/Vulkan/VulkanModel.h"
#include "Rendering/Vulkan/VulkanPipeline.h"
#include "Rendering/Vulkan/VulkanPushPool.h"
#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"
#include "Rendering/Vulkan/VulkanRenderProgramBlob.h"
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
                if (!IISBITSET(m_flags, RenderTextureFirstBindBit))
                {
                    ILRETURN renderTexture->GetRenderPassNoClear();
                }

                ILRETURN renderTexture->GetRenderPass();
            }
            case RenderTextureBindMode_ClearColor:
            {
                if (!IISBITSET(m_flags, RenderTextureFirstBindBit))
                {
                    ILRETURN renderTexture->GetRenderPassNoClear();
                }

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

        ICLEARBIT(m_flags, RenderTextureFirstBindBit);

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

bool VulkanRenderCommand::BindResources(Allocator* a_tempAllocator)
{
    if (m_materialAddr == uint32_t(-1))
    {
        return false;
    }

    const VulkanPipeline* pipeline = m_gEngine->GetPipeline(m_renderTexAddr, m_materialAddr);
    const VulkanShaderData* shaderData = pipeline->GetShaderData();
    const e_MaterialMode materialMode = shaderData->GetMaterialMode();
    const VulkanRenderTexture* renderTexture = m_gEngine->GetRenderTexture(m_renderTexAddr);

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

        if (!IISBITSET(m_flags, RenderTextureBoundBit))
        {
            BindRenderTexturePass();

            ISETBIT(m_flags, RenderTextureBoundBit);
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
        const uint32_t currentFrame = m_engine->GetCurrentFrame();

        if (!pipeline->Bind(currentFrame, m_commandBuffer))
        {
            return false;
        }

        ShaderBufferInput camInput;
        if (shaderData->GetShaderBufferInput(ShaderBufferType_CameraBuffer, &camInput))
        {
            const VulkanUniformBuffer* camBuffer = m_gEngine->GetCameraUniformBuffer(m_bufferIndex);

            shaderData->PushUniformBuffer(m_commandBuffer, camInput.RealSlot, camBuffer, currentFrame);
        }

        if (isCompute)
        {
            const Array<ShaderBufferInput> bufferInputs = shaderData->GetShaderBufferInputs
            (
                ShaderBufferType_BufferTexture,
                a_tempAllocator
            );

            for (const ShaderBufferInput s : bufferInputs) 
            {
                shaderData->PushComputeBufferTexture
                (
                    m_commandBuffer,
                    s.RealSlot,
                    renderTexture,
                    currentFrame
                );
            }
        }

        ISETBIT(m_flags, MaterialBoundBit);
    }

    return true;
}

void VulkanRenderCommand::Flush()
{
    ClearRenderTextureCompute();

    // TODO: This does not seem right need to adjust this
    if (m_materialAddr != uint32_t(-1) && IISBITSET(m_flags, MaterialBoundBit))
    {
        const RenderProgram program = m_gEngine->GetRenderProgram(m_materialAddr);
        IVERIFY(program.Data != nullptr);

        const VulkanRenderProgramBlob* blob = (VulkanRenderProgramBlob*)program.Data;
        const VulkanShaderData* data = blob->Base;
        if (data != nullptr)
        {
            const uint32_t frame = m_engine->GetCurrentFlightFrame();
            data->Unbind(frame, m_commandBuffer);
        }

        ICLEARBIT(m_flags, MaterialBoundBit);
    }

    if (IISBITSET(m_flags, RenderTextureBoundBit))
    {
        m_commandBuffer.endRenderPass();

        ICLEARBIT(m_flags, RenderTextureBoundBit);
    }

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

VulkanPipeline* VulkanRenderCommand::BindMaterial(uint32_t a_materialAddr, bool a_immediate, Allocator* a_tempAllocator)
{
    if (m_materialAddr != a_materialAddr)
    {
        ICLEARBIT(m_flags, MaterialBoundBit);
    }

    if (m_materialAddr != uint32_t(-1) && IISBITSET(m_flags, MaterialBoundBit))
    {
        const RenderProgram program = m_gEngine->GetRenderProgram(m_materialAddr);
        IVERIFY(program.Data != nullptr);

        const VulkanRenderProgramBlob* blob = (VulkanRenderProgramBlob*)program.Data;
        const VulkanShaderData* data = blob->Base;

        if (data != nullptr)
        {
            const uint32_t frame = m_engine->GetCurrentFlightFrame();

            data->Unbind(frame, m_commandBuffer);
        }
    }

    m_materialAddr = a_materialAddr;

    if (m_materialAddr == uint32_t(-1))
    {
        ICLEARBIT(m_flags, MaterialBoundBit);

        return nullptr;
    }

    if (a_immediate)
    {
        if (!BindResources(a_tempAllocator))
        {
            return nullptr;
        }
    }

    return m_gEngine->GetPipeline(m_renderTexAddr, m_materialAddr);
}

void VulkanRenderCommand::PushTexture(uint32_t a_slot, const TextureSamplerBuffer& a_sampler, Allocator* a_tempAllocator)
{
    IVERIFY(m_materialAddr != uint32_t(-1));

    if (!BindResources(a_tempAllocator))
    {
        IWARN("PushTexture failed to bind material");

        return;
    }

    const RenderProgram program = m_gEngine->GetRenderProgram(m_materialAddr);
    IVERIFY(program.Data != nullptr);

    const VulkanRenderProgramBlob* blob = (VulkanRenderProgramBlob*)program.Data;

    const uint32_t index = m_engine->GetCurrentFrame();

    if (blob->Base != nullptr)
    {
        blob->Base->PushTexture(m_commandBuffer, a_slot, a_sampler, index);
    }

    if (blob->Secondary != nullptr)
    {
        blob->Secondary->PushTexture(m_commandBuffer, a_slot, a_sampler, index);
    }

    if (blob->Tertiary != nullptr)
    {
        blob->Tertiary->PushTexture(m_commandBuffer, a_slot, a_sampler, index);
    }
}
void VulkanRenderCommand::PushLight(uint32_t a_slot, e_LightType a_lightType, uint32_t a_lightAddr, Allocator* a_tempAllocator)
{
    IVERIFY(m_materialAddr != uint32_t(-1));

    if (!BindResources(a_tempAllocator))
    {
        IWARN("PushLight failed to bind material");

        return;
    }

    const RenderProgram program = m_gEngine->GetRenderProgram(m_materialAddr);
    IVERIFY(program.Data != nullptr);

    VulkanPushPool* pushPool = m_engine->GetPushPool();

    const uint32_t index = m_engine->GetCurrentFrame();

    const VulkanRenderProgramBlob* blob = (VulkanRenderProgramBlob*)program.Data;

    const VulkanUniformBuffer* lightBuffer = ILAMBDA(
    {
        switch (a_lightType)
        {
        case LightType_Ambient:
        {
            const AmbientLightBuffer light = m_gEngine->GetAmbientLight(a_lightAddr);

            IcarianCore::ShaderAmbientLightBuffer buffer;
            buffer.LightColor = glm::vec4(light.Color.xyz(), light.Intensity);

            VulkanUniformBuffer* lightBuffer = pushPool->AllocateAmbientLightUniformBuffer();
            lightBuffer->SetData(index, &buffer);

            ILRETURN lightBuffer;
        }
        case LightType_Directional:
        {
            const DirectionalLightBuffer light = m_gEngine->GetDirectionalLight(a_lightAddr);

            const glm::mat4 tMat = ObjectManager::GetGlobalMatrix(light.TransformAddr);
            const glm::vec3 forward = glm::normalize(tMat[2].xyz());

            IcarianCore::ShaderDirectionalLightBuffer buffer;
            buffer.LightDir = glm::vec4(forward, light.Intensity);
            buffer.LightColor = light.Color;

            VulkanUniformBuffer* lightBuffer = pushPool->AllocateDirectionalLightUniformBuffer();
            lightBuffer->SetData(index, &buffer);

            ILRETURN lightBuffer;
        }
        case LightType_Point:
        {
            const PointLightBuffer light = m_gEngine->GetPointLight(a_lightAddr);

            const glm::mat4 tMat = ObjectManager::GetGlobalMatrix(light.TransformAddr);
            const glm::vec3 position = tMat[3].xyz();

            IcarianCore::ShaderPointLightBuffer buffer;
            buffer.LightPos = glm::vec4(position, light.Intensity);
            buffer.LightColor = light.Color;
            buffer.Radius = light.Radius;

            VulkanUniformBuffer* lightBuffer = pushPool->AllocatePointLightUniformBuffer();
            lightBuffer->SetData(index, &buffer);

            ILRETURN lightBuffer;
        }
        case LightType_Spot:
        {
            const SpotLightBuffer light = m_gEngine->GetSpotLight(a_lightAddr);

            const glm::mat4 tMat = ObjectManager::GetGlobalMatrix(light.TransformAddr);
            const glm::vec3 position = tMat[3].xyz();
            const glm::vec3 forward = glm::normalize(tMat[2].xyz());

            IcarianCore::ShaderSpotLightBuffer buffer;
            buffer.LightPos = position;
            buffer.LightDir = glm::vec4(forward, light.Intensity);
            buffer.LightColor = light.Color;
            buffer.CutoffAngle = glm::vec3(light.CutoffAngle, light.Radius);

            VulkanUniformBuffer* lightBuffer = pushPool->AllocateSpotLightUniformBuffer();
            lightBuffer->SetData(index, &buffer);

            ILRETURN lightBuffer;
        }
        default:
        {
            break;
        }
        }

        ILRETURN (VulkanUniformBuffer*)nullptr;
    });

    IVERIFY(lightBuffer != nullptr);

    if (blob->Base != nullptr)
    {
        blob->Base->PushUniformBuffer(m_commandBuffer, a_slot, lightBuffer, index);
    }

    if (blob->Secondary != nullptr)
    {
        blob->Secondary->PushUniformBuffer(m_commandBuffer, a_slot, lightBuffer, index);
    }

    if (blob->Tertiary != nullptr)
    {
        blob->Tertiary->PushUniformBuffer(m_commandBuffer, a_slot, lightBuffer, index);
    }
}
void VulkanRenderCommand::PushLightSplits(uint32_t a_slot, const LightShadowSplit* a_splits, uint32_t a_splitCount, Allocator* a_tempAllocator)
{
    if (!BindResources(a_tempAllocator))
    {
        IWARN("PushLightSplits failed to bind material");

        return;
    }

    const RenderProgram program = m_gEngine->GetRenderProgram(m_materialAddr);
    IVERIFY(program.Data != nullptr);

    IcarianCore::ShaderShadowLightBuffer* shadowLightBuffer = ILAMBDA(
    {
        IcarianCore::ShaderShadowLightBuffer* vals = a_tempAllocator->TAllocate<IcarianCore::ShaderShadowLightBuffer>(a_splitCount);

        for (uint32_t i = 0; i < a_splitCount; ++i)
        {
            vals[i].LVP = a_splits[i].LVP;
            vals[i].Split = a_splits[i].Split;
        }

        ILRETURN vals;
    });
    IDEFER(a_tempAllocator->Free(shadowLightBuffer));

    VulkanShaderStorageObject* storage = a_tempAllocator->Create<VulkanShaderStorageObject>
    (
        m_engine,
        sizeof(IcarianCore::ShaderShadowLightBuffer) * a_splitCount,
        a_splitCount,
        shadowLightBuffer
    );
    IDEFER(a_tempAllocator->Destroy(storage));

    const uint32_t index = m_engine->GetCurrentFrame();

    const VulkanRenderProgramBlob* blob = (VulkanRenderProgramBlob*)program.Data;

    if (blob->Base != nullptr)
    {
        blob->Base->PushShaderStorageObject(m_commandBuffer, a_slot, storage, index);
    }

    if (blob->Secondary != nullptr)
    {
        blob->Secondary->PushShaderStorageObject(m_commandBuffer, a_slot, storage, index);
    }

    if (blob->Tertiary != nullptr)
    {
        blob->Tertiary->PushShaderStorageObject(m_commandBuffer, a_slot, storage, index);
    }
}
void VulkanRenderCommand::PushShadowTextureArray(uint32_t a_slot, uint32_t a_dirLightAddr, Allocator* a_tempAllocator)
{
    if (!BindResources(a_tempAllocator))
    {
        IWARN("PushShadowTextureArray failed to bind material");

        return;
    }

    const RenderProgram program = m_gEngine->GetRenderProgram(m_materialAddr);
    IVERIFY(program.Data != nullptr);

    const VulkanRenderProgramBlob* blob = (VulkanRenderProgramBlob*)program.Data;

    const DirectionalLightBuffer dirLight = m_gEngine->GetDirectionalLight(a_dirLightAddr);
    IVERIFY(dirLight.Data != nullptr);

    const VulkanLightBuffer* lightBuffer = (VulkanLightBuffer*)dirLight.Data;
    const uint32_t count = lightBuffer->LightRenderTextureCount;

    TextureSamplerBuffer* buffer = a_tempAllocator->TAllocate<TextureSamplerBuffer>(count);
    IDEFER(
    {
        for (uint32_t i = 0; i < count; ++i)
        {
            ((VulkanTextureSampler*)buffer[i].Data)->~VulkanTextureSampler();
        }

        a_tempAllocator->Free(buffer);
    });

    for (uint32_t i = 0; i < count; ++i)
    {
        const uint32_t renderTex = lightBuffer->LightRenderTextures[i];

        VulkanTextureSampler* sampler = a_tempAllocator->TAllocate<VulkanTextureSampler>();

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

    const uint32_t index = m_engine->GetCurrentFrame();

    if (blob->Base != nullptr)
    {
        blob->Base->PushTextures(m_commandBuffer, a_slot, buffer, count, index, a_tempAllocator);
    }

    if (blob->Secondary != nullptr)
    {
        blob->Secondary->PushTextures(m_commandBuffer, a_slot, buffer, count, index, a_tempAllocator);
    }

    if (blob->Tertiary != nullptr)
    {
        blob->Tertiary->PushTextures(m_commandBuffer, a_slot, buffer, count, index, a_tempAllocator);
    }
}

void VulkanRenderCommand::PushUserTexture(uint32_t a_slot, const TextureSamplerBuffer& a_sampler, Allocator* a_tempAllocator)
{
    IVERIFY(m_materialAddr != uint32_t(-1));

    if (!BindResources(a_tempAllocator))
    {
        IWARN("PushTexture failed to bind material");

        return;
    }

    const RenderProgram program = m_gEngine->GetRenderProgram(m_materialAddr);
    IVERIFY(program.Data != nullptr);

    const VulkanRenderProgramBlob* blob = (VulkanRenderProgramBlob*)program.Data;

    const uint32_t index = m_engine->GetCurrentFrame();

    if (blob->Base != nullptr)
    {
        const uint16_t realSlot = blob->Base->UserSlotToReal(a_slot);

        blob->Base->PushTexture(m_commandBuffer, realSlot, a_sampler, index);
    }

    if (blob->Secondary != nullptr)
    {
        const uint16_t realSlot = blob->Secondary->UserSlotToReal(a_slot);

        blob->Secondary->PushTexture(m_commandBuffer, realSlot, a_sampler, index);
    }

    if (blob->Tertiary != nullptr)
    {
        const uint16_t realSlot = blob->Tertiary->UserSlotToReal(a_slot);

        blob->Tertiary->PushTexture(m_commandBuffer, realSlot, a_sampler, index);
    }
}
void VulkanRenderCommand::PushUserLight(uint32_t a_slot, e_LightType a_lightType, uint32_t a_lightAddr, Allocator* a_tempAllocator)
{
    IVERIFY(m_materialAddr != uint32_t(-1));

    if (!BindResources(a_tempAllocator))
    {
        IWARN("PushLight failed to bind material");

        return;
    }

    const RenderProgram program = m_gEngine->GetRenderProgram(m_materialAddr);
    IVERIFY(program.Data != nullptr);

    VulkanPushPool* pushPool = m_engine->GetPushPool();

    const uint32_t index = m_engine->GetCurrentFrame();

    const VulkanRenderProgramBlob* blob = (VulkanRenderProgramBlob*)program.Data;

    const VulkanUniformBuffer* lightBuffer = ILAMBDA(
    {
        switch (a_lightType)
        {
        case LightType_Ambient:
        {
            const AmbientLightBuffer light = m_gEngine->GetAmbientLight(a_lightAddr);

            IcarianCore::ShaderAmbientLightBuffer buffer;
            buffer.LightColor = glm::vec4(light.Color.xyz(), light.Intensity);

            VulkanUniformBuffer* lightBuffer = pushPool->AllocateAmbientLightUniformBuffer();
            lightBuffer->SetData(index, &buffer);

            ILRETURN lightBuffer;
        }
        case LightType_Directional:
        {
            const DirectionalLightBuffer light = m_gEngine->GetDirectionalLight(a_lightAddr);

            const glm::mat4 tMat = ObjectManager::GetGlobalMatrix(light.TransformAddr);
            const glm::vec3 forward = glm::normalize(tMat[2].xyz());

            IcarianCore::ShaderDirectionalLightBuffer buffer;
            buffer.LightDir = glm::vec4(forward, light.Intensity);
            buffer.LightColor = light.Color;

            VulkanUniformBuffer* lightBuffer = pushPool->AllocateDirectionalLightUniformBuffer();
            lightBuffer->SetData(index, &buffer);

            ILRETURN lightBuffer;
        }
        case LightType_Point:
        {
            const PointLightBuffer light = m_gEngine->GetPointLight(a_lightAddr);

            const glm::mat4 tMat = ObjectManager::GetGlobalMatrix(light.TransformAddr);
            const glm::vec3 position = tMat[3].xyz();

            IcarianCore::ShaderPointLightBuffer buffer;
            buffer.LightPos = glm::vec4(position, light.Intensity);
            buffer.LightColor = light.Color;
            buffer.Radius = light.Radius;

            VulkanUniformBuffer* lightBuffer = pushPool->AllocatePointLightUniformBuffer();
            lightBuffer->SetData(index, &buffer);

            ILRETURN lightBuffer;
        }
        case LightType_Spot:
        {
            const SpotLightBuffer light = m_gEngine->GetSpotLight(a_lightAddr);

            const glm::mat4 tMat = ObjectManager::GetGlobalMatrix(light.TransformAddr);
            const glm::vec3 position = tMat[3].xyz();
            const glm::vec3 forward = glm::normalize(tMat[2].xyz());

            IcarianCore::ShaderSpotLightBuffer buffer;
            buffer.LightPos = position;
            buffer.LightDir = glm::vec4(forward, light.Intensity);
            buffer.LightColor = light.Color;
            buffer.CutoffAngle = glm::vec3(light.CutoffAngle, light.Radius);

            VulkanUniformBuffer* lightBuffer = pushPool->AllocateSpotLightUniformBuffer();
            lightBuffer->SetData(index, &buffer);

            ILRETURN lightBuffer;
        }
        default:
        {
            break;
        }
        }

        ILRETURN (VulkanUniformBuffer*)nullptr;
    });

    IVERIFY(lightBuffer != nullptr);

    if (blob->Base != nullptr)
    {
        const uint16_t realSlot = blob->Base->UserSlotToReal(a_slot);

        blob->Base->PushUniformBuffer(m_commandBuffer, realSlot, lightBuffer, index);
    }

    if (blob->Secondary != nullptr)
    {
        const uint16_t realSlot = blob->Secondary->UserSlotToReal(a_slot);

        blob->Secondary->PushUniformBuffer(m_commandBuffer, realSlot, lightBuffer, index);
    }

    if (blob->Tertiary != nullptr)
    {
        const uint16_t realSlot = blob->Tertiary->UserSlotToReal(a_slot);

        blob->Tertiary->PushUniformBuffer(m_commandBuffer, realSlot, lightBuffer, index);
    }
}
void VulkanRenderCommand::PushUserLightSplits(uint32_t a_slot, const LightShadowSplit* a_splits, uint32_t a_splitCount, Allocator* a_tempAllocator)
{
    if (!BindResources(a_tempAllocator))
    {
        IWARN("PushLightSplits failed to bind material");

        return;
    }

    const RenderProgram program = m_gEngine->GetRenderProgram(m_materialAddr);
    IVERIFY(program.Data != nullptr);

    IcarianCore::ShaderShadowLightBuffer* shadowLightBuffer = ILAMBDA(
    {
        IcarianCore::ShaderShadowLightBuffer* vals = a_tempAllocator->TAllocate<IcarianCore::ShaderShadowLightBuffer>(a_splitCount);

        for (uint32_t i = 0; i < a_splitCount; ++i)
        {
            vals[i].LVP = a_splits[i].LVP;
            vals[i].Split = a_splits[i].Split;
        }

        ILRETURN vals;
    });
    IDEFER(a_tempAllocator->Free(shadowLightBuffer));

    VulkanShaderStorageObject* storage = a_tempAllocator->Create<VulkanShaderStorageObject>
    (
        m_engine,
        sizeof(IcarianCore::ShaderShadowLightBuffer) * a_splitCount,
        a_splitCount,
        shadowLightBuffer
    );
    IDEFER(a_tempAllocator->Destroy(storage));

    const uint32_t index = m_engine->GetCurrentFrame();

    const VulkanRenderProgramBlob* blob = (VulkanRenderProgramBlob*)program.Data;

    if (blob->Base != nullptr)
    {
        const uint16_t realSlot = blob->Base->UserSlotToReal(a_slot);

        blob->Base->PushShaderStorageObject(m_commandBuffer, realSlot, storage, index);
    }

    if (blob->Secondary != nullptr)
    {
        const uint16_t realSlot = blob->Secondary->UserSlotToReal(a_slot);

        blob->Secondary->PushShaderStorageObject(m_commandBuffer, realSlot, storage, index);
    }

    if (blob->Tertiary != nullptr)
    {
        const uint16_t realSlot = blob->Tertiary->UserSlotToReal(a_slot);

        blob->Tertiary->PushShaderStorageObject(m_commandBuffer, realSlot, storage, index);
    }
}
void VulkanRenderCommand::PushUserShadowTextureArray(uint32_t a_slot, uint32_t a_dirLightAddr, Allocator* a_tempAllocator)
{
    if (!BindResources(a_tempAllocator))
    {
        IWARN("PushShadowTextureArray failed to bind material");

        return;
    }

    const RenderProgram program = m_gEngine->GetRenderProgram(m_materialAddr);
    IVERIFY(program.Data != nullptr);

    const VulkanRenderProgramBlob* blob = (VulkanRenderProgramBlob*)program.Data;

    const DirectionalLightBuffer dirLight = m_gEngine->GetDirectionalLight(a_dirLightAddr);
    IVERIFY(dirLight.Data != nullptr);

    const VulkanLightBuffer* lightBuffer = (VulkanLightBuffer*)dirLight.Data;
    const uint32_t count = lightBuffer->LightRenderTextureCount;

    TextureSamplerBuffer* buffer = a_tempAllocator->TAllocate<TextureSamplerBuffer>(count);
    IDEFER(
    {
        for (uint32_t i = 0; i < count; ++i)
        {
            ((VulkanTextureSampler*)buffer[i].Data)->~VulkanTextureSampler();
        }

        a_tempAllocator->Free(buffer);
    });

    for (uint32_t i = 0; i < count; ++i)
    {
        const uint32_t renderTex = lightBuffer->LightRenderTextures[i];

        VulkanTextureSampler* sampler = a_tempAllocator->TAllocate<VulkanTextureSampler>();

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

    const uint32_t index = m_engine->GetCurrentFrame();

    if (blob->Base != nullptr)
    {
        const uint16_t realSlot = blob->Base->UserSlotToReal(a_slot);

        blob->Base->PushTextures(m_commandBuffer, realSlot, buffer, count, index, a_tempAllocator);
    }

    if (blob->Secondary != nullptr)
    {
        const uint16_t realSlot = blob->Secondary->UserSlotToReal(a_slot);

        blob->Secondary->PushTextures(m_commandBuffer, realSlot, buffer, count, index, a_tempAllocator);
    }

    if (blob->Tertiary != nullptr)
    {
        const uint16_t realSlot = blob->Tertiary->UserSlotToReal(a_slot);

        blob->Tertiary->PushTextures(m_commandBuffer, realSlot, buffer, count, index, a_tempAllocator);
    }
}

void VulkanRenderCommand::BindRenderTexture(uint32_t a_renderTexAddr, e_RenderTextureBindMode a_bindMode)
{
    if (m_renderTexAddr == a_renderTexAddr)
    {
        m_renderBindMode = a_bindMode;

        return;
    }

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

    ISETBIT(m_flags, RenderTextureFirstBindBit);
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

void VulkanRenderCommand::DrawMaterial(Allocator* a_tempAllocator)
{
    if (m_materialAddr == uint32_t(-1))
    {
        IERROR("Drawing Material with no Material");

        return;
    }

    if (!BindResources(a_tempAllocator))
    {
        IWARN("DrawMaterial failed to bind material");

        return;
    }

    const RenderProgram program = m_gEngine->GetRenderProgram(m_materialAddr);
    IVERIFY(program.Data != nullptr);

    const VulkanRenderProgramBlob* blob = (VulkanRenderProgramBlob*)program.Data;

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
        IVERIFY(blob->Base != nullptr);

        // const VulkanComputeShader* shader = m_gEngine->GetComputeShader(program.ExtraShader);
        const VulkanShaderData* data = blob->Base;
        IVERIFY(data->GetShaderCount() == 1);

        const VulkanShader* shader = data->GetShader(0);
        IVERIFY(shader != nullptr);
        IVERIFY(shader->GetShaderType() == VulkanShaderType_Compute);

        const VulkanComputeShader* computeShader = (VulkanComputeShader*)shader;

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

        const uint32_t groupX = computeShader->GetWorkgroupX();
        const uint32_t groupY = computeShader->GetWorkgroupY();

        const uint32_t dispatchX = screenWidth / groupX + 1;
        const uint32_t dispatchY = screenHeight / groupY + 1;

        m_commandBuffer.dispatch(dispatchX, dispatchY, 1);

        break;
    }
    default:
    {
        IERROR("DrawMaterial with bad MaterialMode");

        break;
    }
    }
}
void VulkanRenderCommand::DrawModel(const glm::mat4& a_transform, uint32_t a_modelAddr, Allocator* a_tempAllocator)
{
    if (m_materialAddr == uint32_t(-1))
    {
        IERROR("Drawing Model with no Material");

        return;
    }

    if (!BindResources(a_tempAllocator))
    {
        IWARN("DrawModel failed to bind Material");

        return;
    }

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
        const IcarianCore::ShaderModelBuffer buffer = 
        {
            .Model = a_transform,
            .InvModel = glm::inverse(a_transform)
        };

        VulkanShaderStorageObject* storage = a_tempAllocator->Create<VulkanShaderStorageObject>(m_engine, sizeof(IcarianCore::ShaderModelBuffer), 1, &buffer);
        IDEFER(a_tempAllocator->Destroy(storage));

        const uint32_t frameIndex = m_engine->GetCurrentFlightFrame();

        shaderData->PushShaderStorageObject(m_commandBuffer, input.RealSlot, storage, frameIndex);
    }
    else
    {
        shaderData->UpdateTransformBuffer(m_commandBuffer, a_transform);
    }

    m_commandBuffer.drawIndexed(indexCount, 1, 0, 0, 0);
}
void VulkanRenderCommand::DrawMesh(const glm::mat4& a_transform, uint32_t a_meshAddr, uint32_t a_indexCount, Allocator* a_tempAllocator)
{
    if (m_materialAddr == uint32_t(-1))
    {
        IERROR("Drawing Mesh with no Material");

        return;
    }

    const RenderProgram program = m_gEngine->GetRenderProgram(m_materialAddr);
    IVERIFY(program.Data != nullptr);

    const VulkanRenderProgramBlob* blob = (VulkanRenderProgramBlob*)program.Data;
    IVERIFY(blob->Base != nullptr);

    const VulkanMesh* mesh = m_gEngine->GetMesh(a_meshAddr);

    const e_MaterialMode materialMode = blob->Base->GetMaterialMode();

    if (materialMode != MaterialMode_BaseMesh)
    {
        IERROR("Drawing non Mesh Material with DrawMesh");

        return;
    }

    const uint32_t index = m_engine->GetCurrentFrame();

    const bool isMeshEnabled = m_engine->IsMeshEnabled();
    if (isMeshEnabled)
    {
        if (!BindResources(a_tempAllocator))
        {
            IWARN("DrawMesh failed to bind material");

            return;
        }

        const VulkanShaderData* data = blob->Base;

        ShaderBufferInput input;
        if (data->GetShaderBufferInput(ShaderBufferType_SSModelBuffer, &input))
        {
            const IcarianCore::ShaderModelBuffer buffer =
            {
                .Model = a_transform,
                .InvModel = glm::inverse(a_transform)
            };

            const VulkanShaderStorageObject storage = VulkanShaderStorageObject
            (
                m_engine,
                sizeof(IcarianCore::ShaderModelBuffer),
                1,
                &buffer
            );

            data->PushShaderStorageObject(m_commandBuffer, input.RealSlot, &storage, index);
        }
        else
        {
            data->UpdateTransformBuffer(m_commandBuffer, a_transform);
        }

        // We do have native mesh support so we can just directly dispatch the call no questions asked
        m_commandBuffer.drawMeshTasksEXT(a_indexCount, 1, 1);
    }
    else
    {
        if (IISBITSET(m_flags, RenderTextureBoundBit))
        {
            m_commandBuffer.endRenderPass();

            ICLEARBIT(m_flags, RenderTextureBoundBit);
        }
        ICLEARBIT(m_flags, MaterialBoundBit);

        // We do not have native Mesh support so we need to emulate the functionality
        // Note Vulkan is annoying as you cannot dispatch Compute in an active RenderPass
        // To get around this when RenderCommands are in emulated mode we have to manually bind the RenderPass
        // when it is a Mesh Material
        const VulkanMeshEmulationData* emulationData = m_gEngine->GetMeshEmulationData();
        IVERIFY(emulationData != nullptr);

        const bool hasTask = program.ExtraShader != uint32_t(-1);

        const VulkanPipeline* meshPipeline = m_gEngine->GetComputeMeshPipeline(m_materialAddr);
        IVERIFY(meshPipeline != nullptr);

        const VulkanShaderData* meshComputeData = blob->Secondary;
        IVERIFY(meshComputeData != nullptr);

        const vk::BufferMemoryBarrier drawBarrier = vk::BufferMemoryBarrier
        (
            vk::AccessFlagBits::eIndirectCommandRead, // I think?
            vk::AccessFlagBits::eTransferWrite,
            vk::QueueFamilyIgnored,
            vk::QueueFamilyIgnored,
            emulationData->DrawBuffer,
            0,
            vk::WholeSize
        );
        m_commandBuffer.pipelineBarrier
        (
            vk::PipelineStageFlagBits::eDrawIndirect,
            vk::PipelineStageFlagBits::eTransfer,
            { },
            0,
            nullptr,
            1,
            &drawBarrier,
            0,
            nullptr
        );

        m_commandBuffer.fillBuffer(emulationData->DrawBuffer, 0, vk::WholeSize, 0);

        // TODO: We need to properly handle Task Shaders
        // This will likely end up a Dispatch->IndirectDispatch->IndirectDraw still figuring out details however
        if (hasTask)
        {
            IERROR("Not implemented!~");
        }
        else
        {
            {
                // Compute is annoying so have to manually bind alot of things
                if (!meshPipeline->Bind(index, m_commandBuffer))
                {
                    IWARN("DrawMesh failed to bind compute material");

                    return;
                }
                IDEFER(meshPipeline->Unbind(index, m_commandBuffer));

                ShaderBufferInput input;

                meshComputeData->PushMeshBuffers(m_commandBuffer, mesh, index);

                if (meshComputeData->GetShaderBufferInput(ShaderBufferType_SSModelBuffer, &input))
                {
                    const IcarianCore::ShaderModelBuffer buffer =
                    {
                        .Model = a_transform,
                        .InvModel = glm::inverse(a_transform)
                    };

                    const VulkanShaderStorageObject storage = VulkanShaderStorageObject
                    (
                        m_engine,
                        sizeof(IcarianCore::ShaderModelBuffer),
                        1,
                        &buffer
                    );

                    meshComputeData->PushShaderStorageObject(m_commandBuffer, input.RealSlot, &storage, index);
                }
                else
                {
                    meshComputeData->UpdateTransformBuffer(m_commandBuffer, a_transform);
                }

                if (meshComputeData->GetShaderBufferInput(ShaderBufferType_CameraBuffer, &input))
                {
                    if (m_cameraAddr == uint32_t(-1))
                    {
                        IERROR("No bound camera");
                    }

                    const VulkanRenderTexture* renderTexture = m_gEngine->GetRenderTexture(m_renderTexAddr);
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

                    VulkanUniformBuffer* camUBO = m_gEngine->GetCameraUniformBuffer(m_bufferIndex);
                    camUBO->SetData(curFrame, &cameraShaderData);

                    meshComputeData->PushUniformBuffer(m_commandBuffer, input.RealSlot, camUBO, index);
                }

                const vk::BufferMemoryBarrier drawBarrier = vk::BufferMemoryBarrier
                (
                    vk::AccessFlagBits::eTransferWrite, // I think?
                    vk::AccessFlagBits::eShaderWrite,
                    vk::QueueFamilyIgnored,
                    vk::QueueFamilyIgnored,
                    emulationData->DrawBuffer,
                    0,
                    vk::WholeSize
                );
                m_commandBuffer.pipelineBarrier
                (
                    vk::PipelineStageFlagBits::eTransfer,
                    vk::PipelineStageFlagBits::eComputeShader,
                    { },
                    0,
                    nullptr,
                    1,
                    &drawBarrier,
                    0,
                    nullptr
                );

                const vk::BufferMemoryBarrier inputBarriers[] =
                {
                    vk::BufferMemoryBarrier
                    (
                        vk::AccessFlagBits::eVertexAttributeRead,
                        vk::AccessFlagBits::eShaderWrite,
                        vk::QueueFamilyIgnored,
                        vk::QueueFamilyIgnored,
                        emulationData->VertexBuffer,
                        0,
                        vk::WholeSize
                    ),
                    vk::BufferMemoryBarrier
                    (
                        vk::AccessFlagBits::eIndexRead,
                        vk::AccessFlagBits::eShaderWrite,
                        vk::QueueFamilyIgnored,
                        vk::QueueFamilyIgnored,
                        emulationData->IndexBuffer,
                        0,
                        vk::WholeSize
                    )
                };

                constexpr uint32_t BarrierCount = sizeof(inputBarriers) / sizeof(*inputBarriers);

                m_commandBuffer.pipelineBarrier
                (
                    vk::PipelineStageFlagBits::eVertexInput,
                    vk::PipelineStageFlagBits::eComputeShader,
                    { },
                    0,
                    nullptr,
                    BarrierCount,
                    inputBarriers,
                    0,
                    nullptr
                );

                m_commandBuffer.dispatch(a_indexCount, 1, 1);
            }

            {
                const vk::BufferMemoryBarrier drawBarrier = vk::BufferMemoryBarrier
                (
                    vk::AccessFlagBits::eShaderWrite,
                    vk::AccessFlagBits::eIndirectCommandRead, // I think?
                    vk::QueueFamilyIgnored,
                    vk::QueueFamilyIgnored,
                    emulationData->DrawBuffer,
                    0,
                    vk::WholeSize
                );
                m_commandBuffer.pipelineBarrier
                (
                    vk::PipelineStageFlagBits::eComputeShader,
                    vk::PipelineStageFlagBits::eDrawIndirect,
                    { },
                    0,
                    nullptr,
                    1,
                    &drawBarrier,
                    0,
                    nullptr
                );

                const vk::BufferMemoryBarrier inputBarriers[] = 
                {
                    vk::BufferMemoryBarrier
                    (
                        vk::AccessFlagBits::eShaderWrite,
                        vk::AccessFlagBits::eVertexAttributeRead,
                        vk::QueueFamilyIgnored,
                        vk::QueueFamilyIgnored,
                        emulationData->VertexBuffer,
                        0,
                        vk::WholeSize
                    ),
                    vk::BufferMemoryBarrier
                    (
                        vk::AccessFlagBits::eShaderWrite,
                        vk::AccessFlagBits::eIndexRead,
                        vk::QueueFamilyIgnored,
                        vk::QueueFamilyIgnored,
                        emulationData->IndexBuffer,
                        0,
                        vk::WholeSize
                    )
                };

                constexpr uint32_t BarrierCount = sizeof(inputBarriers) / sizeof(*inputBarriers);

                m_commandBuffer.pipelineBarrier
                (
                    vk::PipelineStageFlagBits::eComputeShader,
                    vk::PipelineStageFlagBits::eVertexInput,
                    { },
                    0,
                    nullptr,
                    BarrierCount,
                    inputBarriers,
                    0,
                    nullptr
                );

                if (!BindResources(a_tempAllocator))
                {
                    IWARN("DrawMesh failed to bind material");

                    return;
                }

                const VulkanPipeline* pipeline = m_gEngine->GetPipeline(m_renderTexAddr, m_materialAddr);
                IVERIFY(pipeline != nullptr);

                const VulkanShaderData* data = blob->Base;
                IVERIFY(data != nullptr);

                pipeline->Bind(index, m_commandBuffer);
                IDEFER(pipeline->Unbind(index, m_commandBuffer));

                ShaderBufferInput input;
                if (data->GetShaderBufferInput(ShaderBufferType_SSModelBuffer, &input))
                {
                    const IcarianCore::ShaderModelBuffer buffer =
                    {
                        .Model = a_transform,
                        .InvModel = glm::inverse(a_transform)
                    };

                    const VulkanShaderStorageObject storage = VulkanShaderStorageObject
                    (
                        m_engine,
                        sizeof(IcarianCore::ShaderModelBuffer),
                        1,
                        &buffer
                    );

                    data->PushShaderStorageObject(m_commandBuffer, input.RealSlot, &storage, index);
                }
                else
                {
                    data->UpdateTransformBuffer(m_commandBuffer, a_transform);
                }

                data->PushMeshBuffers(m_commandBuffer, mesh, index);

                const vk::DeviceSize Offsets[] = { 0 };

                m_commandBuffer.bindIndexBuffer(emulationData->IndexBuffer, 0, vk::IndexType::eUint32);
                m_commandBuffer.bindVertexBuffers(0, 1, &emulationData->VertexBuffer, Offsets);

                m_commandBuffer.drawIndexedIndirect(emulationData->DrawBuffer, 0, a_indexCount, sizeof(vk::DrawIndexedIndirectCommand));
            }
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
