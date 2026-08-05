// Icarian Engine - C# Game Engine
//
// License at end of file.

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/VulkanGraphicsEngine.h"

#include <future>
#include <glm/gtx/matrix_decompose.hpp>
#include <vulkan/vulkan_handles.hpp>

#include "Core/DataTypes/Allocators/MallocAllocator.h"
#include "Core/DataTypes/Allocators/StackAllocator.h"
#include "Core/IcarianDefer.h"
#include "Core/IcarianLambda.h"
#include "Core/ShaderBuffers.h"
#include "IcarianError.h"
#include "Logger.h"
#include "ObjectManager.h"
#include "Profiler.h"
#include "Rendering/AnimationController.h"
#include "Rendering/RenderAssetStore.h"
#include "Rendering/RenderEngine.h"
#include "Rendering/UI/ImageUIElement.h"
#include "Rendering/UI/TextUIElement.h"
#include "Rendering/UI/UIControl.h"
#include "Rendering/UI/UIElement.h"
#include "Rendering/Vulkan/VulkanDepthCubeRenderTexture.h"
#include "Rendering/Vulkan/VulkanDepthRenderTexture.h"
#include "Rendering/Vulkan/VulkanGraphicsEngineBindings.h"
#include "Rendering/Vulkan/VulkanGraphicsParticle2D.h"
#include "Rendering/Vulkan/VulkanLightBuffer.h"
#include "Rendering/Vulkan/VulkanLightData.h"
#include "Rendering/Vulkan/VulkanMesh.h"
#include "Rendering/Vulkan/VulkanModel.h"
#include "Rendering/Vulkan/VulkanPipeline.h"
#include "Rendering/Vulkan/VulkanProfiler.h"
#include "Rendering/Vulkan/VulkanPushPool.h"
#include "Rendering/Vulkan/VulkanRenderCommand.h"
#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"
#include "Rendering/Vulkan/VulkanRenderProgramBlob.h"
#include "Rendering/Vulkan/VulkanRenderTexture.h"
#include "Rendering/Vulkan/VulkanShaderData.h"
#include "Rendering/Vulkan/VulkanShaderStorageObject.h"
#include "Rendering/Vulkan/VulkanSwapchain.h"
#include "Rendering/Vulkan/VulkanTexture.h"
#include "Rendering/Vulkan/VulkanTextureSampler.h"
#include "Rendering/Vulkan/VulkanUniformBuffer.h"
#include "Rendering/Vulkan/VulkanVideoTexture.h"
#include "Runtime/RuntimeFunction.h"
#include "Runtime/RuntimeManager.h"
#include "Shaders.h"
#include "Trace.h"
#include "ThreadPool.h"

#include "EngineLightInteropStructures.h"

VulkanGraphicsEngine::VulkanGraphicsEngine(VulkanRenderEngineBackend* a_vulkanEngine, VulkanSwapchain* a_swapchain) :
    m_pipelines(a_vulkanEngine->GetAllocator()),
    m_shadowPipelines(a_vulkanEngine->GetAllocator()),
    m_cubeShadowPipelines(a_vulkanEngine->GetAllocator()),
    m_computeImports(a_vulkanEngine->GetAllocator()),
    m_vertexImports(a_vulkanEngine->GetAllocator()),
    m_meshImports(a_vulkanEngine->GetAllocator()),
    m_pixelImports(a_vulkanEngine->GetAllocator()),
    m_cameraUniforms(a_vulkanEngine->GetAllocator())
{
    m_vulkanEngine = a_vulkanEngine;

    m_swapchain = a_swapchain;

    IcarianCore::Allocator* blockAllocator = m_vulkanEngine->GetAllocator();
    IcarianCore::StackAllocator* scratchAllocator = RenderScratchAlloc::GetAllocator();

    for (uint32_t i = 0; i < VulkanFlightPoolSize; ++i)
    {
        m_commandPool[i] = blockAllocator->Create<IcarianCore::Array<vk::CommandPool>>(blockAllocator);
        m_commandBuffers[i] = blockAllocator->Create<IcarianCore::Array<vk::CommandBuffer>>(blockAllocator);
    }

    TRACE("Getting RenderPipeline Functions");
    m_shadowSetupFunc = RuntimeManager::GetFunction("IcarianEngine.Rendering", "RenderPipeline", ":ShadowSetupS(uint,uint)");
    m_preShadowFunc = RuntimeManager::GetFunction("IcarianEngine.Rendering", "RenderPipeline", ":PreShadowS(uint,uint,uint,uint)");
    m_postShadowFunc = RuntimeManager::GetFunction("IcarianEngine.Rendering", "RenderPipeline", ":PostShadowS(uint,uint,uint,uint)");
    m_preRenderFunc = RuntimeManager::GetFunction("IcarianEngine.Rendering", "RenderPipeline", ":PreRenderS(uint)");
    m_postRenderFunc = RuntimeManager::GetFunction("IcarianEngine.Rendering", "RenderPipeline", ":PostRenderS(uint)");
    m_lightSetupFunc = RuntimeManager::GetFunction("IcarianEngine.Rendering", "RenderPipeline", ":LightSetupS(uint)");
    m_preShadowLightFunc = RuntimeManager::GetFunction("IcarianEngine.Rendering", "RenderPipeline", ":PreShadowLightS(uint,uint,uint)");
    m_postShadowLightFunc = RuntimeManager::GetFunction("IcarianEngine.Rendering", "RenderPipeline", ":PostShadowLightS(uint,uint,uint)");
    m_preLightFunc = RuntimeManager::GetFunction("IcarianEngine.Rendering", "RenderPipeline", ":PreLightS(uint,uint)");
    m_postLightFunc = RuntimeManager::GetFunction("IcarianEngine.Rendering", "RenderPipeline", ":PostLightS(uint,uint)");
    m_preForwardFunc = RuntimeManager::GetFunction("IcarianEngine.Rendering", "RenderPipeline", ":PreForwardS(uint)");
    m_postForwardFunc = RuntimeManager::GetFunction("IcarianEngine.Rendering", "RenderPipeline", ":PostForwardS(uint)");
    m_postProcessFunc = RuntimeManager::GetFunction("IcarianEngine.Rendering", "RenderPipeline", ":PostProcessS(uint)");

    {
        RENDERSCRATCHFRAME;

        const uint32_t vertexShader = ILAMBDA(
        {
            RENDERSCRATCHFRAME;

            const IcarianCore::COWU8String str = IcarianCore::COWU8String
            (
                UIVertexShader,
                sizeof(UIVertexShader) / sizeof(*UIVertexShader),
                scratchAllocator
            );

            ILRETURN GenerateFVertexShader(str);
        });
        const uint32_t pixelShader = ILAMBDA(
        {
            RENDERSCRATCHFRAME;

            const IcarianCore::COWU8String str = IcarianCore::COWU8String
            (
                UITextPixelShader,
                sizeof(UITextPixelShader) / sizeof(*UITextPixelShader),
                scratchAllocator
            );

            ILRETURN GenerateFPixelShader(str);
        });

        constexpr uint8_t Flags = ILAMBDA(
        {
            uint8_t val = 0;

            ISETBIT(val, RenderProgram::DestroyFlag);

            ILRETURN val;
        });

        const RenderProgram textProgram =
        {
            .VertexShader = vertexShader,
            .PixelShader = pixelShader,
            .ShadowVertexShader = uint32_t(-1),
            .ColorBlendMode = MaterialBlendMode_Alpha,
            .CullingMode = CullMode_None,
            .PrimitiveMode = PrimitiveMode_TriangleStrip,
            .Flags = Flags,
        };

        m_textUIPipelineAddr = GenerateRenderProgram(textProgram, scratchAllocator);
    }

    {
        RENDERSCRATCHFRAME;

        const RenderProgram imageProgram =
        {
            .VertexShader = ILAMBDA(
            {
                RENDERSCRATCHFRAME;

                const IcarianCore::COWU8String str = IcarianCore::COWU8String
                (
                    UIVertexShader,
                    sizeof(UIVertexShader) / sizeof(*UIVertexShader),
                    scratchAllocator
                );

                ILRETURN GenerateFVertexShader(str);
            }),
            .PixelShader = ILAMBDA(
            {
                RENDERSCRATCHFRAME;

                const IcarianCore::COWU8String str = IcarianCore::COWU8String
                (
                    UIImagePixelShader,
                    sizeof(UIImagePixelShader) / sizeof(*UIImagePixelShader),
                    scratchAllocator
                );

                ILRETURN GenerateFPixelShader(str);
            }),
            .ShadowVertexShader = uint32_t(-1),
            .ColorBlendMode = MaterialBlendMode_Alpha,
            .CullingMode = CullMode_None,
            .PrimitiveMode = PrimitiveMode_TriangleStrip,
            .Flags = 0b1 << RenderProgram::DestroyFlag
        };

        m_imageUIPipelineAddr = GenerateRenderProgram(imageProgram, scratchAllocator);
    }

    m_timeUniform = blockAllocator->Create<VulkanUniformBuffer>(m_vulkanEngine, sizeof(IcarianCore::ShaderTimeBuffer));

    m_meshEmulationData = nullptr;

    const bool isMeshEnabled = m_vulkanEngine->IsMeshEnabled();
    if (!isMeshEnabled)
    {
        TRACE("Creating Mesh emulation data");

        const VmaAllocator vmaAllocator = m_vulkanEngine->GetVMAAllocator();

        m_meshEmulationData = blockAllocator->ZTAllocate<VulkanMeshEmulationData>();

        m_meshEmulationData->MeshPipelines = IcarianCore::Dictionary<uint64_t, VulkanPipeline*>(blockAllocator);

        VkBuffer buffer;
        const VmaAllocationCreateInfo allocInfo =
        {
            .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
        };

        const VkBufferCreateInfo drawBufferInfo =
        {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = (VkDeviceSize)(64UL << 20),
            .usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE
        };

        VKRESERRMSG(vmaCreateBuffer
        (
            vmaAllocator,
            &drawBufferInfo,
            &allocInfo,
            &buffer,
            &m_meshEmulationData->DrawAlloction,
            NULL
        ), "Failed to create Mesh Emulation Draw Buffer");
#ifdef DEBUG
        vmaSetAllocationName(vmaAllocator, m_meshEmulationData->DrawAlloction, "Mesh Emulation Draw Buffer");
#endif
        m_meshEmulationData->DrawBuffer = buffer;

        // If mesh shaders are not supported by the GPU we need to run it on the compute pipeline
        // In order to do that will need some intermediary buffers
        // We also need to generate a passthrough Vertex shader to get it to the rasterizer to run the Pixel/Fragment shader
        // Yes this will have high overhead however want to start moving to Mesh as primary as non Mesh hardware is getting old now
        // Also Mesh is much more flexible then the fixed function pipeline
        // All this is a stop gap as we cannot quite kill that hardware that does not support it just yet
        const VkBufferCreateInfo vertexBufferInfo =
        {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = (VkDeviceSize)(128UL << 20),
            .usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE
        };

        VKRESERRMSG(vmaCreateBuffer
        (
            vmaAllocator,
            &vertexBufferInfo,
            &allocInfo,
            &buffer,
            &m_meshEmulationData->VertexAllocation,
            NULL
        ), "Failed to create Mesh Emulation Vertex Buffer");
#ifdef DEBUG
        vmaSetAllocationName(vmaAllocator, m_meshEmulationData->VertexAllocation, "Mesh Emulation Vertex Buffer");
#endif
        m_meshEmulationData->VertexBuffer = buffer;

        const VkBufferCreateInfo indexBufferInfo =
        {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = (VkDeviceSize)(64UL << 20),
            .usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE
        };

        VKRESERRMSG(vmaCreateBuffer
        (
            vmaAllocator,
            &indexBufferInfo,
            &allocInfo,
            &buffer,
            &m_meshEmulationData->IndexAllocation,
            NULL
        ), "Failed to create Mesh Emulation Index Buffer");
#ifdef DEBUG
        vmaSetAllocationName(vmaAllocator, m_meshEmulationData->IndexAllocation, "Mesh Emulation Index Buffer");
#endif
        m_meshEmulationData->IndexBuffer = buffer;
    }

    m_runtimeBindings = blockAllocator->Create<VulkanGraphicsEngineBindings>(this);
}
VulkanGraphicsEngine::~VulkanGraphicsEngine()
{
    IcarianCore::Allocator* blockAllocator = m_vulkanEngine->GetAllocator();
    IcarianCore::StackAllocator* scratchAllocator = RenderScratchAlloc::GetAllocator();

    TRACE("Checking if shaders where deleted");
    for (uint32_t i = 0; i < m_vertexShaders.Size(); ++i)
    {
        if (m_vertexShaders.Exists(i))
        {
            Logger::Warning("Vertex Shader was not destroyed");

            // blockAllocator->Destroy(m_vertexShaders[i]);
            m_vertexShaders.Erase(i);
        }
    }

    for (uint32_t i = 0; i < m_pixelShaders.Size(); ++i)
    {
        if (m_pixelShaders.Exists(i))
        {
            Logger::Warning("Pixel Shader was not destroyed");

            // blockAllocator->Destroy(m_pixelShaders[i]);
            m_pixelShaders.Erase(i);
        }
    }

    for (uint32_t i = 0; i < m_meshShaders.Size(); ++i)
    {
        if (m_meshShaders.Exists(i))
        {
            Logger::Warning("Mesh Shader was not destroyed");

            // blockAllocator->Destroy(m_meshShaders[i]);
            m_meshShaders.Erase(i);
        }
    }

    for (uint32_t i = 0; i < m_taskShaders.Size(); ++i)
    {
        if (m_taskShaders.Exists(i))
        {
            Logger::Warning("Task Shader was not destroyed");

            // blockAllocator->Destroy(m_taskShaders[i]);
            m_taskShaders.Erase(i);
        }
    }

    for (uint32_t i = 0; i < m_computeShaders.Size(); ++i)
    {
        if (m_computeShaders.Exists(i))
        {
            Logger::Warning("Compute Shader was not destroyed");

            m_computeShaders.Erase(i);
        }
    }

    {
        RENDERSCRATCHFRAME;

        TRACE("Deleting Pipelines");
        const IcarianCore::Array<VulkanPipeline*> pipeValues = m_pipelines.GetValues(scratchAllocator);
        for (VulkanPipeline* p : pipeValues)
        {
            blockAllocator->Destroy(p);
        }
    }

    {
        RENDERSCRATCHFRAME;

        TRACE("Deleting Shadow Pipelines");
        const IcarianCore::Array<VulkanPipeline*> shadowPipeValues = m_shadowPipelines.GetValues(scratchAllocator);
        for (VulkanPipeline* p : shadowPipeValues)
        {
            blockAllocator->Destroy(p);
        }
    }

    {
        RENDERSCRATCHFRAME;

        TRACE("Deleting Cube Shadow Pipelines");
        const IcarianCore::Array<VulkanPipeline*> cubeShadowPipeValues = m_cubeShadowPipelines.GetValues(scratchAllocator);
        for (VulkanPipeline* p : cubeShadowPipeValues)
        {
            blockAllocator->Destroy(p);
        }
    }

    if (m_meshEmulationData != nullptr)
    {
        const VmaAllocator allocator = m_vulkanEngine->GetVMAAllocator();

        {
            RENDERSCRATCHFRAME;

            const IcarianCore::Array<VulkanPipeline*> meshPipeValues = m_meshEmulationData->MeshPipelines.GetValues(scratchAllocator);
            for (VulkanPipeline* p : meshPipeValues)
            {
                blockAllocator->Destroy(p);
            }
        }

        vmaDestroyBuffer(allocator, m_meshEmulationData->DrawBuffer, m_meshEmulationData->DrawAlloction);
        vmaDestroyBuffer(allocator, m_meshEmulationData->VertexBuffer, m_meshEmulationData->VertexAllocation);
        vmaDestroyBuffer(allocator, m_meshEmulationData->IndexBuffer, m_meshEmulationData->IndexAllocation);

        blockAllocator->Destroy(m_meshEmulationData);
    }

    TRACE("Checking shader program buffer health");
    for (uint32_t i = 0; i < m_shaderPrograms.Size(); ++i)
    {
        if (m_shaderPrograms.Exists(i))
        {
            Logger::Warning("Shader program was not destroyed");
        }

        if (m_shaderPrograms[i].Data != nullptr)
        {
            Logger::Warning("Shader data was not destroyed");

            VulkanRenderProgramBlob* blob = (VulkanRenderProgramBlob*)m_shaderPrograms[i].Data;
            IDEFER(blockAllocator->Free(blob));

            if (blob->Shadow != nullptr)
            {
                blockAllocator->Destroy(blob->Shadow);
            }
            if (blob->Base != nullptr)
            {
                blockAllocator->Destroy(blob->Base);
            }
            if (blob->Secondary != nullptr)
            {
                blockAllocator->Destroy(blob->Secondary);
            }
            if (blob->Tertiary != nullptr)
            {
                blockAllocator->Destroy(blob->Tertiary);
            }
        }
    }

    blockAllocator->Destroy(m_runtimeBindings);

    for (uint32_t i = 0; i < VulkanFlightPoolSize; ++i)
    {
        blockAllocator->Destroy(m_commandBuffers[i]);
        blockAllocator->Destroy(m_commandPool[i]);
    }

    IcarianCore::MallocAllocator::Instance->Destroy(m_shadowSetupFunc);
    IcarianCore::MallocAllocator::Instance->Destroy(m_preShadowFunc);
    IcarianCore::MallocAllocator::Instance->Destroy(m_postShadowFunc);
    IcarianCore::MallocAllocator::Instance->Destroy(m_preRenderFunc);
    IcarianCore::MallocAllocator::Instance->Destroy(m_postRenderFunc);
    IcarianCore::MallocAllocator::Instance->Destroy(m_lightSetupFunc);
    IcarianCore::MallocAllocator::Instance->Destroy(m_preShadowLightFunc);
    IcarianCore::MallocAllocator::Instance->Destroy(m_postShadowLightFunc);
    IcarianCore::MallocAllocator::Instance->Destroy(m_preLightFunc);
    IcarianCore::MallocAllocator::Instance->Destroy(m_postLightFunc);
    IcarianCore::MallocAllocator::Instance->Destroy(m_preForwardFunc);
    IcarianCore::MallocAllocator::Instance->Destroy(m_postForwardFunc);
    IcarianCore::MallocAllocator::Instance->Destroy(m_postProcessFunc);
}

void VulkanGraphicsEngine::Cleanup()
{
    IcarianCore::Allocator* allocator = m_vulkanEngine->GetAllocator();

    allocator->Destroy(m_timeUniform);

    const RenderProgram textProgram = m_shaderPrograms[m_textUIPipelineAddr];
    IDEFER(
    {
        if (textProgram.VertexAttributes != nullptr)
        {
            delete[] textProgram.VertexAttributes;
        }
    });

    const RenderProgram imageProgram = m_shaderPrograms[m_imageUIPipelineAddr];
    IDEFER(
    {
        if (imageProgram.VertexAttributes != nullptr)
        {
            delete[] imageProgram.VertexAttributes;
        }
    });

    DestroyRenderProgram(m_textUIPipelineAddr);
    DestroyRenderProgram(m_imageUIPipelineAddr);

    const vk::Device device = m_vulkanEngine->GetLogicalDevice();

    TRACE("Deleting command pool");
    for (uint32_t i = 0; i < VulkanFlightPoolSize; ++i)
    {
        for (const vk::CommandPool& pool : *m_commandPool[i])
        {
            device.destroyCommandPool(pool);
        }

        m_commandPool[i]->Clear();
        m_commandBuffers[i]->Clear();
    }

    TRACE("Deleting camera ubos");
    for (VulkanUniformBuffer* uniform : m_cameraUniforms)
    {
        if (uniform != nullptr)
        {
            allocator->Destroy(uniform);
        }
    }

    TRACE("Checking is particle emitters where deleted");
    for (uint32_t i = 0; i < m_particleEmitters.Size(); ++i)
    {
        if (m_particleEmitters.Exists(i))
        {
            Logger::Warning("Particle emitter was not destroyed");

            allocator->Destroy(m_particleEmitters[i]);
            m_particleEmitters.Erase(i);
        }
    }

    TRACE("Checking if models where deleted");
    for (uint32_t i = 0; i < m_models.Size(); ++i)
    {
        if (m_models.Exists(i))
        {
            Logger::Warning("Model was not destroyed");

            allocator->Destroy(m_models[i]);
            m_models.Erase(i);
        }
    }

    TRACE("Checking if meshes where deleted");
    for (uint32_t i = 0; i < m_meshes.Size(); ++i)
    {
        if (m_meshes.Exists(i))
        {
            Logger::Warning("Mesh was not destroyed");

            allocator->Destroy(m_meshes[i]);
            m_meshes.Erase(i);
        }
    }

    TRACE("Checking camera buffer health");
    for (uint32_t i = 0; i < m_cameraBuffers.Size(); ++i)
    {
        if (m_cameraBuffers[i].TransformAddr != uint32_t(-1))
        {
            Logger::Warning("Camera was not destroyed");
        }
    }

    TRACE("Checking if render textures where deleted");
    for (uint32_t i = 0; i < m_renderTextures.Size(); ++i)
    {
        if (m_renderTextures.Exists(i))
        {
            Logger::Warning("Render Texture was not destroyed");

            allocator->Destroy(m_renderTextures[i]);
            m_renderTextures.Erase(i);
        }
    }
    for (uint32_t i = 0; i < m_depthCubeRenderTextures.Size(); ++i)
    {
        if (m_depthCubeRenderTextures.Exists(i))
        {
            Logger::Warning("Depth Cube Render Texture was not destroyed");

            allocator->Destroy(m_depthCubeRenderTextures[i]);
            m_depthCubeRenderTextures.Erase(i);
        }
    }
    for (uint32_t i = 0; i < m_depthRenderTextures.Size(); ++i)
    {
        if (m_depthRenderTextures.Exists(i))
        {
            Logger::Warning("Depth Render Texture was not destroyed");

            allocator->Destroy(m_depthRenderTextures[i]);
            m_depthRenderTextures.Erase(i);
        }
    }

    TRACE("Checking if textures where deleted");
    for (uint32_t i = 0; i < m_textures.Size(); ++i)
    {
        if (m_textures.Exists(i))
        {
            Logger::Warning("Texture was not destroyed");

            delete m_textures[i];
            m_textures.Erase(i);
        }
    }
    TRACE("Checking if texture samplers where deleted");
    for (uint32_t i = 0; i < m_textureSampler.Size(); ++i)
    {
        if (m_textureSampler.Exists(i))
        {
            Logger::Warning("Texture sampler was not destroyed");
        }

        if (m_textureSampler[i].Data != nullptr)
        {
            Logger::Warning("Texture sampler data was not destroyed");

            allocator->Destroy((VulkanTextureSampler*)m_textureSampler[i].Data);
        }
    }
}

uint32_t VulkanGraphicsEngine::GenerateFVertexShader(const IcarianCore::COWU8String& a_source)
{
    IVERIFY(!a_source.Empty());

    IcarianCore::Allocator* blockAllocator = m_vulkanEngine->GetAllocator();

    const VulkanShaderInfo info =
    {
        // Copying as this could be allocated on a temp allocator
        .Data = IcarianCore::COWU8String(a_source, blockAllocator),
        .Type = VulkanShaderInfoType_Flare,
    };

    return m_vertexShaders.PushVal(info);
}
void VulkanGraphicsEngine::DestroyVertexShader(uint32_t a_addr)
{
    IVERIFY(m_vertexShaders.Exists(a_addr));

    m_vertexShaders.Erase(a_addr);
}
VulkanShaderInfo VulkanGraphicsEngine::GetVertexShaderInfo(uint32_t a_addr)
{
    IVERIFY(m_vertexShaders.Exists(a_addr));

    return m_vertexShaders[a_addr];
}
IcarianCore::Dictionary<IcarianCore::COWU8String, IcarianCore::COWU8String> VulkanGraphicsEngine::GetVertexShaderImports()
{
    const IcarianCore::SharedThreadGuard g = IcarianCore::SharedThreadGuard(m_importLock);

    return m_vertexImports;
}

uint32_t VulkanGraphicsEngine::GenerateFTaskShader(const IcarianCore::COWU8String& a_source)
{
    IVERIFY(!a_source.Empty());

    IcarianCore::Allocator* blockAllocator = m_vulkanEngine->GetAllocator();

    const VulkanShaderInfo info =
    {
        .Data = IcarianCore::COWU8String(a_source, blockAllocator),
        .Type = VulkanShaderInfoType_Flare,
    };

    return m_taskShaders.PushVal(info);
}
void VulkanGraphicsEngine::DestroyTaskShader(uint32_t a_addr)
{
    IVERIFY(m_taskShaders.Exists(a_addr));

    m_taskShaders.Erase(a_addr);
}
VulkanShaderInfo VulkanGraphicsEngine::GetTaskShaderInfo(uint32_t a_addr)
{
    IVERIFY(m_taskShaders.Exists(a_addr));

    return m_taskShaders[a_addr];
}
uint32_t VulkanGraphicsEngine::GenerateFMeshShader(const IcarianCore::COWU8String& a_source)
{
    IVERIFY(!a_source.Empty());

    IcarianCore::Allocator* blockAllocator = m_vulkanEngine->GetAllocator();

    const VulkanShaderInfo info =
    {
        .Data = IcarianCore::COWU8String(a_source, blockAllocator),
        .Type = VulkanShaderInfoType_Flare
    };

    return m_meshShaders.PushVal(info);
}
void VulkanGraphicsEngine::DestroyMeshShader(uint32_t a_addr)
{
    IVERIFY(m_meshShaders.Exists(a_addr));

    m_meshShaders.Erase(a_addr);
}
VulkanShaderInfo VulkanGraphicsEngine::GetMeshShaderInfo(uint32_t a_addr)
{
    IVERIFY(m_meshShaders.Exists(a_addr));

    return m_meshShaders[a_addr];
}
IcarianCore::Dictionary<IcarianCore::COWU8String, IcarianCore::COWU8String> VulkanGraphicsEngine::GetMeshShaderImports()
{
    const IcarianCore::SharedThreadGuard g = IcarianCore::SharedThreadGuard(m_importLock);

    return m_meshImports;
}

uint32_t VulkanGraphicsEngine::GenerateFPixelShader(const IcarianCore::COWU8String& a_source)
{
    IVERIFY(!a_source.Empty());

    IcarianCore::Allocator* blockAllocator = m_vulkanEngine->GetAllocator();

    const VulkanShaderInfo info =
    {
        .Data = IcarianCore::COWU8String(a_source, blockAllocator),
        .Type = VulkanShaderInfoType_Flare
    };

    return m_pixelShaders.PushVal(info);
}
void VulkanGraphicsEngine::DestroyPixelShader(uint32_t a_addr)
{
    IVERIFY(m_pixelShaders.Exists(a_addr));

    m_pixelShaders.Erase(a_addr);
}
VulkanShaderInfo VulkanGraphicsEngine::GetPixelShaderInfo(uint32_t a_addr)
{
    IVERIFY(m_pixelShaders.Exists(a_addr));

    return m_pixelShaders[a_addr];
}
IcarianCore::Dictionary<IcarianCore::COWU8String, IcarianCore::COWU8String> VulkanGraphicsEngine::GetPixelShaderImports()
{
    const IcarianCore::SharedThreadGuard g = IcarianCore::SharedThreadGuard(m_importLock);

    return m_pixelImports;
}

uint32_t VulkanGraphicsEngine::GenerateFComputeShader(const IcarianCore::COWU8String& a_source)
{
    IVERIFY(!a_source.Empty());

    IcarianCore::Allocator* blockAllocator = m_vulkanEngine->GetAllocator();

    const VulkanShaderInfo info =
    {
        .Data = IcarianCore::COWU8String(a_source, blockAllocator),
        .Type = VulkanShaderInfoType_Flare,
    };

    return m_computeShaders.PushVal(info);
}
void VulkanGraphicsEngine::DestroyComputeShader(uint32_t a_addr)
{
    IVERIFY(m_computeShaders.Exists(a_addr));

    m_computeShaders.Erase(a_addr);
}
VulkanShaderInfo VulkanGraphicsEngine::GetComputeShaderInfo(uint32_t a_addr)
{
    IVERIFY(m_computeShaders.Exists(a_addr));

    return m_computeShaders[a_addr];
}
IcarianCore::Dictionary<IcarianCore::COWU8String, IcarianCore::COWU8String> VulkanGraphicsEngine::GetComputeShaderImports()
{
    const IcarianCore::SharedThreadGuard g = IcarianCore::SharedThreadGuard(m_importLock);

    return m_computeImports;
}

uint32_t VulkanGraphicsEngine::GenerateRenderProgram(const RenderProgram& a_program, IcarianCore::Allocator* a_tempAllocator)
{
    IcarianCore::Allocator* allocator = m_vulkanEngine->GetAllocator();

    TRACE("Creating Shader Program");
    // Create a VulkanRenderProgram from a generic RenderProgram
    RenderProgram p = a_program;
    p.Data = ILAMBDA(
    {
        VulkanRenderProgramBlob* val = allocator->ZTAllocate<VulkanRenderProgramBlob>();

        switch (a_program.MaterialMode)
        {
        case MaterialMode_BaseVertex:
        {
            IVERIFY(m_vertexShaders.Exists(a_program.VertexShader));
            IVERIFY(m_pixelShaders.Exists(a_program.PixelShader));

            VulkanBaseShaderDataBuilder baseBuilder;
            baseBuilder.Engine = m_vulkanEngine;
            baseBuilder.GraphicsEngine = this;
            baseBuilder.Program = a_program;

            val->Base = allocator->TAllocate<VulkanShaderData>();
            VulkanShaderData::CreateBaseShaderData(val->Base, baseBuilder, allocator, a_tempAllocator);

            if (a_program.ShadowVertexShader != uint32_t(-1))
            {
                IVERIFY(m_vertexShaders.Exists(a_program.ShadowVertexShader));

                VulkanShadowShaderDataBuilder shadowBuilder;
                shadowBuilder.Engine = m_vulkanEngine;
                shadowBuilder.GraphicsEngine = this;
                shadowBuilder.Program = a_program;

                val->Shadow = allocator->TAllocate<VulkanShaderData>();
                VulkanShaderData::CreateShadowShaderData(val->Shadow, shadowBuilder, allocator, a_tempAllocator);
            }

            break;
        }
        case MaterialMode_BaseMesh:
        {
            const bool isMeshEnabled = m_vulkanEngine->IsMeshEnabled();

            IVERIFY(m_meshShaders.Exists(a_program.VertexShader));
            if (!isMeshEnabled)
            {
                VulkanComputeMeshShaderDataBuilder meshBuilder;
                meshBuilder.Engine = m_vulkanEngine;
                meshBuilder.GraphicsEngine = this;
                meshBuilder.Program = a_program;

                val->Secondary = allocator->TAllocate<VulkanShaderData>();
                VulkanShaderData::CreateComputeMeshShaderData(val->Secondary, meshBuilder, allocator, a_tempAllocator);
            }

            if (a_program.ExtraShader != uint32_t(-1))
            {
                IVERIFY(m_taskShaders.Exists(a_program.ExtraShader));
            }

            if (a_program.ShadowVertexShader != uint32_t(-1))
            {
                IVERIFY(m_meshShaders.Exists(a_program.ShadowVertexShader));
            }

            IVERIFY(m_pixelShaders.Exists(a_program.PixelShader));

            VulkanBaseShaderDataBuilder baseBuilder;
            baseBuilder.Engine = m_vulkanEngine;
            baseBuilder.GraphicsEngine = this;
            baseBuilder.Program = a_program;

            val->Base = allocator->TAllocate<VulkanShaderData>();
            VulkanShaderData::CreateBaseShaderData(val->Base, baseBuilder, allocator, a_tempAllocator);

            break;
        }
        case MaterialMode_Compute:
        {
            IVERIFY(m_computeShaders.Exists(a_program.ExtraShader));

            VulkanBaseShaderDataBuilder baseBuilder;
            baseBuilder.Engine = m_vulkanEngine;
            baseBuilder.GraphicsEngine = this;
            baseBuilder.Program = a_program;

            val->Base = allocator->TAllocate<VulkanShaderData>();
            VulkanShaderData::CreateBaseShaderData(val->Base, baseBuilder, allocator, a_tempAllocator);

            break;
        }
        }

        ILRETURN val;
    });

    return m_shaderPrograms.PushVal(p);
}
void VulkanGraphicsEngine::DestroyRenderProgram(uint32_t a_addr)
{
    IVERIFY(m_shaderPrograms.Exists(a_addr));

    TRACE("Destroying Shader Program");

    RENDERSCRATCHFRAME;

    IcarianCore::Allocator* allocator = m_vulkanEngine->GetAllocator();
    IcarianCore::StackAllocator* scratchAllocator = RenderScratchAlloc::GetAllocator();

    const RenderProgram program = m_shaderPrograms[a_addr];
    IDEFER(
    {
        if (program.Data != nullptr)
        {
            VulkanRenderProgramBlob* blob = (VulkanRenderProgramBlob*)program.Data;
            IDEFER(allocator->Free(blob));

            if (blob->Base != nullptr)
            {
                allocator->Destroy(blob->Base);
            }
            if (blob->Shadow != nullptr)
            {
                allocator->Destroy(blob->Shadow);
            }
            if (blob->Secondary != nullptr)
            {
                allocator->Destroy(blob->Secondary);
            }
            if (blob->Tertiary != nullptr)
            {
                allocator->Destroy(blob->Tertiary);
            }
        }

        if (IISBITSET(program.Flags, RenderProgram::DestroyFlag))
        {
            switch (program.MaterialMode)
            {
            case MaterialMode_BaseVertex:
            {
                DestroyVertexShader(program.VertexShader);

                break;
            }
            case MaterialMode_BaseMesh:
            {
                DestroyMeshShader(program.VertexShader);

                if (program.ExtraShader != uint32_t(-1))
                {
                    DestroyTaskShader(program.ExtraShader);
                }

                break;
            }
            default:
            {
                IERROR("Invalid material mode");

                break;
            }
            }

            DestroyPixelShader(program.PixelShader);

            // TODO: Need to change this down the line to support mesh shaders
            if (program.ShadowVertexShader != uint32_t(-1))
            {
                DestroyVertexShader(program.ShadowVertexShader);
            }
        }
    });

    m_shaderPrograms.Erase(a_addr);

    {
        TLockArray<MaterialRenderStack*> a = m_renderStacks.ToLockArray();

        const uint32_t size = a.Size();
        for (uint32_t i = 0; i < size; ++i)
        {
            MaterialRenderStack* stack = a[i];

            const uint32_t matAddr = stack->GetMaterialAddr();

            if (matAddr == a_addr)
			{
                m_renderStacks.UErase(i);

                allocator->Destroy(stack);

                break;
			}
        }
    }

    {
        RENDERSCRATCHFRAME;

        const IcarianCore::ThreadGuard g = IcarianCore::ThreadGuard(m_pipeLock);

        const IcarianCore::Array<uint64_t> keys = m_pipelines.GetKeys(scratchAllocator);
        for (uint64_t k : keys)
        {
            const uint32_t val = (uint32_t)(k >> 32);
            if (val != a_addr)
            {
                continue;
            }

            VulkanPipeline* pipeline = m_pipelines[k];
            IDEFER(allocator->Destroy(pipeline));

            m_pipelines.Erase(k);
        }
    }

    if (program.ShadowVertexShader != uint32_t(-1))
    {
        {
            RENDERSCRATCHFRAME;

            const IcarianCore::ThreadGuard g = IcarianCore::ThreadGuard(m_shadowPipeLock);

            const IcarianCore::Array<uint64_t> keys = m_shadowPipelines.GetKeys(scratchAllocator);
            for (const uint64_t k : keys)
            {
                const uint32_t val = (uint32_t)(k >> 32);
                if (val != a_addr)
                {
                    continue;
                }

                VulkanPipeline* pipeline = m_shadowPipelines[k];
                IDEFER(allocator->Destroy(pipeline));

                m_shadowPipelines.Erase(k);
            }
        }

        {
            RENDERSCRATCHFRAME;

            const IcarianCore::ThreadGuard g = IcarianCore::ThreadGuard(m_cubeShadowPipeLock);

            const IcarianCore::Array<uint64_t> keys = m_cubeShadowPipelines.GetKeys(scratchAllocator);
            for (const uint64_t k : keys)
            {
                const uint32_t val = (uint32_t)(k >> 32);
                if (val != a_addr)
                {
                    continue;
                }

                VulkanPipeline* pipeline = m_cubeShadowPipelines[k];
                IDEFER(allocator->Destroy(pipeline));

                m_cubeShadowPipelines.Erase(k);
            }
        }
    }

    if (m_meshEmulationData != nullptr)
    {
        RENDERSCRATCHFRAME;

        const IcarianCore::ThreadGuard g = IcarianCore::ThreadGuard(m_meshPipelineLock);

        const IcarianCore::Array<uint64_t> keys = m_meshEmulationData->MeshPipelines.GetKeys(scratchAllocator);
        for (uint64_t k : keys)
        {
            if (k != a_addr)
            {
                continue;
            }

            VulkanPipeline* pipeline = m_meshEmulationData->MeshPipelines[k];
            IDEFER(allocator->Destroy(pipeline));

            m_meshEmulationData->MeshPipelines.Erase(k);
        }
    }
}

RenderProgram VulkanGraphicsEngine::GetRenderProgram(uint32_t a_addr)
{
    IVERIFY(m_shaderPrograms.Exists(a_addr));

    return m_shaderPrograms[a_addr];
}

VulkanPipeline* VulkanGraphicsEngine::GetShadowPipeline(uint32_t a_renderTexture, uint32_t a_pipeline)
{
    IVERIFY(m_shaderPrograms.Exists(a_pipeline));

    RENDERSCRATCHFRAME;
    IcarianCore::StackAllocator* scratchAllocator = RenderScratchAlloc::GetAllocator();

    const uint64_t addr = (uint64_t)a_renderTexture | (uint64_t)a_pipeline << 32;

    const IcarianCore::ThreadGuard g = IcarianCore::ThreadGuard(m_shadowPipeLock);
    if (m_shadowPipelines.Exists(addr))
    {
        return m_shadowPipelines[addr];
    }

    TRACE("Allocating Vulkan Shadow Pipeline");
    const VulkanDepthRenderTexture* tex = GetDepthRenderTexture(a_renderTexture);
    IVERIFY(tex != nullptr);

    const vk::RenderPass pass = tex->GetRenderPass();

    IcarianCore::Allocator* allocator = m_vulkanEngine->GetAllocator();

    const VulkanGraphicsPipelineBuilder builder =
    {
        .Engine = m_vulkanEngine,
        .GraphicsEngine = this,
        .RenderPass = pass,
        .ProgramAddr = a_pipeline,
    };

    VulkanPipeline* pipeline = allocator->TAllocate<VulkanPipeline>();
    VulkanPipeline::CreateShadowPipeline(pipeline, builder, scratchAllocator);

    m_shadowPipelines.Push(addr, pipeline);

    return pipeline;
}
VulkanPipeline* VulkanGraphicsEngine::GetCubeShadowPipeline(uint32_t a_renderTexture, uint32_t a_pipeline)
{
    IVERIFY(m_shaderPrograms.Exists(a_pipeline));

    RENDERSCRATCHFRAME;
    IcarianCore::StackAllocator* scratchAllocator = RenderScratchAlloc::GetAllocator();

    const uint64_t addr = (uint64_t)a_renderTexture | (uint64_t)a_pipeline << 32;

    const IcarianCore::ThreadGuard g = IcarianCore::ThreadGuard(m_cubeShadowPipeLock);
    if (m_cubeShadowPipelines.Exists(addr))
    {
        return m_cubeShadowPipelines[addr];
    }

    TRACE("Allocating Vulkan Cube Shadow Pipeline");
    const VulkanDepthCubeRenderTexture* tex = GetDepthCubeRenderTexture(a_renderTexture);
    IVERIFY(tex != nullptr);

    const vk::RenderPass pass = tex->GetRenderPass();

    IcarianCore::Allocator* allocator = m_vulkanEngine->GetAllocator();

    const VulkanGraphicsPipelineBuilder builder =
    {
        .Engine = m_vulkanEngine,
        .GraphicsEngine = this,
        .RenderPass = pass,
        .ProgramAddr = a_pipeline,
    };

    VulkanPipeline* pipeline = allocator->TAllocate<VulkanPipeline>();
    VulkanPipeline::CreateShadowPipeline(pipeline, builder, scratchAllocator);

    m_cubeShadowPipelines.Push(addr, pipeline);

    return pipeline;
}
VulkanPipeline* VulkanGraphicsEngine::GetPipeline(uint32_t a_renderTexture, uint32_t a_pipeline)
{
    IVERIFY(m_shaderPrograms.Exists(a_pipeline));

    const uint64_t addr = (uint64_t)a_renderTexture | (uint64_t)a_pipeline << 32;

    const IcarianCore::ThreadGuard g = IcarianCore::ThreadGuard(m_pipeLock);
    if (m_pipelines.Exists(addr))
    {
        return m_pipelines[addr];
    }

    RENDERSCRATCHFRAME;

    IcarianCore::StackAllocator* scratchAllocator = RenderScratchAlloc::GetAllocator();

    const VulkanRenderTexture* tex = GetRenderTexture(a_renderTexture);
    const RenderProgram program = m_shaderPrograms[a_pipeline];

    TRACE("Allocating Vulkan Pipeline");
    IcarianCore::Allocator* allocator = m_vulkanEngine->GetAllocator();
    VulkanPipeline* pipeline = allocator->TAllocate<VulkanPipeline>();

    switch (program.MaterialMode)
    {
    case MaterialMode_BaseMesh:
    case MaterialMode_BaseVertex:
    {
        const vk::RenderPass renderPass = ILAMBDA(
        {
            if (tex != nullptr)
            {
                ILRETURN tex->GetRenderPass();
            }

            ILRETURN m_swapchain->GetRenderPass();
        });

        const bool hasDepth = ILAMBDA(
        {
            if (tex != nullptr)
            {
                ILRETURN tex->HasDepthTexture();
            }

            ILRETURN false;
        });

        const uint32_t textureCount = ILAMBDA(
        {
            if (tex != nullptr)
            {
                ILRETURN tex->GetTextureCount();
            }

            ILRETURN uint32_t(1);
        });

        const VulkanGraphicsPipelineBuilder builder =
        {
            .Engine = m_vulkanEngine,
            .GraphicsEngine = this,
            .RenderPass = renderPass,
            .TextureCount = textureCount,
            .ProgramAddr = a_pipeline,
            .Depth = hasDepth,
        };

        VulkanPipeline::CreatePipeline(pipeline, builder, scratchAllocator);

        break;
    }
    case MaterialMode_Compute:
    {
        const VulkanGraphicsComputePipelineBuilder builder =
        {
            .Engine = m_vulkanEngine,
            .GraphicsEngine = this,
            .ProgramAddr = a_pipeline
        };

        VulkanPipeline::CreateComputePipeline(pipeline, builder);

        break;
    }
    default:
    {
        IERROR("Invalid material mode");

        break;
    }
    }

    m_pipelines.Push(addr, pipeline);

    return pipeline;
}
VulkanPipeline* VulkanGraphicsEngine::GetComputeMeshPipeline(uint32_t a_pipeline)
{
    IVERIFY(m_meshEmulationData != nullptr);

    const uint64_t addr = (uint64_t)a_pipeline;

    const IcarianCore::ThreadGuard g = IcarianCore::ThreadGuard(m_meshPipelineLock);
    if (m_meshEmulationData->MeshPipelines.Exists(addr))
    {
        return m_meshEmulationData->MeshPipelines[addr];
    }

    TRACE("Allocating Mesh Emulation Vulkan Mesh Pipeline");
    IcarianCore::Allocator* allocator = m_vulkanEngine->GetAllocator();
    VulkanPipeline* pipeline = allocator->TAllocate<VulkanPipeline>();

    const VulkanGraphicsComputePipelineBuilder builder =
    {
        .Engine = m_vulkanEngine,
        .GraphicsEngine = this,
        .ProgramAddr = a_pipeline,
    };
    VulkanPipeline::CreateMeshComputePipeline(pipeline, builder);

    m_meshEmulationData->MeshPipelines.Push(addr, pipeline);

    return pipeline;
}

vk::CommandBuffer VulkanGraphicsEngine::StartCommandBuffer(uint32_t a_bufferIndex, uint32_t a_index) const
{
    const vk::CommandBuffer commandBuffer = (*m_commandBuffers[a_index])[a_bufferIndex];

    constexpr vk::CommandBufferBeginInfo BeginInfo;
    commandBuffer.begin(BeginInfo);

    return commandBuffer;
}

void VulkanGraphicsEngine::Draw(bool a_forward, const CameraBuffer& a_camBuffer, const Frustum& a_frustum, VulkanRenderCommand* a_renderCommand, uint32_t a_frameIndex)
{
    RENDERSCRATCHFRAME;

    IcarianCore::StackAllocator* scratchAllocator = RenderScratchAlloc::GetAllocator();

    const vk::CommandBuffer commandBuffer = a_renderCommand->GetCommandBuffer();

    const bool meshEnabled = m_vulkanEngine->IsMeshEnabled();

    const IcarianCore::Array<MaterialRenderStack*> stacks = m_renderStacks.ToArray(scratchAllocator);
    const IcarianCore::Array<RenderProgram> shaderPrograms = m_shaderPrograms.ToArray(scratchAllocator);

    for (const MaterialRenderStack* renderStack : stacks)
    {
        const uint32_t matAddr = renderStack->GetMaterialAddr();

        const RenderProgram& program = shaderPrograms[matAddr];
        IVERIFY(program.Data != nullptr);

        const bool shareLayer = (a_camBuffer.RenderLayer & program.RenderLayer) != 0;
        if (!shareLayer)
        {
            continue;
        }

        if (a_forward && program.ColorBlendMode == MaterialBlendMode_None)
        {
            continue;
        }
        else if (!a_forward && program.ColorBlendMode != MaterialBlendMode_None)
        {
            continue;
        }

        const uint32_t modelCount = renderStack->GetModelBufferCount();
        const ModelBuffer* modelBuffers = renderStack->GetModelBuffers();

        const e_RenderStackMode renderStackMode = renderStack->GetRenderStackMode();
        switch (renderStackMode)
        {
        case RenderStackMode_Model:
        {
            PROFILESTACK("Models");

            RENDERSCRATCHFRAME;

            const VulkanPipeline* pipeline = a_renderCommand->BindMaterial(matAddr, true, scratchAllocator);
            if (pipeline == nullptr)
            {
                IERROR("Failed to bind material");
            }

            const VulkanShaderData* shaderData = pipeline->GetShaderData();

            for (uint32_t i = 0; i < modelCount; ++i)
            {
                RENDERSCRATCHFRAME;

                const ModelBuffer& modelBuffer = modelBuffers[i];
                const bool valid = modelBuffer.ModelAddr != uint32_t(-1);
                if (!valid)
                {
                    continue;
                }

                const VulkanModel* model = GetModel(modelBuffer.ModelAddr);
                IVERIFY(model != nullptr);

                const float radius = model->GetRadius();
                const uint32_t indexCount = model->GetIndexCount();

                glm::mat4* transforms = scratchAllocator->TAllocate<glm::mat4>(modelBuffer.TransformCount);
                uint32_t transformCount = 0;

                {
                    PROFILESTACK("Culling");

                    for (uint32_t j = 0; j < modelBuffer.TransformCount; ++j)
                    {
                        const uint32_t transformAddr = modelBuffer.TransformAddr[j];
                        if (transformAddr != uint32_t(-1))
                        {
                            const glm::mat4 transform = ObjectManager::GetGlobalMatrix(transformAddr);
                            glm::vec3 scale;
                            glm::quat rotation;
                            glm::vec3 translation;
                            glm::vec3 s;
                            glm::vec4 p;
                            glm::decompose(transform, scale, rotation, translation, s, p);

                            const float sFactor = glm::max(scale.x, glm::max(scale.y, scale.z));

                            if (a_frustum.CompareSphere(translation, radius * sFactor))
                            {
                                transforms[transformCount++] = transform;
                            }
                        }
                    }
                }

                PROFILESTACK("Draw");
                if (transformCount == 0)
                {
                    continue;
                }

                VULKAN_MARKER(m_vulkanEngine, commandBuffer, "Models");

                model->Bind(commandBuffer);

                ShaderBufferInput modelSlot;
                if (shaderData->GetShaderBufferInput(ShaderBufferType_SSModelBuffer, &modelSlot))
                {
                    IcarianCore::ShaderModelBuffer* modelBuffer = scratchAllocator->TAllocate<IcarianCore::ShaderModelBuffer>(transformCount);

                    for (uint32_t j = 0; j < transformCount; ++j)
                    {
                        const glm::mat4& mat = transforms[j];
                        modelBuffer[j].Model = mat;
                        modelBuffer[j].InvModel = glm::inverse(mat);
                    }

                    const VulkanShaderStorageObject storage = VulkanShaderStorageObject
                    (
                        m_vulkanEngine,
                        sizeof(IcarianCore::ShaderModelBuffer) * transformCount,
                        transformCount,
                        modelBuffer
                    );

                    shaderData->PushShaderStorageObject(commandBuffer, modelSlot.RealSlot, &storage, a_frameIndex);

                    commandBuffer.drawIndexed(indexCount, transformCount, 0, 0, 0);
                }
                else
                {
                    for (uint32_t i = 0; i < transformCount; ++i)
                    {
                        shaderData->UpdateTransformBuffer(commandBuffer, transforms[i]);

                        commandBuffer.drawIndexed(indexCount, 1, 0, 0, 0);
                    }
                }
            }

            break;
        }
        case RenderStackMode_Skinned:
        {
            PROFILESTACK("Skinned");

            RENDERSCRATCHFRAME;

            const VulkanPipeline* pipeline = a_renderCommand->BindMaterial(matAddr, true, scratchAllocator);
            if (pipeline == nullptr)
            {
                IERROR("Failed to bind material");
            }

            const VulkanShaderData* shaderData = pipeline->GetShaderData();

            for (uint32_t i = 0; i < modelCount; ++i)
            {
                const ModelBuffer& modelBuffer = modelBuffers[i];
                const bool valid = modelBuffer.ModelAddr != uint32_t(-1);
                if (!valid)
                {
                    continue;
                }

                const VulkanModel* model = GetModel(modelBuffer.ModelAddr);
                IVERIFY(model != nullptr);

                bool modelBound = false;

                const float radius = model->GetRadius();
                const uint32_t indexCount = model->GetIndexCount();

                const uint32_t objectCount = modelBuffer.TransformCount;
                for (uint32_t j = 0; j < objectCount; ++j)
                {
                    RENDERSCRATCHFRAME;

                    const uint32_t transformAddr = modelBuffer.TransformAddr[j];
                    const bool valid = transformAddr != uint32_t(-1);
                    if (!valid)
                    {
                        continue;
                    }

                    const glm::mat4 transform = ObjectManager::GetGlobalMatrix(transformAddr);
                    const glm::vec3 position = transform[3].xyz();

                    if (!a_frustum.CompareSphere(position, radius))
                    {
                        continue;
                    }

                    VULKAN_MARKER(m_vulkanEngine, commandBuffer, "Skinned Models");

                    if (!modelBound)
                    {
                        model->Bind(commandBuffer);

                        modelBound = true;
                    }

                    ShaderBufferInput boneSlot;
                    if (shaderData->GetShaderBufferInput(ShaderBufferType_SSBoneBuffer, &boneSlot))
                    {
                        const SkeletonData skeleton = AnimationController::GetSkeleton(modelBuffer.SkeletonAddr[j]);
                        const uint32_t boneCount = (uint32_t)skeleton.BoneData.size();

                        typedef std::unordered_map
                        <
                            uint32_t, uint32_t,
                            std::hash<uint32_t>,
                            std::equal_to<uint32_t>,
                            STLRenderScratchAlloc<std::pair<const uint32_t, uint32_t>>
                        > BoneMap;

                        BoneMap boneMap;
                        boneMap.reserve(boneCount);
                        for (uint32_t i = 0; i < boneCount; ++i)
                        {
                            boneMap.emplace(skeleton.BoneData[i].TransformIndex, i);
                        }

                        IcarianCore::ShaderBoneBuffer* boneBuffer = scratchAllocator->TAllocate<IcarianCore::ShaderBoneBuffer>(boneCount);

                        for (uint32_t k = 0; k < boneCount; ++k)
                        {
                            const BoneTransformData& bone = skeleton.BoneData[k];
                            const TransformBuffer buffer = ObjectManager::GetTransformBuffer(bone.TransformIndex);

                            glm::mat4 transform = buffer.ToMat4();
                            auto iter = boneMap.find(buffer.ParentAddr);
                            while (iter != boneMap.end())
                            {
                                const uint32_t index = iter->second;

                                const BoneTransformData& parentBone = skeleton.BoneData[index];
                                const TransformBuffer parentBuffer = ObjectManager::GetTransformBuffer(parentBone.TransformIndex);

                                transform = parentBuffer.ToMat4() * transform;
                                iter = boneMap.find(parentBuffer.ParentAddr);
                            }

                            boneBuffer[k].BoneMatrix = transform * bone.InverseBindPose;
                        }

                        const VulkanShaderStorageObject storage = VulkanShaderStorageObject
                        (
                            m_vulkanEngine,
                            sizeof(IcarianCore::ShaderBoneBuffer) * boneCount,
                            boneCount,
                            boneBuffer
                        );

                        shaderData->PushShaderStorageObject(commandBuffer, boneSlot.RealSlot, &storage, a_frameIndex);
                    }

                    shaderData->UpdateTransformBuffer(commandBuffer, transform);

                    commandBuffer.drawIndexed(indexCount, 1, 0, 0, 0);
                }
            }

            break;
        }
        case RenderStackMode_Mesh:
        {
            PROFILESTACK("Mesh");

            RENDERSCRATCHFRAME;

            if (a_renderCommand->BindMaterial(matAddr, meshEnabled, scratchAllocator) == nullptr)
            {
                IERROR("Failed to bind material");
            }

            for (uint32_t i = 0; i < modelCount; ++i)
            {
                RENDERSCRATCHFRAME;

                const ModelBuffer& modelBuffer = modelBuffers[i];

                const VulkanMesh* mesh = GetMesh(modelBuffer.ModelAddr);

                const uint32_t indexCount = ILAMBDA(
                {
                    if (modelBuffer.IndexCount != uint32_t(-1))
                    {
                        ILRETURN modelBuffer.IndexCount;
                    }

                    if (mesh != nullptr)
                    {
                        ILRETURN mesh->GetMeshletCount();
                    }

                    ILRETURN uint32_t(0);
                });

                if (indexCount <= 0)
                {
                    continue;
                }

                // Possibly have a radius override incase it does not have a mesh attached but still wants culling?
                const float radius = ILAMBDA(
                {
                    if (mesh != nullptr)
                    {
                        const float val = mesh->GetRadius();
                        if (val > 0)
                        {
                            ILRETURN val;
                        }
                    }

                    ILRETURN std::numeric_limits<float>::infinity();
                });

                glm::mat4* transforms = scratchAllocator->TAllocate<glm::mat4>(modelBuffer.TransformCount);
                uint32_t transformCount = 0;

                {
                    PROFILESTACK("Culling");

                    for (uint32_t j = 0; j < modelBuffer.TransformCount; ++j)
                    {
                        const uint32_t transformAddr = modelBuffer.TransformAddr[j];
                        if (transformAddr != uint32_t(-1))
                        {
                            const glm::mat4 transform = ObjectManager::GetGlobalMatrix(transformAddr);
                            glm::vec3 scale;
                            glm::quat rotation;
                            glm::vec3 translation;
                            glm::vec3 s;
                            glm::vec4 p;
                            glm::decompose(transform, scale, rotation, translation, s, p);

                            const float sFactor = glm::max(scale.x, glm::max(scale.y, scale.z));

                            if (a_frustum.CompareSphere(translation, radius * sFactor))
                            {
                                transforms[transformCount++] = transform;
                            }
                        }
                    }
                }

                PROFILESTACK("Draw");
                if (transformCount <= 0)
                {
                    continue;
                }

                const VulkanRenderProgramBlob* blob = (VulkanRenderProgramBlob*)program.Data;
                IVERIFY(blob->Base != nullptr);

                const VulkanShaderData* shaderData = blob->Base;

                shaderData->PushMeshBuffers(commandBuffer, mesh, a_frameIndex);

                VULKAN_MARKER(m_vulkanEngine, commandBuffer, "Mesh");

                if (meshEnabled)
                {
                    ShaderBufferInput modelSlot;
                    if (shaderData->GetShaderBufferInput(ShaderBufferType_SSModelBuffer, &modelSlot))
                    {
                        RENDERSCRATCHFRAME;

                        const IcarianCore::ShaderModelBuffer* modelBuffer = ILAMBDA(
                        {
                            IcarianCore::ShaderModelBuffer* vals = scratchAllocator->TAllocate<IcarianCore::ShaderModelBuffer>(transformCount);

                            for (uint32_t i = 0; i < transformCount; ++i)
                            {
                                const glm::mat4& mat = transforms[i];

                                vals[i].Model = mat;
                                vals[i].InvModel = glm::inverse(mat);
                            }

                            ILRETURN vals;
                        });

                        const VulkanShaderStorageObject storage = VulkanShaderStorageObject
                        (
                            m_vulkanEngine,
                            sizeof(IcarianCore::ShaderModelBuffer) * transformCount,
                            transformCount,
                            modelBuffer
                        );

                        shaderData->PushShaderStorageObject(commandBuffer, modelSlot.RealSlot, &storage, a_frameIndex);

                        // Does not support instancing does Y make sense for instance data?
                        commandBuffer.drawMeshTasksEXT(indexCount, transformCount, 1);
                    }
                    else
                    {
                        for (uint32_t i = 0; i < transformCount; ++i)
                        {
                            shaderData->UpdateTransformBuffer(commandBuffer, transforms[i]);

                            commandBuffer.drawMeshTasksEXT(indexCount, 1, 1);
                        }
                    }
                }
                else
                {
                    IVERIFY(m_meshEmulationData != nullptr);

                    for (uint32_t j = 0; j < transformCount; ++j)
                    {
                        RENDERSCRATCHFRAME;

                        a_renderCommand->DrawMesh(transforms[i], modelBuffer.ModelAddr, indexCount, scratchAllocator);
                    }
                }
            }

            break;
        }
        default:
        {
            IERROR("Invalid Render Stack Mode");

            break;
        }
        }
    }
}
void VulkanGraphicsEngine::DrawShadow
(
    const glm::mat4& a_lvp,
    float a_split,
    const glm::vec2& a_bias,
    uint32_t a_renderLayer,
    uint32_t a_renderTexture,
    bool a_cube,
    vk::CommandBuffer a_commandBuffer,
    uint32_t a_frameIndex
)
{
    PROFILESTACK("Rendering");

    RENDERSCRATCHFRAME;

    IcarianCore::StackAllocator* scratchAllocator = RenderScratchAlloc::GetAllocator();

    VulkanUniformBuffer* shadowLightBuffer = nullptr;

    VulkanPushPool* pushPool = m_vulkanEngine->GetPushPool();

    const Frustum frustum = Frustum::FromMat4(a_lvp);

    const IcarianCore::Array<MaterialRenderStack*> stacks = m_renderStacks.ToArray(scratchAllocator);
    const IcarianCore::Array<RenderProgram> shaderPrograms = m_shaderPrograms.ToArray(scratchAllocator);

    for (const MaterialRenderStack* renderStack : stacks)
    {
        const uint32_t materialAddr = renderStack->GetMaterialAddr();
        const RenderProgram& program = shaderPrograms[materialAddr];
        IVERIFY(program.Data != nullptr);

        const bool sharesLayer = (a_renderLayer & program.RenderLayer) != 0;
        if (!sharesLayer)
        {
            continue;
        }

        if (program.ShadowVertexShader == uint32_t(-1))
        {
            continue;
        }

        const VulkanRenderProgramBlob* blob = (VulkanRenderProgramBlob*)program.Data;
        IVERIFY(blob->Shadow != nullptr);

        VulkanShaderData* shaderData = blob->Shadow;
        bool pipelineBound = false;

        ShaderBufferInput shadowLightInput;
        if (shaderData->GetShaderBufferInput(ShaderBufferType_ShadowLightBuffer, &shadowLightInput))
        {
            if (shadowLightBuffer == nullptr)
            {
                shadowLightBuffer = pushPool->AllocateShadowUniformBuffer();

                const IcarianCore::ShaderShadowLightBuffer buffer =
                {
                    .LVP = a_lvp,
                    .Split = a_split
                };

                shadowLightBuffer->SetData(a_frameIndex, &buffer);
            }

            shaderData->PushUniformBuffer(a_commandBuffer, shadowLightInput.RealSlot, shadowLightBuffer, a_frameIndex);
        }
        else
        {
            shaderData->UpdateShadowLightBuffer(a_commandBuffer, a_lvp, a_split);
        }

        const uint32_t modelCount = renderStack->GetModelBufferCount();
        const ModelBuffer* modelBuffers = renderStack->GetModelBuffers();

        const e_RenderStackMode stackMode = renderStack->GetRenderStackMode();
        switch (stackMode)
        {
        case RenderStackMode_Model:
        {
            PROFILESTACK("Models");

            for (uint32_t i = 0; i < modelCount; ++i)
            {
                RENDERSCRATCHFRAME;

                const ModelBuffer& modelBuffer = modelBuffers[i];

                if (modelBuffer.ModelAddr != uint32_t(-1))
                {
                    const VulkanModel* model = GetModel(modelBuffer.ModelAddr);
                    IVERIFY(model != nullptr);

                    const uint32_t indexCount = model->GetIndexCount();
                    const float radius = model->GetRadius();

                    const uint32_t transformCount = modelBuffer.TransformCount;

                    glm::mat4* transforms = scratchAllocator->TAllocate<glm::mat4>(transformCount);

                    uint32_t finalTransformCount = 0;

                    {
                        PROFILESTACK("Culling");
                        for (uint32_t j = 0; j < transformCount; ++j)
                        {
                            const uint32_t transformAddr = modelBuffer.TransformAddr[j];
                            if (transformAddr == uint32_t(-1))
                            {
                                continue;
                            }

                            const glm::mat4 transform = ObjectManager::GetGlobalMatrix(transformAddr);
                            glm::vec3 scale;
                            glm::quat rotation;
                            glm::vec3 translation;
                            glm::vec3 s;
                            glm::vec4 p;
                            glm::decompose(transform, scale, rotation, translation, s, p);

                            const float sFactor = glm::max(scale.x, glm::max(scale.y, scale.z));

                            if (frustum.CompareSphere(translation, radius * sFactor))
                            {
                                // Scale slightly to improve shadows around edges of objects
                                transforms[finalTransformCount++] = transform;
                            }
                        }
                    }

                    PROFILESTACK("Draw");

                    if (finalTransformCount <= 0)
                    {
                        continue;
                    }

                    VULKAN_MARKER(m_vulkanEngine, a_commandBuffer, "Models");

                    if (!pipelineBound)
                    {
                        pipelineBound = true;

                        const VulkanPipeline* pipeline = ILAMBDA(
                        {
                            if (a_cube)
                            {
                                ILRETURN GetCubeShadowPipeline(a_renderTexture, materialAddr);
                            }

                            ILRETURN GetShadowPipeline(a_renderTexture, materialAddr);
                        });

                        pipeline->Bind(a_frameIndex, a_commandBuffer);

                        a_commandBuffer.setDepthBias(a_bias.x, 0.0f, a_bias.y);
                    }

                    model->Bind(a_commandBuffer);

                    ShaderBufferInput modelSlot;
                    if (shaderData->GetShaderBufferInput(ShaderBufferType_SSModelBuffer, &modelSlot))
                    {
                        IcarianCore::ShaderModelBuffer* modelBuffer = scratchAllocator->TAllocate<IcarianCore::ShaderModelBuffer>(finalTransformCount);
                        for (uint32_t j = 0; j < finalTransformCount; ++j)
                        {
                            const glm::mat4& mat = transforms[j];

                            modelBuffer[j].Model = mat;
                            modelBuffer[j].InvModel = glm::inverse(mat);
                        }

                        const VulkanShaderStorageObject storage = VulkanShaderStorageObject
                        (
                            m_vulkanEngine,
                            sizeof(IcarianCore::ShaderModelBuffer) * finalTransformCount,
                            finalTransformCount,
                            modelBuffer
                        );

                        shaderData->PushShaderStorageObject(a_commandBuffer, modelSlot.RealSlot, &storage, a_frameIndex);

                        a_commandBuffer.drawIndexed(indexCount, finalTransformCount, 0, 0, 0);
                    }
                    else
                    {
                        for (uint32_t i = 0; i < finalTransformCount; ++i)
                        {
                            shaderData->UpdateTransformBuffer(a_commandBuffer, transforms[i]);

                            a_commandBuffer.drawIndexed(indexCount, 1, 0, 0, 0);
                        }
                    }
                }
            }

            break;
        }
        case RenderStackMode_Skinned:
        {
            PROFILESTACK("Skinned");

            for (uint32_t i = 0; i < modelCount; ++i)
            {
                const ModelBuffer& modelBuffer = modelBuffers[i];

                if (modelBuffer.ModelAddr == uint32_t(-1))
                {
                    continue;
                }

                const VulkanModel* model = nullptr;

                const uint32_t indexCount = model->GetIndexCount();
                const float radius = model->GetRadius();

                const uint32_t objectCont = modelBuffer.TransformCount;
                for (uint32_t j = 0; j < objectCont; ++j)
                {
                    const uint32_t transformAddr = modelBuffer.TransformAddr[j];
                    if (transformAddr == uint32_t(-1))
                    {
                        continue;
                    }

                    const glm::mat4 transform = ObjectManager::GetGlobalMatrix(transformAddr);
                    const glm::vec3 pos = transform[3].xyz();

                    if (!frustum.CompareSphere(pos, radius))
                    {
                        continue;
                    }

                    VULKAN_MARKER(m_vulkanEngine, a_commandBuffer, "Skinned Models");

                    if (!pipelineBound)
                    {
                        pipelineBound = true;

                        const VulkanPipeline* pipeline = ILAMBDA(
                        {
                            if (a_cube)
                            {
                                ILRETURN GetCubeShadowPipeline(a_renderTexture, materialAddr);
                            }

                            ILRETURN GetShadowPipeline(a_renderTexture, materialAddr);
                        });

                        pipeline->Bind(a_frameIndex, a_commandBuffer);
                    }

                    if (model == nullptr)
                    {
                        model = GetModel(modelBuffer.ModelAddr);
                        IVERIFY(model != nullptr);

                        model->Bind(a_commandBuffer);
                    }

                    ShaderBufferInput boneSlot;
                    if (shaderData->GetShaderBufferInput(ShaderBufferType_SSBoneBuffer, &boneSlot))
                    {
                        RENDERSCRATCHFRAME;

                        const SkeletonData skeleton = AnimationController::GetSkeleton(modelBuffer.SkeletonAddr[j]);
                        const uint32_t boneCount = (uint32_t)skeleton.BoneData.size();

                        typedef std::unordered_map
                        <
                            uint32_t, uint32_t,
                            std::hash<uint32_t>,
                            std::equal_to<uint32_t>,
                            STLRenderScratchAlloc<std::pair<const uint32_t, uint32_t>>
                        > BoneMap;

                        BoneMap boneMap;
                        boneMap.reserve(boneCount);
                        for (uint32_t i = 0; i < boneCount; ++i)
                        {
                            boneMap.emplace(skeleton.BoneData[i].TransformIndex, i);
                        }

                        IcarianCore::ShaderBoneBuffer* boneBuffer = scratchAllocator->TAllocate<IcarianCore::ShaderBoneBuffer>(boneCount);
                        for (uint32_t k = 0; k < boneCount; ++k)
                        {
                            const BoneTransformData& bone = skeleton.BoneData[k];
                            const TransformBuffer buffer = ObjectManager::GetTransformBuffer(bone.TransformIndex);

                            glm::mat4 transform = buffer.ToMat4();

                            auto iter = boneMap.find(buffer.ParentAddr);
                            while (iter != boneMap.end())
                            {
                                const uint32_t index = iter->second;

                                const BoneTransformData& parentBone = skeleton.BoneData[index];
                                const TransformBuffer parentBuffer = ObjectManager::GetTransformBuffer(parentBone.TransformIndex);

                                transform = parentBuffer.ToMat4() * transform;

                                iter = boneMap.find(parentBuffer.ParentAddr);
                            }

                            boneBuffer[k].BoneMatrix = transform * bone.InverseBindPose;
                        }

                        const VulkanShaderStorageObject storage = VulkanShaderStorageObject
                        (
                            m_vulkanEngine,
                            sizeof(IcarianCore::ShaderBoneBuffer) * boneCount,
                            boneCount,
                            boneBuffer
                        );

                        shaderData->PushShaderStorageObject(a_commandBuffer, boneSlot.RealSlot, &storage, a_frameIndex);
                    }

                    shaderData->UpdateTransformBuffer(a_commandBuffer, transform);

                    a_commandBuffer.drawIndexed(indexCount, 1, 0, 0, 0);
                }
            }

            break;
        }
        default:
        {
            IERROR("Invalid render stack mode");

            break;
        }
        }
    }
}

VulkanCommandBuffer VulkanGraphicsEngine::DirectionalShadowPass(uint32_t a_camIndex, uint32_t a_bufferIndex, uint32_t a_frameIndex)
{
    RENDERSCRATCHFRAME;

    IcarianCore::StackAllocator* scratchAllocator = RenderScratchAlloc::GetAllocator();

    VulkanCommandBuffer vCmdBuffer = VulkanCommandBuffer(nullptr, VulkanCommandBufferType_Graphics, VulkanCommandBufferStage_ShadowPass);

    // Could possibly reverse the pass order and do culling on the previous pass to speed this up
    Profiler::Start("Dir Shadow Pass");
    IDEFER(Profiler::Stop());

    Profiler::StartFrame("Update");

    if (m_directionalLights.Empty())
    {
        Profiler::StopFrame();

        return vCmdBuffer;
    }

    const CameraBuffer& camBuffer = m_cameraBuffers[a_camIndex];

    const vk::CommandBuffer commandBuffer = StartCommandBuffer(a_bufferIndex, a_frameIndex);
    IDEFER(commandBuffer.end());
    vCmdBuffer.SetCommandBuffer(commandBuffer);

    VulkanProfiler::StartTimingPoint(vCmdBuffer, "Directional Shadow Pass");

    VULKAN_MARKER_COL(m_vulkanEngine, commandBuffer, "Directional Shadow Pass", 255, 0, 0);

    VulkanRenderCommand& renderCommand = m_renderCommands.Push(VulkanRenderCommand
    (
        m_vulkanEngine,
        this,
        m_swapchain,
        commandBuffer,
        -1,
        a_bufferIndex
    ));
    VulkanLightData& lightData = m_lightData.Push(VulkanLightData());

    e_LightType lightType = LightType_Directional;

    void* shadowSetupArgs[] =
    {
        &lightType,
        &a_camIndex
    };

    {
        PROFILESTACK("Shadow Setup");

        m_shadowSetupFunc->Exec(shadowSetupArgs);

        VulkanProfiler::PushTimingPoint(vCmdBuffer, "Shadow Setup");
    }

    const uint32_t size = m_directionalLights.Size();
    const IcarianCore::Array<uint8_t> state = m_directionalLights.ToPackedStateArray(scratchAllocator);
    const IcarianCore::Array<DirectionalLightBuffer> lights = m_directionalLights.ToArray(scratchAllocator);

    for (uint32_t i = 0; i < size; ++i)
    {
        const uint32_t index = i / 8;
        const uint32_t offset = i % 8;

        if (!IISBITSET(state[index], offset))
        {
            continue;
        }

        const DirectionalLightBuffer& buffer = lights[i];
        if (buffer.TransformAddr == uint32_t(-1) || (buffer.RenderLayer & camBuffer.RenderLayer) == 0 || buffer.Intensity <= 0.0f)
        {
            continue;
        }

        IVERIFY(buffer.Data != nullptr);
        const VulkanLightBuffer* lightBuffer = (VulkanLightBuffer*)buffer.Data;

        for (uint32_t j = 0; j < lightBuffer->LightRenderTextureCount; ++j)
        {
            void* shadowArgs[] =
            {
                &lightType,
                &i,
                &a_camIndex,
                &j
            };

            {
                PROFILESTACK("Pre Shadow");

                m_preShadowFunc->Exec(shadowArgs);

                VulkanProfiler::PushTimingPoint(vCmdBuffer, "Pre Shadow");
            }

            const uint32_t splitCount = lightData.GetSplitCount();
            const LightShadowSplit* splits = lightData.GetSplits();
            // Hmm I am an idiot it can be null briefly after being deleted so skip if that happens
            if (splitCount <= 0 || splits == nullptr)
            {
                continue;
            }

            const uint32_t lightRenderTexture = lightBuffer->LightRenderTextures[j];
            const VulkanDepthRenderTexture* depthRenderTexture = GetDepthRenderTexture(lightRenderTexture);

            const glm::vec2 screenSize = glm::vec2(depthRenderTexture->GetWidth(), depthRenderTexture->GetHeight());

            constexpr vk::ClearValue ClearDepth = vk::ClearValue(vk::ClearDepthStencilValue(1.0f, 0));

            const vk::Rect2D scissor = vk::Rect2D({ 0, 0 }, { (uint32_t)screenSize.x, (uint32_t)screenSize.y });
            const vk::Viewport viewport = vk::Viewport(0.0f, 0.0f, screenSize.x, screenSize.y, 0.0f, 1.0f);

            const vk::RenderPassBeginInfo renderPassInfo = vk::RenderPassBeginInfo
            (
                depthRenderTexture->GetRenderPass(),
                depthRenderTexture->GetFrameBuffer(),
                scissor,
                1,
                &ClearDepth
            );

            commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
            IDEFER(commandBuffer.endRenderPass());

            commandBuffer.setScissor(0, 1, &scissor);
            commandBuffer.setViewport(0, 1, &viewport);

            DrawShadow
            (
                splits[0].LVP,
                splits[0].Split,
                buffer.ShadowBias,
                buffer.RenderLayer,
                lightRenderTexture,
                false,
                commandBuffer,
                a_frameIndex
            );

            VulkanProfiler::PushTimingPoint(vCmdBuffer, "Draw");

            {
                PROFILESTACK("Post Shadow");

                m_postShadowFunc->Exec(shadowArgs);

                VulkanProfiler::PushTimingPoint(vCmdBuffer, "Post Shadow");
            }
        }
    }

    renderCommand.Flush();

    Profiler::StopFrame();

    return vCmdBuffer;
}
VulkanCommandBuffer VulkanGraphicsEngine::PointShadowPass(uint32_t a_camIndex, uint32_t a_bufferIndex, uint32_t a_frameIndex)
{
    RENDERSCRATCHFRAME;

    IcarianCore::StackAllocator* scratchAllocator = RenderScratchAlloc::GetAllocator();

    VulkanCommandBuffer vCmdBuffer = VulkanCommandBuffer(nullptr, VulkanCommandBufferType_Graphics, VulkanCommandBufferStage_ShadowPass);

    // ASAN hamstrings rendering performance have seen 2-4x performance improvements on linux with it disabled so make sure using release and not releasewithdebug for final build...
    // Probably want to do more effective culling and caching for point light shadow rendering as well

    // May switch this to omnidirectional image on 2d texture down the line not sure need more research
    // Also may want to do multi pass culling for point lights
    // This assumes that down the line I dont just switch everything to a compute shader with compute driven rendering
    // This is a bit janky and slow but it works for now
    Profiler::Start("Point Shadow Pass");
    IDEFER(Profiler::Stop());

    Profiler::StartFrame("Update");

    if (m_pointLights.Empty())
    {
        Profiler::StopFrame();

        return vCmdBuffer;
    }

    const CameraBuffer& camBuffer = m_cameraBuffers[a_camIndex];

    const vk::CommandBuffer commandBuffer = StartCommandBuffer(a_bufferIndex, a_frameIndex);
    IDEFER(commandBuffer.end());
    vCmdBuffer.SetCommandBuffer(commandBuffer);

    VulkanProfiler::StartTimingPoint(vCmdBuffer, "Point Shadow Pass");

    VULKAN_MARKER_COL(m_vulkanEngine, commandBuffer, "Point Shadow Pass", 255, 0, 0);

    VulkanRenderCommand& renderCommand = m_renderCommands.Push(VulkanRenderCommand
    (
        m_vulkanEngine,
        this,
        m_swapchain,
        commandBuffer,
        -1,
        a_bufferIndex
    ));
    m_lightData.Push(VulkanLightData());

    e_LightType lightType = LightType_Point;

    void* shadowSetupArgs[] =
    {
        &lightType,
        &a_camIndex
    };

    {
        PROFILESTACK("Shadow Setup");

        m_shadowSetupFunc->Exec(shadowSetupArgs);

        VulkanProfiler::PushTimingPoint(vCmdBuffer, "Shadow Setup");
    }

    const uint32_t swapWidth = m_swapchain->GetWidth();
    const uint32_t swapHeight = m_swapchain->GetHeight();

    const Frustum cameraFrustum = camBuffer.ToFrustum(glm::vec2(swapWidth, swapHeight));

    const IcarianCore::Array<PointLightBuffer> lights = m_pointLights.ToActiveArray(scratchAllocator);

    for (const PointLightBuffer& buffer : lights)
    {
        if (buffer.TransformAddr == uint32_t(-1) || (buffer.RenderLayer & camBuffer.RenderLayer) == 0 || buffer.Radius <= 0.0f || buffer.Intensity <= 0.0f)
        {
            continue;
        }

        IVERIFY(buffer.Data != nullptr);
        const VulkanLightBuffer* lightBuffer = (VulkanLightBuffer*)buffer.Data;
        if (lightBuffer->LightRenderTextureCount < 1)
        {
            continue;
        }

        const glm::mat4 transform = ObjectManager::GetGlobalMatrix(buffer.TransformAddr);
        const glm::vec3 position = transform[3].xyz();

        if (!cameraFrustum.CompareSphere(position, buffer.Radius))
        {
            continue;
        }

        const glm::mat4 proj = glm::perspective(glm::half_pi<float>(), 1.0f, 0.1f, buffer.Radius);

        const uint32_t renderTextureIndex = lightBuffer->LightRenderTextures[0];

        const VulkanDepthCubeRenderTexture* depthCubeRenderTexture = GetDepthCubeRenderTexture(renderTextureIndex);
        const glm::vec2 screenSize = glm::vec2(depthCubeRenderTexture->GetWidth(), depthCubeRenderTexture->GetHeight());

        const vk::RenderPass renderPass = depthCubeRenderTexture->GetRenderPass();

        constexpr vk::ClearValue ClearDepth = vk::ClearValue(vk::ClearDepthStencilValue(1.0f, 0));

        const vk::Rect2D scissor = vk::Rect2D({ 0, 0 }, { (uint32_t)screenSize.x, (uint32_t)screenSize.y });
        const vk::Viewport viewport = vk::Viewport(0.0f, 0.0f, screenSize.x, screenSize.y, 0.0f, 1.0f);

        const float halfRadius = buffer.Radius * 0.5f;

        for (int j = 0; j < 6; ++j)
        {
            // I believe Vulkan is +X, -X, +Y, -Y, +Z, -Z
            // Cannot be fucked to do this properly weird maths and loops it is
            glm::vec3 dir = glm::vec3(0.0f);
            // Huh turns out compilers are still dumb and still want to do bit magic instead of modulo
            dir[j / 2] = (1.0f - (j & 0b1)) * 2.0f - 1.0f;

            // Ekk out some extra performance for point lights partially on screen
            // Not much for small point lights
            // Still want to look into multipass culling but lazy
            // Bout ~20ms faster and ~2/3rds the vulkan calls in the sponza scene with renderdoc overhead
            // Only bout ~3-4ms uplift in editor
            if (!cameraFrustum.CompareSphere(position + dir * halfRadius, halfRadius))
            {
                continue;
            }

            glm::vec3 up = glm::vec3(0.0f, -1.0f, 0.0f);
            if (glm::abs(glm::dot(dir, up)) > 0.95f)
            {
                up = glm::vec3(0.0f, 0.0f, 1.0f);
            }

            const glm::mat4 view = glm::lookAt(position, position + dir, up);
            const glm::mat4 lvp = proj * view;

            const vk::RenderPassBeginInfo renderPassInfo = vk::RenderPassBeginInfo
            (
                renderPass,
                depthCubeRenderTexture->GetFrameBuffer(j),
                scissor,
                1,
                &ClearDepth
            );

            commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
            IDEFER(commandBuffer.endRenderPass());

            commandBuffer.setScissor(0, 1, &scissor);
            commandBuffer.setViewport(0, 1, &viewport);

            DrawShadow
            (
                lvp,
                buffer.Radius,
                buffer.ShadowBias,
                buffer.RenderLayer,
                renderTextureIndex,
                true,
                commandBuffer,
                a_frameIndex
            );
        }
    }

    VulkanProfiler::PushTimingPoint(vCmdBuffer, "Draw");

    renderCommand.Flush();

    Profiler::StopFrame();

    return vCmdBuffer;
}
VulkanCommandBuffer VulkanGraphicsEngine::SpotShadowPass(uint32_t a_camIndex, uint32_t a_bufferIndex, uint32_t a_frameIndex)
{
    RENDERSCRATCHFRAME;

    IcarianCore::StackAllocator* scratchAllocator = RenderScratchAlloc::GetAllocator();

    VulkanCommandBuffer vCmdBuffer = VulkanCommandBuffer(nullptr, VulkanCommandBufferType_Graphics, VulkanCommandBufferStage_ShadowPass);

    Profiler::Start("Spot Shadow Pass");
    IDEFER(Profiler::Stop());

    Profiler::StartFrame("Update");

    if (m_spotLights.Empty())
    {
        Profiler::StopFrame();

        return vCmdBuffer;
    }

    const CameraBuffer& camBuffer = m_cameraBuffers[a_camIndex];

    const vk::CommandBuffer commandBuffer = StartCommandBuffer(a_bufferIndex, a_frameIndex);
    IDEFER(commandBuffer.end());
    vCmdBuffer.SetCommandBuffer(commandBuffer);

    VulkanProfiler::StartTimingPoint(vCmdBuffer, "Spot Shadow Pass");

    VULKAN_MARKER_COL(m_vulkanEngine, commandBuffer, "Spot Shadow Pass", 255, 0, 0);

    VulkanRenderCommand& renderCommand = m_renderCommands.Push(VulkanRenderCommand(m_vulkanEngine, this, m_swapchain, commandBuffer, -1, a_bufferIndex));
    VulkanLightData& lightData = m_lightData.Push(VulkanLightData());

    e_LightType lightType = LightType_Spot;

    void* shadowSetupArgs[] =
    {
        &lightType,
        &a_camIndex
    };

    {
        PROFILESTACK("Shadow Setup");

        m_shadowSetupFunc->Exec(shadowSetupArgs);

        VulkanProfiler::PushTimingPoint(vCmdBuffer, "Shadow Setup");
    }

    const uint32_t swapWidth = m_swapchain->GetWidth();
    const uint32_t swapHeight = m_swapchain->GetHeight();
    const Frustum cameraFrustum = camBuffer.ToFrustum(glm::vec2(swapWidth, swapHeight));

    const uint32_t size = m_spotLights.Size();
    const IcarianCore::Array<SpotLightBuffer> lights = m_spotLights.ToArray(scratchAllocator);
    const IcarianCore::Array<uint8_t> state = m_spotLights.ToPackedStateArray(scratchAllocator);

    for (uint32_t i = 0; i < size; ++i)
    {
        const uint32_t index = i / 8;
        const uint32_t offset = i % 8;

        if (!IISBITSET(state[index], offset))
        {
            continue;
        }

        const SpotLightBuffer& buffer = lights[i];
        IVERIFY(buffer.Data != nullptr);

        const bool isValid = buffer.TransformAddr != uint32_t(-1) && buffer.Intensity > 0.0f && buffer.Radius > 0.0f;
        const bool sharesLayer = (buffer.RenderLayer & camBuffer.RenderLayer) != 0;
        if (!isValid || !sharesLayer)
        {
            continue;
        }

        const VulkanLightBuffer* lightBuffer = (VulkanLightBuffer*)buffer.Data;
        if (lightBuffer->LightRenderTextureCount < 1)
        {
            continue;
        }

        const glm::mat4 transform = ObjectManager::GetGlobalMatrix(buffer.TransformAddr);
        const glm::vec3 position = transform[3].xyz();

        if (!cameraFrustum.CompareSphere(position, buffer.Radius))
        {
            continue;
        }

        uint32_t renderTextureIndex = 0;
        void* shadowArgs[] =
        {
            &lightType,
            &i,
            &a_camIndex,
            &renderTextureIndex
        };

        {
            PROFILESTACK("Pre Shadow");

            m_preShadowFunc->Exec(shadowArgs);

            VulkanProfiler::PushTimingPoint(vCmdBuffer, "Pre Shadow");
        }

        const uint32_t lightRenderTexture = lightBuffer->LightRenderTextures[0];
        const VulkanDepthRenderTexture* depthRenderTexture = GetDepthRenderTexture(lightRenderTexture);

        const glm::vec2 screenSize = glm::vec2(depthRenderTexture->GetWidth(), depthRenderTexture->GetHeight());

        constexpr vk::ClearValue ClearDepth = vk::ClearValue(vk::ClearDepthStencilValue(1.0f, 0));

        const vk::Rect2D scissor = vk::Rect2D({ 0, 0 }, { (uint32_t)screenSize.x, (uint32_t)screenSize.y });
        const vk::Viewport viewport = vk::Viewport(0.0f, 0.0f, screenSize.x, screenSize.y, 0.0f, 1.0f);

        const vk::RenderPassBeginInfo renderPassInfo = vk::RenderPassBeginInfo
        (
            depthRenderTexture->GetRenderPass(),
            depthRenderTexture->GetFrameBuffer(),
            scissor,
            1,
            &ClearDepth
        );

        const uint32_t splitCount = lightData.GetSplitCount();
        const LightShadowSplit* splits = lightData.GetSplits();
        if (splitCount <= 0 || splits == nullptr)
        {
            continue;
        }

        commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
        IDEFER(commandBuffer.endRenderPass());

        commandBuffer.setScissor(0, 1, &scissor);
        commandBuffer.setViewport(0, 1, &viewport);

        DrawShadow(splits[0].LVP, buffer.Radius, buffer.ShadowBias, buffer.RenderLayer, lightRenderTexture, false, commandBuffer, a_frameIndex);

        VulkanProfiler::PushTimingPoint(vCmdBuffer, "Draw");

        {
            PROFILESTACK("Post Shadow");

            m_postShadowFunc->Exec(shadowArgs);

            VulkanProfiler::PushTimingPoint(vCmdBuffer, "Post Shadow");
        }
    }

    renderCommand.Flush();

    Profiler::StopFrame();

    return vCmdBuffer;
}

VulkanCommandBuffer VulkanGraphicsEngine::DrawPass(uint32_t a_camIndex, uint32_t a_bufferIndex, uint32_t a_frameIndex)
{
    Profiler::Start("Draw Pass");
    IDEFER(Profiler::Stop());

    Profiler::StartFrame("Update");

    const CameraBuffer camBuffer = m_cameraBuffers[a_camIndex];

    const vk::CommandBuffer commandBuffer = StartCommandBuffer(a_bufferIndex, a_frameIndex);
    IDEFER(commandBuffer.end());
    const VulkanCommandBuffer vCmdBuffer = VulkanCommandBuffer(commandBuffer, VulkanCommandBufferType_Graphics, VulkanCommandBufferStage_DeferredPass);

    VulkanProfiler::StartTimingPoint(vCmdBuffer, "Draw Pass");

    VULKAN_MARKER_COL(m_vulkanEngine, commandBuffer, "Draw Pass", 0, 255, 0);

    VulkanRenderCommand& renderCommand = m_renderCommands.Push(VulkanRenderCommand(m_vulkanEngine, this, m_swapchain, commandBuffer, a_camIndex, a_bufferIndex));

    void* camArgs[] =
    {
        &a_camIndex
    };

    {
        PROFILESTACK("Pre Render");

        m_preRenderFunc->Exec(camArgs);

        VulkanProfiler::PushTimingPoint(vCmdBuffer, "Pre Render");
    }

    const VulkanRenderTexture* renderTexture = renderCommand.GetRenderTexture();

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

    const Frustum frustum = camBuffer.ToFrustum(glm::vec2((float)screenWidth, (float)screenHeight));

    Draw(false, camBuffer, frustum, &renderCommand, a_frameIndex);

    VulkanProfiler::PushTimingPoint(vCmdBuffer, "Draw");

    {
        PROFILESTACK("Post Render");

        m_postRenderFunc->Exec(camArgs);

        VulkanProfiler::PushTimingPoint(vCmdBuffer, "Post Render");
    }

    renderCommand.Flush();

    Profiler::StopFrame();

    return vCmdBuffer;
}
VulkanCommandBuffer VulkanGraphicsEngine::LightPass(uint32_t a_camIndex, uint32_t a_bufferIndex, uint32_t a_frameIndex)
{
    Profiler::Start("Light Pass");
    IDEFER(Profiler::Stop());

    Profiler::StartFrame("Update");

    RENDERSCRATCHFRAME;

    IcarianCore::StackAllocator* scratchAllocator = RenderScratchAlloc::GetAllocator();

    VulkanPushPool* pushPool = m_vulkanEngine->GetPushPool();

    const CameraBuffer& camBuffer = m_cameraBuffers[a_camIndex];

    const vk::CommandBuffer commandBuffer = StartCommandBuffer(a_bufferIndex, a_frameIndex);
    IDEFER(commandBuffer.end());
    const VulkanCommandBuffer vCmdBuffer = VulkanCommandBuffer(commandBuffer, VulkanCommandBufferType_Graphics, VulkanCommandBufferStage_LightingPass);

    VulkanProfiler::StartTimingPoint(vCmdBuffer, "Light Pass");

    VULKAN_MARKER_COL(m_vulkanEngine, commandBuffer, "Light Pass", 0, 0, 255);

    VulkanRenderCommand& renderCommand = m_renderCommands.Push(VulkanRenderCommand
    (
        m_vulkanEngine,
        this,
        m_swapchain,
        commandBuffer,
        a_camIndex,
        a_bufferIndex
    ));
    VulkanLightData& lightData = m_lightData.Push(VulkanLightData());

    void* lightSetupArgs[] =
    {
        &a_camIndex
    };

    {
        PROFILESTACK("Light Setup");

        m_lightSetupFunc->Exec(lightSetupArgs);

        VulkanProfiler::PushTimingPoint(vCmdBuffer, "Light Setup");
    }

    const VulkanRenderTexture* renderTexture = renderCommand.GetRenderTexture();
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

    const Frustum frustum = camBuffer.ToFrustum(glm::vec2(screenWidth, screenHeight));

    for (uint32_t i = 0; i < LightType_ShadowEnd; ++i)
    {
        switch ((e_LightType)i)
        {
        case LightType_Directional:
        {
            PROFILESTACK("Shadow Directional Light");

            RENDERSCRATCHFRAME;

            VULKAN_MARKER(m_vulkanEngine, commandBuffer, "Shadow Directional Light");

            const uint32_t size = m_directionalLights.Size();
            const IcarianCore::Array<DirectionalLightBuffer> lights = m_directionalLights.ToArray(scratchAllocator);
            const IcarianCore::Array<uint8_t> state = m_directionalLights.ToPackedStateArray(scratchAllocator);

            for (uint32_t j = 0; j < size; ++j)
            {
                const uint32_t index = i / 8;
                const uint32_t offset = i % 8;

                if (!IISBITSET(state[index], offset))
                {
                    continue;
                }

                const DirectionalLightBuffer& buffer = lights[j];

                const bool isValid = buffer.TransformAddr != uint32_t(-1) && buffer.Intensity > 0.0f;
                const bool shareLayer = (camBuffer.RenderLayer & buffer.RenderLayer) != 0;
                if (!isValid || !shareLayer)
                {
                    continue;
                }

                IVERIFY(buffer.Data != nullptr);
                const VulkanLightBuffer* lightBuffer = (VulkanLightBuffer*)buffer.Data;

                const bool isShadowLight = lightBuffer->LightRenderTextureCount > 0;
                if (!isShadowLight)
                {
                    continue;
                }

                void* lightArgs[] =
                {
                    &i,
                    &j,
                    &a_camIndex
                };

                m_preShadowLightFunc->Exec(lightArgs);
                IDEFER(m_postShadowLightFunc->Exec(lightArgs));

                const uint32_t splitCount = lightData.GetSplitCount();
                const LightShadowSplit* splits = lightData.GetSplits();
                if (splitCount <= 0 || splits == nullptr)
                {
                    continue;
                }

                const VulkanPipeline* pipeline = renderCommand.GetPipeline();
                if (pipeline == nullptr)
                {
                    continue;
                }

                const VulkanShaderData* data = pipeline->GetShaderData();
                IVERIFY(data != nullptr);

                ShaderBufferInput dirLightInput;
                if (data->GetShaderBufferInput(ShaderBufferType_DirectionalLightBuffer, &dirLightInput))
                {
                    RENDERSCRATCHFRAME;

                    renderCommand.PushLight(dirLightInput.RealSlot, LightType_Directional, j, scratchAllocator);
                }

                ShaderBufferInput shadowLightInput;
                if (data->GetShaderBufferInput(ShaderBufferType_SSShadowLightBuffer, &shadowLightInput))
                {
                    RENDERSCRATCHFRAME;

                    renderCommand.PushLightSplits(shadowLightInput.RealSlot, splits, splitCount, scratchAllocator);
                }

                ShaderBufferInput shadowTextureInput;
                if (data->GetShaderBufferInput(ShaderBufferType_AShadowTexture2D, &shadowTextureInput))
                {
                    RENDERSCRATCHFRAME;

                    renderCommand.PushShadowTextureArray(shadowTextureInput.RealSlot, j, scratchAllocator);
                }

                commandBuffer.draw(4, 1, 0, 0);
            }

            VulkanProfiler::PushTimingPoint(vCmdBuffer, "Shadow Directional Light");

            break;
        }
        case LightType_Point:
        {
            PROFILESTACK("Shadow Point Light");

            RENDERSCRATCHFRAME;

            VULKAN_MARKER(m_vulkanEngine, commandBuffer, "Shadow Point Light");

            const uint32_t size = m_pointLights.Size();
            const IcarianCore::Array<PointLightBuffer> lights = m_pointLights.ToArray(scratchAllocator);
            const IcarianCore::Array<uint8_t> state = m_pointLights.ToPackedStateArray(scratchAllocator);

            for (uint32_t j = 0; j < size; ++j)
            {
                const uint32_t index = i / 8;
                const uint32_t offset = i % 8;

                if (!IISBITSET(state[index], offset))
                {
                    continue;
                }

                const PointLightBuffer& buffer = lights[j];

                const bool isValid = buffer.TransformAddr != uint32_t(-1) && buffer.Radius > 0.0f && buffer.Intensity > 0.0f;
                const bool shareLayer = (buffer.RenderLayer & camBuffer.RenderLayer) != 0;
                if (!isValid || !shareLayer)
                {
                    continue;
                }

                const VulkanLightBuffer* lightBuffer = (VulkanLightBuffer*)buffer.Data;
                IVERIFY(lightBuffer != nullptr);

                const bool isShadowLight = lightBuffer->LightRenderTextureCount > 0;
                if (!isShadowLight)
                {
                    continue;
                }

                const glm::mat4 transform = ObjectManager::GetGlobalMatrix(buffer.TransformAddr);
                const glm::vec3 position = transform[3].xyz();

                if (!frustum.CompareSphere(position, buffer.Radius))
                {
                    continue;
                }

                void* lightArgs[] =
                {
                    &i,
                    &j,
                    &a_camIndex
                };

                m_preShadowLightFunc->Exec(lightArgs);
                IDEFER(m_postShadowLightFunc->Exec(lightArgs));

                const VulkanPipeline* pipeline = renderCommand.GetPipeline();
                if (pipeline == nullptr)
                {
                    continue;
                }

                const VulkanShaderData* data = pipeline->GetShaderData();
                IVERIFY(data != nullptr);

                ShaderBufferInput pointLightInput;
                if (data->GetShaderBufferInput(ShaderBufferType_PointLightBuffer, &pointLightInput))
                {
                    // Yes I do have a render command for it
                    // Already have all the data so just do it
                    // Otherwise will waste time getting data again
                    // It is a point light so important to be fast
                    VulkanUniformBuffer* uniformBuffer = pushPool->AllocatePointLightUniformBuffer();

                    const IcarianCore::ShaderPointLightBuffer shaderBuffer =
                    {
                        .LightPos = glm::vec4(position, buffer.Intensity),
                        .LightColor = buffer.Color,
                        .Radius = buffer.Radius,
                    };

                    uniformBuffer->SetData(a_frameIndex, &shaderBuffer);

                    data->PushUniformBuffer(commandBuffer, pointLightInput.RealSlot, uniformBuffer, a_frameIndex);
                }

                ShaderBufferInput shadowTextureInput;
                if (data->GetShaderBufferInput(ShaderBufferType_ShadowTextureCube, &shadowTextureInput))
                {
                    VulkanTextureSampler* sampler = scratchAllocator->TAllocate<VulkanTextureSampler>();

                    const TextureSamplerBuffer buffer =
                    {
                        .Addr = lightBuffer->LightRenderTextures[0],
                        .Slot = 0,
                        .TextureMode = TextureMode_DepthCubeRenderTexture,
                        .FilterMode = TextureFilter_Linear,
                        .AddressMode = TextureAddress_ClampToEdge,
                        .Data = sampler
                    };

                    const VulkanTextureSamplerBuilder builder =
                    {
                        .Engine = m_vulkanEngine,
                        .Sampler = buffer
                    };
                    VulkanTextureSampler::GenerateFromBuffer(sampler, builder);

                    IDEFER(sampler->~VulkanTextureSampler());

                    data->PushTexture(commandBuffer, shadowTextureInput.RealSlot, buffer, a_frameIndex);
                }

                commandBuffer.draw(4, 1, 0, 0);
            }

            VulkanProfiler::PushTimingPoint(vCmdBuffer, "Shadow Point Light");

            break;
        }
        case LightType_Spot:
        {
            PROFILESTACK("Shadow Spot Light");

            RENDERSCRATCHFRAME;

            VULKAN_MARKER(m_vulkanEngine, commandBuffer, "Shadow Spot Light");

            const uint32_t size = m_spotLights.Size();
            const IcarianCore::Array<SpotLightBuffer> lights = m_spotLights.ToArray(scratchAllocator);
            const IcarianCore::Array<uint8_t> state = m_spotLights.ToPackedStateArray(scratchAllocator);

            for (uint32_t j = 0; j < size; ++j)
            {
                const uint32_t index = i / 8;
                const uint32_t offset = i % 8;

                if (!IISBITSET(state[index], offset))
                {
                    continue;
                }

                const SpotLightBuffer& buffer = lights[j];

                const bool isValid = buffer.TransformAddr != uint32_t(-1) && buffer.Radius > 0 && buffer.Intensity > 0;
                const bool shareLayer = (camBuffer.RenderLayer & buffer.RenderLayer) != 0;
                if (!isValid || !shareLayer)
                {
                    continue;
                }

                const VulkanLightBuffer* lightBuffer = (VulkanLightBuffer*)buffer.Data;
                IVERIFY(lightBuffer != nullptr);

                const bool isShadowLight = lightBuffer->LightRenderTextureCount > 0;
                if (!isShadowLight)
                {
                    continue;
                }

                const glm::mat4 transform = ObjectManager::GetGlobalMatrix(buffer.TransformAddr);
                const glm::vec3 position = transform[3].xyz();

                if (!frustum.CompareSphere(position, buffer.Radius))
                {
                    continue;
                }

                void* lightArgs[] =
                {
                    &i,
                    &j,
                    &a_camIndex
                };

                m_preShadowLightFunc->Exec(lightArgs);
                IDEFER(m_postShadowLightFunc->Exec(lightArgs));

                const uint32_t splitCount = lightData.GetSplitCount();
                const LightShadowSplit* splits = lightData.GetSplits();
                if (splitCount <= 0 || splits == nullptr)
                {
                    continue;
                }

                const VulkanPipeline* pipeline = renderCommand.GetPipeline();
                if (pipeline == nullptr)
                {
                    continue;
                }

                const VulkanShaderData* data = pipeline->GetShaderData();
                IVERIFY(data != nullptr);

                ShaderBufferInput spotLightInput;
                if (data->GetShaderBufferInput(ShaderBufferType_SpotLightBuffer, &spotLightInput))
                {
                    VulkanUniformBuffer* uniformBuffer = pushPool->AllocateSpotLightUniformBuffer();

                    const glm::vec3 forward = glm::normalize(transform[2].xyz());

                    const IcarianCore::ShaderSpotLightBuffer shaderBuffer =
                    {
                        .LightPos = position,
                        .LightDir = glm::vec4(forward, buffer.Intensity),
                        .LightColor = buffer.Color,
                        .CutoffAngle = glm::vec3(buffer.CutoffAngle, buffer.Radius)
                    };

                    uniformBuffer->SetData(a_frameIndex, &shaderBuffer);

                    data->PushUniformBuffer(commandBuffer, spotLightInput.RealSlot, uniformBuffer, a_frameIndex);
                }

                ShaderBufferInput shadowTextureInput;
                if (data->GetShaderBufferInput(ShaderBufferType_ShadowTexture2D, &shadowTextureInput))
                {
                    VulkanTextureSampler* sampler = scratchAllocator->TAllocate<VulkanTextureSampler>();

                    const TextureSamplerBuffer buffer =
                    {
                        .Addr = lightBuffer->LightRenderTextures[0],
                        .Slot = 0,
                        .TextureMode = TextureMode_DepthRenderTexture,
                        .FilterMode = TextureFilter_Linear,
                        .AddressMode = TextureAddress_ClampToEdge,
                        .Data = sampler
                    };

                    const VulkanTextureSamplerBuilder builder =
                    {
                        .Engine = m_vulkanEngine,
                        .Sampler = buffer
                    };

                    VulkanTextureSampler::GenerateFromBuffer(sampler, builder);
                    IDEFER(sampler->~VulkanTextureSampler());

                    data->PushTexture(commandBuffer, shadowTextureInput.RealSlot, buffer, a_frameIndex);
                }

                ShaderBufferInput shadowBufferInput;
                if (data->GetShaderBufferInput(ShaderBufferType_ShadowLightBuffer, &shadowBufferInput))
                {
                    VulkanUniformBuffer* uniformBuffer = pushPool->AllocateShadowUniformBuffer();

                    const IcarianCore::ShaderShadowLightBuffer shaderBuffer =
                    {
                        .LVP = splits[0].LVP,
                        .Split = buffer.Radius
                    };

                    uniformBuffer->SetData(a_frameIndex, &shaderBuffer);

                    data->PushUniformBuffer(commandBuffer, shadowBufferInput.RealSlot, uniformBuffer, a_frameIndex);
                }

                commandBuffer.draw(4, 1, 0, 0);
            }

            VulkanProfiler::PushTimingPoint(vCmdBuffer, "Shadow Spot Light");

            break;
        }
        default:
        {
            IERROR("Invalid light type");
        }
        }
    }

    for (uint32_t i = 0; i < LightType_End; ++i)
    {
        RENDERSCRATCHFRAME;

        void* lightArgs[] =
        {
            &i,
            &a_camIndex
        };

        m_preLightFunc->Exec(lightArgs);

        const VulkanPipeline* pipeline = renderCommand.GetPipeline();
        if (pipeline == nullptr)
        {
            continue;
        }

        const VulkanShaderData* data = pipeline->GetShaderData();
        IVERIFY(data != nullptr);

        switch ((e_LightType)i)
        {
        case LightType_Ambient:
        {
            PROFILESTACK("Ambient Light");

            RENDERSCRATCHFRAME;

            VULKAN_MARKER(m_vulkanEngine, commandBuffer, "Ambient Light");

            const IcarianCore::Array<AmbientLightBuffer> lights = m_ambientLights.ToActiveArray(scratchAllocator);
            const uint32_t size = lights.Size();

            ShaderBufferInput ambientLightInput;
            if (data->GetShaderBufferInput(ShaderBufferType_AmbientLightBuffer, &ambientLightInput))
            {
                for (const AmbientLightBuffer& ambientLight : lights)
                {
                    const bool sharesLayer = (camBuffer.RenderLayer & ambientLight.RenderLayer) != 0;
                    if (!sharesLayer)
                    {
                        continue;
                    }

                    const bool isValid = ambientLight.Intensity > 0.0f;
                    if (!isValid)
                    {
                        continue;
                    }

                    VulkanUniformBuffer* uniformBuffer = pushPool->AllocateAmbientLightUniformBuffer();

                    const IcarianCore::ShaderAmbientLightBuffer buffer =
                    {
                        .LightColor = glm::vec4(ambientLight.Color.xyz(), ambientLight.Intensity)
                    };

                    uniformBuffer->SetData(a_frameIndex, &buffer);

                    data->PushUniformBuffer(commandBuffer, ambientLightInput.RealSlot, uniformBuffer, a_frameIndex);

                    RENDERSCRATCHFRAME;

                    renderCommand.DrawMaterial(scratchAllocator);
                }
            }
            else if (data->GetShaderBufferInput(ShaderBufferType_SSAmbientLightBuffer, &ambientLightInput))
            {
                RENDERSCRATCHFRAME;

                IcarianCore::ShaderAmbientLightBuffer* buffers = scratchAllocator->TAllocate<IcarianCore::ShaderAmbientLightBuffer>(size);
                uint32_t count = 0;

                for (const AmbientLightBuffer& ambientLight : lights)
                {
                    const bool sharesLayer = (camBuffer.RenderLayer & ambientLight.RenderLayer) != 0;
                    if (!sharesLayer)
                    {
                        continue;
                    }

                    const bool isValid = ambientLight.Intensity > 0.0f;
                    if (!isValid)
                    {
                        continue;
                    }

                    buffers[count++].LightColor = glm::vec4(ambientLight.Color.xyz(), ambientLight.Intensity);
                }

                if (count > 0)
                {
                    const VulkanShaderStorageObject storage = VulkanShaderStorageObject
                    (
                        m_vulkanEngine,
                        sizeof(IcarianCore::ShaderAmbientLightBuffer) * count,
                        count,
                        buffers
                    );

                    data->PushShaderStorageObject(commandBuffer, ambientLightInput.RealSlot, &storage, a_frameIndex);

                    renderCommand.DrawMaterial(scratchAllocator);
                }
            }

            VulkanProfiler::PushTimingPoint(vCmdBuffer, "Ambient Light");

            break;
        }
        case LightType_Directional:
        {
            PROFILESTACK("Directional Light");

            RENDERSCRATCHFRAME;

            VULKAN_MARKER(m_vulkanEngine, commandBuffer, "Directional Light");

            const IcarianCore::Array<DirectionalLightBuffer> lights = m_directionalLights.ToActiveArray(scratchAllocator);

            ShaderBufferInput dirLightInput;
            if (data->GetShaderBufferInput(ShaderBufferType_DirectionalLightBuffer, &dirLightInput))
            {
                for (const DirectionalLightBuffer& dirLight : lights)
                {
                    const bool sharesLayer = (camBuffer.RenderLayer & dirLight.RenderLayer) != 0;
                    if (!sharesLayer)
                    {
                        continue;
                    }

                    const bool isValid = dirLight.TransformAddr != uint32_t(-1) && dirLight.Intensity > 0.0f;
                    if (!isValid)
                    {
                        continue;
                    }

                    const VulkanLightBuffer* lightData = (VulkanLightBuffer*)dirLight.Data;
                    IVERIFY(lightData != nullptr);

                    const bool isShadowLight = lightData->LightRenderTextureCount > 0;
                    if (isShadowLight)
                    {
                        continue;
                    }

                    VulkanUniformBuffer* uniformBuffer = pushPool->AllocateDirectionalLightUniformBuffer();

                    const glm::mat4 tMat = ObjectManager::GetGlobalMatrix(dirLight.TransformAddr);
                    const glm::vec3 forward = glm::normalize(tMat[2].xyz());

                    const IcarianCore::ShaderDirectionalLightBuffer buffer =
                    {
                        .LightDir = glm::vec4(forward, dirLight.Intensity),
                        .LightColor = dirLight.Color
                    };

                    uniformBuffer->SetData(a_frameIndex, &buffer);

                    data->PushUniformBuffer(commandBuffer, dirLightInput.RealSlot, uniformBuffer, a_frameIndex);

                    RENDERSCRATCHFRAME;

                    renderCommand.DrawMaterial(scratchAllocator);
                }
            }
            else if (data->GetShaderBufferInput(ShaderBufferType_SSDirectionalLightBuffer, &dirLightInput))
            {
                RENDERSCRATCHFRAME;

                const uint32_t size = lights.Size();

                IcarianCore::ShaderDirectionalLightBuffer* buffers = scratchAllocator->TAllocate<IcarianCore::ShaderDirectionalLightBuffer>(size);
                uint32_t count = 0;

                for (const DirectionalLightBuffer& dirLight : lights)
                {
                    const bool sharesLayer = (camBuffer.RenderLayer & dirLight.RenderLayer) != 0;
                    if (!sharesLayer)
                    {
                        continue;
                    }

                    const bool isValid = dirLight.TransformAddr != uint32_t(-1) && dirLight.Intensity > 0.0f;
                    if (!isValid)
                    {
                        continue;
                    }

                    const VulkanLightBuffer* lightData = (VulkanLightBuffer*)dirLight.Data;
                    IVERIFY(lightData != nullptr);

                    const bool isShadowLight = lightData->LightRenderTextureCount > 0;
                    if (isShadowLight)
                    {
                        continue;
                    }

                    IDEFER(++count);

                    const glm::mat4 tMat = ObjectManager::GetGlobalMatrix(dirLight.TransformAddr);
                    const glm::vec3 forward = glm::normalize(tMat[2].xyz());

                    buffers[count].LightDir = glm::vec4(forward, dirLight.Intensity);
                    buffers[count].LightColor = dirLight.Color;
                }

                if (count > 0)
                {
                    const VulkanShaderStorageObject storage = VulkanShaderStorageObject
                    (
                        m_vulkanEngine,
                        sizeof(IcarianCore::ShaderDirectionalLightBuffer) * count,
                        count,
                        buffers
                    );

                    data->PushShaderStorageObject(commandBuffer, dirLightInput.RealSlot, &storage, a_frameIndex);

                    renderCommand.DrawMaterial(scratchAllocator);
                }
            }

            VulkanProfiler::PushTimingPoint(vCmdBuffer, "Directional Light");

            break;
        }
        case LightType_Point:
        {
            PROFILESTACK("Point Light");

            RENDERSCRATCHFRAME;

            VULKAN_MARKER(m_vulkanEngine, commandBuffer, "Point Light");

            const IcarianCore::Array<PointLightBuffer> lights = m_pointLights.ToActiveArray(scratchAllocator);

            ShaderBufferInput pointLightInput;
            if (data->GetShaderBufferInput(ShaderBufferType_PointLightBuffer, &pointLightInput))
            {
                for (const PointLightBuffer& pointLight : lights)
                {
                    const bool sharesLayer = (camBuffer.RenderLayer & pointLight.RenderLayer) != 0;
                    if (!sharesLayer)
                    {
                        continue;
                    }

                    const bool isValid = pointLight.TransformAddr != uint32_t(-1) && pointLight.Radius > 0.0f && pointLight.Intensity > 0.0f;
                    if (!isValid)
                    {
                        continue;
                    }

                    const VulkanLightBuffer* lightData = (VulkanLightBuffer*)pointLight.Data;
                    IVERIFY(lightData != nullptr);

                    const bool isShadowLight = lightData->LightRenderTextureCount > 0;
                    if (isShadowLight)
                    {
                        continue;
                    }

                    const glm::mat4 tMat = ObjectManager::GetGlobalMatrix(pointLight.TransformAddr);
                    const glm::vec3 position = tMat[3].xyz();
                    if (!frustum.CompareSphere(position, pointLight.Radius))
                    {
                        continue;
                    }

                    VulkanUniformBuffer* uniformBuffer = pushPool->AllocatePointLightUniformBuffer();

                    const IcarianCore::ShaderPointLightBuffer buffer =
                    {
                        .LightPos = glm::vec4(position, pointLight.Intensity),
                        .LightColor = pointLight.Color,
                        .Radius = pointLight.Radius,
                    };

                    uniformBuffer->SetData(a_frameIndex, &buffer);

                    data->PushUniformBuffer(commandBuffer, pointLightInput.RealSlot, uniformBuffer, a_frameIndex);

                    RENDERSCRATCHFRAME;

                    renderCommand.DrawMaterial(scratchAllocator);
                }
            }
            else if (data->GetShaderBufferInput(ShaderBufferType_SSPointLightBuffer, &pointLightInput))
            {
                RENDERSCRATCHFRAME;

                const uint32_t size = lights.Size();

                IcarianCore::ShaderPointLightBuffer* buffers = scratchAllocator->TAllocate<IcarianCore::ShaderPointLightBuffer>(size);
                uint32_t count = 0;

                for (const PointLightBuffer& pointLight : lights)
                {
                    const bool sharesLayer = (camBuffer.RenderLayer & pointLight.RenderLayer) != 0;
                    if (!sharesLayer)
                    {
                        continue;
                    }

                    const bool isValid = pointLight.TransformAddr != uint32_t(-1) && pointLight.Radius > 0.0f && pointLight.Intensity > 0.0f;
                    if (!isValid)
                    {
                        continue;
                    }

                    const VulkanLightBuffer* lightData = (VulkanLightBuffer*)pointLight.Data;
                    IVERIFY(lightData != nullptr);

                    const bool isShadowLight = lightData->LightRenderTextureCount > 0;
                    if (isShadowLight)
                    {
                        continue;
                    }

                    const glm::mat4 tMat = ObjectManager::GetGlobalMatrix(pointLight.TransformAddr);
                    const glm::vec3 position = tMat[3].xyz();
                    if (!frustum.CompareSphere(position, pointLight.Radius))
                    {
                        continue;
                    }

                    IDEFER(++count);

                    buffers[count].LightPos = glm::vec4(position, pointLight.Intensity);
                    buffers[count].LightColor = pointLight.Color;
                    buffers[count].Radius = pointLight.Radius;
                }

                if (count > 0)
                {
                    const VulkanShaderStorageObject storage = VulkanShaderStorageObject
                    (
                        m_vulkanEngine,
                        sizeof(IcarianCore::ShaderPointLightBuffer) * count,
                        count,
                        buffers
                    );

                    data->PushShaderStorageObject(commandBuffer, pointLightInput.RealSlot, &storage, a_frameIndex);

                    RENDERSCRATCHFRAME;

                    renderCommand.DrawMaterial(scratchAllocator);
                }
            }

            VulkanProfiler::PushTimingPoint(vCmdBuffer, "Point Light");

            break;
        }
        case LightType_Spot:
        {
            PROFILESTACK("Spot Light");

            RENDERSCRATCHFRAME;

            VULKAN_MARKER(m_vulkanEngine, commandBuffer, "Spot Light");

            const IcarianCore::Array<SpotLightBuffer> lights = m_spotLights.ToActiveArray(scratchAllocator);

            ShaderBufferInput spotLightInput;
            if (data->GetShaderBufferInput(ShaderBufferType_SpotLightBuffer, &spotLightInput))
            {
                for (const SpotLightBuffer& spotLight : lights)
                {
                    const bool sharesLayer = (camBuffer.RenderLayer & spotLight.RenderLayer) != 0;
                    if (!sharesLayer)
                    {
                        continue;
                    }

                    const bool isValid = spotLight.TransformAddr != uint32_t(-1) && spotLight.Radius > 0.0f && spotLight.Intensity > 0.0f;
                    if (!isValid)
                    {
                        continue;
                    }

                    const VulkanLightBuffer* lightData = (VulkanLightBuffer*)spotLight.Data;
                    IVERIFY(lightData != nullptr);

                    const bool isShadowLight = lightData->LightRenderTextureCount > 0;
                    if (isShadowLight)
                    {
                        continue;
                    }

                    const glm::mat4 tMat = ObjectManager::GetGlobalMatrix(spotLight.TransformAddr);
                    const glm::vec3 position = tMat[3].xyz();
                    if (!frustum.CompareSphere(position, spotLight.Radius))
                    {
                        continue;
                    }

                    const glm::vec3 forward = glm::normalize(tMat[2].xyz());

                    VulkanUniformBuffer* uniformBuffer = pushPool->AllocateSpotLightUniformBuffer();

                    const IcarianCore::ShaderSpotLightBuffer buffer =
                    {
                        .LightPos = position,
                        .LightDir = glm::vec4(forward, spotLight.Intensity),
                        .LightColor = spotLight.Color,
                        .CutoffAngle = glm::vec3(spotLight.CutoffAngle, spotLight.Radius)
                    };

                    uniformBuffer->SetData(a_frameIndex, &buffer);

                    data->PushUniformBuffer
                    (
                        commandBuffer,
                        spotLightInput.RealSlot,
                        uniformBuffer,
                        a_frameIndex
                    );

                    RENDERSCRATCHFRAME;

                    renderCommand.DrawMaterial(scratchAllocator);
                }
            }
            else if (data->GetShaderBufferInput(ShaderBufferType_SSSpotLightBuffer, &spotLightInput))
            {
                RENDERSCRATCHFRAME;

                const uint32_t size = lights.Size();

                IcarianCore::ShaderSpotLightBuffer* buffers = scratchAllocator->TAllocate<IcarianCore::ShaderSpotLightBuffer>(size);
                uint32_t count = 0;
                for (const SpotLightBuffer& spotLight : lights)
                {
                    const bool sharesLayer = (camBuffer.RenderLayer & spotLight.RenderLayer) != 0;
                    if (!sharesLayer)
                    {
                        continue;
                    }

                    const bool isValid = spotLight.TransformAddr != uint32_t(-1) && spotLight.Radius > 0.0f && spotLight.Intensity > 0.0f;
                    if (!isValid)
                    {
                        continue;
                    }

                    const VulkanLightBuffer* lightData = (VulkanLightBuffer*)spotLight.Data;
                    IVERIFY(lightData != nullptr);

                    const bool isShadowLight = lightData->LightRenderTextureCount > 0;
                    if (isShadowLight)
                    {
                        continue;
                    }

                    const glm::mat4 tMat = ObjectManager::GetGlobalMatrix(spotLight.TransformAddr);
                    const glm::vec3 position = tMat[3].xyz();

                    if (!frustum.CompareSphere(position, spotLight.Radius))
                    {
                        continue;
                    }

                    IDEFER(++count);

                    const glm::vec3 forward = glm::normalize(tMat[2].xyz());

                    buffers[count].LightPos = position;
                    buffers[count].LightDir = glm::vec4(forward, spotLight.Intensity);
                    buffers[count].LightColor = spotLight.Color;
                    buffers[count].CutoffAngle = glm::vec3(spotLight.CutoffAngle, spotLight.Radius);
                }

                if (count > 0)
                {
                    const VulkanShaderStorageObject storage = VulkanShaderStorageObject
                    (
                        m_vulkanEngine,
                        sizeof(IcarianCore::ShaderSpotLightBuffer) * count,
                        count,
                        buffers
                    );

                    data->PushShaderStorageObject(commandBuffer, spotLightInput.RealSlot, &storage, a_frameIndex);

                    renderCommand.DrawMaterial(scratchAllocator);
                }
            }

            VulkanProfiler::PushTimingPoint(vCmdBuffer, "Spot Light");

            break;
        }
        default:
        {
            IERROR("Invalid light type");

            break;
        }
        }

        m_postLightFunc->Exec(lightArgs);
    }

    renderCommand.Flush();

    Profiler::StopFrame();

    return vCmdBuffer;
}
VulkanCommandBuffer VulkanGraphicsEngine::ForwardPass(uint32_t a_camIndex, uint32_t a_bufferIndex, uint32_t a_frameIndex)
{
    Profiler::Start("Forward Pass");
    IDEFER(Profiler::Stop());

    Profiler::StartFrame("Update");

    RENDERSCRATCHFRAME;

    IcarianCore::StackAllocator* scratchAllocator = RenderScratchAlloc::GetAllocator();

    const CameraBuffer camBuffer = m_cameraBuffers[a_camIndex];

    const vk::CommandBuffer commandBuffer = StartCommandBuffer(a_bufferIndex, a_frameIndex);
    IDEFER(commandBuffer.end());
    const VulkanCommandBuffer vCmdBuffer = VulkanCommandBuffer(commandBuffer, VulkanCommandBufferType_Graphics, VulkanCommandBufferStage_ForwardPass);

    VulkanProfiler::StartTimingPoint(vCmdBuffer, "Forward Pass");

    VULKAN_MARKER_COL(m_vulkanEngine, commandBuffer, "Forward Pass", 0, 255, 255);

    VulkanRenderCommand& renderCommand = m_renderCommands.Push(VulkanRenderCommand
    (
        m_vulkanEngine,
        this,
        m_swapchain,
        commandBuffer,
        a_camIndex,
        a_bufferIndex
    ));

    void* camArgs[] =
    {
        &a_camIndex
    };

    {
        PROFILESTACK("Pre Forward");

        IVERIFY(m_preForwardFunc != nullptr);
        m_preForwardFunc->Exec(camArgs);

        VulkanProfiler::PushTimingPoint(vCmdBuffer, "Pre Forward");
    }

    uint32_t screenWidth = m_swapchain->GetWidth();
    uint32_t screenHeight = m_swapchain->GetHeight();

    const VulkanRenderTexture* renderTexture = renderCommand.GetRenderTexture();
    if (renderTexture != nullptr)
    {
        screenWidth = renderTexture->GetWidth();
        screenHeight = renderTexture->GetHeight();
    }

    const Frustum frustum = camBuffer.ToFrustum(glm::vec2(screenWidth, screenHeight));

    Draw(true, camBuffer, frustum, &renderCommand, a_frameIndex);

    VulkanProfiler::PushTimingPoint(vCmdBuffer, "Draw");

    {
        PROFILESTACK("Particles");

        RENDERSCRATCHFRAME;

        const uint32_t renderTextureAddr = renderCommand.GetRenderTexutreAddr();
        const IcarianCore::Array<VulkanGraphicsParticle2D*> particleSystems = m_particleEmitters.ToActiveArray(scratchAllocator);

        for (VulkanGraphicsParticle2D* pSys : particleSystems)
        {
            RENDERSCRATCHFRAME;

            IVERIFY(pSys != nullptr);

            pSys->Update
            (
                a_frameIndex,
                a_bufferIndex,
                camBuffer.RenderLayer,
                commandBuffer,
                renderTextureAddr,
                scratchAllocator
            );
        }

        VulkanProfiler::PushTimingPoint(vCmdBuffer, "Particles");
    }

    {
        PROFILESTACK("Post Forward");

        IVERIFY(m_postForwardFunc != nullptr);
        m_postForwardFunc->Exec(camArgs);

        VulkanProfiler::PushTimingPoint(vCmdBuffer, "Post Forward");
    }

    renderCommand.Flush();

    Profiler::StopFrame();

    return vCmdBuffer;
}
VulkanCommandBuffer VulkanGraphicsEngine::PostPass(uint32_t a_camIndex, uint32_t a_bufferIndex, uint32_t a_frameIndex)
{
    Profiler::Start("Post Pass");
    IDEFER(Profiler::Stop());

    Profiler::StartFrame("Update");

    const vk::CommandBuffer commandBuffer = StartCommandBuffer(a_bufferIndex, a_frameIndex);
    IDEFER(commandBuffer.end());
    const VulkanCommandBuffer vCmdBuffer = VulkanCommandBuffer(commandBuffer, VulkanCommandBufferType_Graphics, VulkanCommandBufferStage_PostPass);

    VulkanProfiler::StartTimingPoint(vCmdBuffer, "Post Pass");

    VULKAN_MARKER_COL(m_vulkanEngine, commandBuffer, "Post Pass", 255, 255, 0);

    VulkanRenderCommand& renderCommand = m_renderCommands.Push(VulkanRenderCommand
    (
        m_vulkanEngine,
        this,
        m_swapchain,
        commandBuffer,
        a_camIndex,
        a_bufferIndex
    ));

    void* camArgs[] =
    {
        &a_camIndex
    };

    m_postProcessFunc->Exec(camArgs);

    VulkanProfiler::PushTimingPoint(vCmdBuffer, "Post Processing");

    renderCommand.Flush();

    Profiler::StopFrame();

    return vCmdBuffer;
}

void VulkanGraphicsEngine::DrawUIElement(vk::CommandBuffer a_commandBuffer, uint32_t a_addr, const CanvasBuffer& a_canvas, const glm::vec2& a_screenSize,uint32_t a_index)
{
    const bool valid = a_addr != uint32_t(-1);
    if (!valid)
    {
        return;
    }

    UIElement* element = UIControl::GetUIElement(a_addr);
    if (element == nullptr)
    {
        return;
    }

    const glm::vec2 pos = element->GetCanvasPosition(a_canvas, a_screenSize);
    const glm::vec2 scale = element->GetCanvasScale(a_canvas, a_screenSize);

    const glm::vec2 screenPos = pos * a_screenSize;
    const glm::vec2 screenSize = scale * a_screenSize;

    RenderEngine* renderEngine = m_vulkanEngine->GetRenderEngine();

    {
        RENDERSCRATCHFRAME;

        IcarianCore::Allocator* scratchAllocator = RenderScratchAlloc::GetAllocator();

        element->Update(renderEngine, scratchAllocator);
    }

    VulkanPipeline* pipeline = nullptr;
    VulkanShaderData* shaderData = nullptr;

    const e_UIElementType type = element->GetType();
    switch (type)
    {
    case UIElementType_Base:
    {
        break;
    }
    case UIElementType_Text:
    {
        const TextUIElement* text = (TextUIElement*)element;

        if (!text->IsValid())
        {
            break;
        }

        const vk::Rect2D scissor = vk::Rect2D
        (
            {
                (int32_t)screenPos.x,
                (int32_t)screenPos.y
            },
            {
                (uint32_t)screenSize.x,
                (uint32_t)screenSize.y
            }
        );
        a_commandBuffer.setScissor(0, 1, &scissor);
        const vk::Viewport viewport = vk::Viewport
        (
            screenPos.x,
            screenPos.y,
            screenSize.x,
            screenSize.y,
            0.0f,
            1.0f
        );
        a_commandBuffer.setViewport(0, 1, &viewport);

        pipeline = GetPipeline(-1, m_textUIPipelineAddr);
        IVERIFY(pipeline != nullptr);
        shaderData = pipeline->GetShaderData();
        IVERIFY(shaderData != nullptr);

        const uint32_t samplerAddr = text->GetSamplerAddr();
        const TextureSamplerBuffer& sampler = GetTextureSampler(samplerAddr);

        shaderData->PushTexture(a_commandBuffer, 0, sampler, a_index);

        break;
    }
    case UIElementType_Image:
    {
        const ImageUIElement* image = (ImageUIElement*)element;

        const uint32_t samplerAddr = image->GetSamplerAddr();
        if (samplerAddr == uint32_t(-1))
        {
            break;
        }

        const vk::Rect2D scissor = vk::Rect2D
        (
            {
                (int32_t)screenPos.x,
                (int32_t)screenPos.y
            },
            {
                (uint32_t)screenSize.x,
                (uint32_t)screenSize.y
            }
        );
        a_commandBuffer.setScissor(0, 1, &scissor);
        const vk::Viewport viewport = vk::Viewport
        (
            screenPos.x,
            screenPos.y,
            screenSize.x,
            screenSize.y,
            0.0f,
            1.0f
        );
        a_commandBuffer.setViewport(0, 1, &viewport);

        pipeline = GetPipeline(-1, m_imageUIPipelineAddr);
        IVERIFY(pipeline != nullptr);
        shaderData = pipeline->GetShaderData();
        IVERIFY(shaderData != nullptr);

        const TextureSamplerBuffer& sampler = GetTextureSampler(samplerAddr);

        shaderData->PushTexture(a_commandBuffer, 0, sampler, a_index);

        break;
    }
    default:
    {
        IERROR("Invalid UIElement Type");

        break;
    }
    }

    const bool set = pipeline != nullptr && shaderData != nullptr;
    if (set)
    {
        pipeline->Bind(a_index, a_commandBuffer);
        shaderData->UpdateUIBuffer(a_commandBuffer, element);

        a_commandBuffer.draw(4, 1, 0, 0);
    }

    const uint32_t childCount = element->GetChildCount();
    const uint32_t* children = element->GetChildren();
    for (uint32_t i = 0; i < childCount; ++i)
    {
        DrawUIElement(a_commandBuffer, children[i], a_canvas, a_screenSize, a_index);
    }
}

typedef VulkanCommandBuffer (VulkanGraphicsEngine::*DrawFunc)(uint32_t a_camIndex, uint32_t a_bufferIndex, uint32_t a_index);

struct DrawCallBind
{
    VulkanGraphicsEngine* Engine;

    uint32_t CamIndex;
    uint32_t BufferIndex;
    uint32_t FrameIndex;

    DrawFunc Function;

    DrawCallBind() = default;
    DrawCallBind(const DrawCallBind& a_other) = default;
    DrawCallBind(VulkanGraphicsEngine* a_engine, uint32_t a_camIndex, uint32_t a_bufferIndex, uint32_t a_frameIndex, DrawFunc a_function)
    {
        Engine = a_engine;

        CamIndex = a_camIndex;
        BufferIndex = a_bufferIndex;
        FrameIndex = a_frameIndex;

        Function = a_function;
    }

    inline VulkanCommandBuffer operator()() const
    {
        return (Engine->*Function)(CamIndex, BufferIndex, FrameIndex);
    }
};

IcarianCore::Array<VulkanCommandBuffer> VulkanGraphicsEngine::Update(double a_delta, double a_time, uint32_t a_index)
{
    RENDERSCRATCHFRAME;

    // TODO: Prebuild camera uniform buffers
    Profiler::StartFrame("Drawing Setup");
    m_renderCommands.Clear();

    IcarianCore::Allocator* blockAllocator = m_vulkanEngine->GetAllocator();
    IcarianCore::StackAllocator* scratchAllocator = RenderScratchAlloc::GetAllocator();

    {
        RENDERSCRATCHFRAME;

        const uint32_t size = m_shaderPrograms.Size();
        const IcarianCore::Array<uint8_t> state = m_shaderPrograms.ToPackedStateArray(scratchAllocator);
        TLockArray<RenderProgram> a = m_shaderPrograms.ToLockArray();

        for (uint32_t i = 0; i < size; ++i)
        {
            const uint32_t index = i / 8;
            const uint32_t offset = i % 8;

            if (!IISBITSET(state[index], offset))
            {
                continue;
            }

            const RenderProgram& program = a[i];
            IVERIFY(program.Data != nullptr);

            const VulkanRenderProgramBlob* blob = (VulkanRenderProgramBlob*)program.Data;
            if (blob->Shadow != nullptr)
            {
                blob->Shadow->Update(a_index, program);
            }

            if (blob->Base != nullptr)
            {
                blob->Base->Update(a_index, program);
            }

            if (blob->Secondary != nullptr)
            {
                blob->Secondary->Update(a_index, program);
            }

            if (blob->Tertiary != nullptr)
            {
                blob->Tertiary->Update(a_index, program);
            }
        }

        const IcarianCore::ShaderTimeBuffer timeBuffer =
        {
            .Time = glm::vec2((float)a_delta, (float)a_time)
        };

        m_timeUniform->SetData(a_index, &timeBuffer);
    }

    const vk::Device device = m_vulkanEngine->GetLogicalDevice();

    const uint32_t camBufferSize = m_cameraBuffers.Size();
    IcarianCore::Array<uint32_t> camIndices = IcarianCore::Array<uint32_t>(scratchAllocator);
    for (uint32_t i = 0; i < camBufferSize; ++i)
    {
        if (m_cameraBuffers[i].TransformAddr != uint32_t(-1))
        {
            camIndices.Push(i);
        }
    }

    TReadLockArray<CanvasRendererBuffer> canvasBuffer = m_canvasRenderers.ToReadLockArray();
    const uint32_t canvasCount = canvasBuffer.Size();
    const uint32_t camIndexSize = camIndices.Size();
    const uint32_t totalPoolSize = camIndexSize * DrawingPassCount + canvasCount;
    const uint32_t poolSize = (uint32_t)m_commandPool[a_index]->Size();

    if (poolSize < totalPoolSize)
    {
        TRACE("Allocating graphics command pools");
        const vk::CommandPoolCreateInfo poolInfo = vk::CommandPoolCreateInfo
        (
            vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
            m_vulkanEngine->GetGraphicsQueueIndex()
        );

        const uint32_t diff = totalPoolSize - poolSize;
        for (uint32_t i = 0; i < diff; ++i)
        {
            vk::CommandPool pool;
            VKRESERRMSG(device.createCommandPool(&poolInfo, nullptr, &pool), "Failed to create graphics command pool");

            m_commandPool[a_index]->Push(pool);

            const vk::CommandBufferAllocateInfo commandBufferInfo = vk::CommandBufferAllocateInfo
            (
                pool,
                vk::CommandBufferLevel::ePrimary,
                1
            );

            vk::CommandBuffer buffer;
            VKRESERRMSG(device.allocateCommandBuffers(&commandBufferInfo, &buffer), "Failed to allocate graphics command buffer");

            m_commandBuffers[a_index]->Push(buffer);
        }
    }

    const uint32_t camUniformSize = m_cameraUniforms.Size();

    if (camUniformSize < totalPoolSize)
    {
        TRACE("Allocating camera ubos");
        const uint32_t diff = totalPoolSize - camUniformSize;
        for (uint32_t i = 0; i < diff; ++i)
        {
            VulkanUniformBuffer* buffer = blockAllocator->Create<VulkanUniformBuffer>(m_vulkanEngine, sizeof(IcarianCore::ShaderCameraBuffer));

            m_cameraUniforms.Push(buffer);
        }
    }

    for (uint32_t i = 0; i < poolSize; ++i)
    {
        device.resetCommandPool((*m_commandPool[a_index])[i]);
    }

    Profiler::StopFrame();

    PROFILESTACK("Drawing Cmd");

    // Do not mind this ungodly type name just STL things
    // Having to use vector because std::future is being a bitch
    std::vector<std::future<VulkanCommandBuffer>, STLRenderScratchAlloc<std::future<VulkanCommandBuffer>>> futures;
    futures.reserve(camIndexSize * DrawingPassCount);
    for (uint32_t i = 0; i < camIndexSize; ++i)
    {
        const uint32_t camIndex = camIndices[i];
        const uint32_t poolIndex = i * DrawingPassCount;

        FThreadJob<VulkanCommandBuffer, DrawCallBind>* dirShadowJob = IcarianCore::MallocAllocator::Instance->Create<FThreadJob<VulkanCommandBuffer, DrawCallBind>>
        (
            DrawCallBind(this, camIndex, poolIndex + 0, a_index, &VulkanGraphicsEngine::DirectionalShadowPass),
            JobPriority_EngineUrgent
        );
        futures.emplace_back(dirShadowJob->GetFuture());
        ThreadPool::PushJob(dirShadowJob);

        FThreadJob<VulkanCommandBuffer, DrawCallBind>* pointShadowJob = IcarianCore::MallocAllocator::Instance->Create<FThreadJob<VulkanCommandBuffer, DrawCallBind>>
        (
            DrawCallBind(this, camIndex, poolIndex + 1, a_index, &VulkanGraphicsEngine::PointShadowPass),
            JobPriority_EngineUrgent
        );
        futures.emplace_back(pointShadowJob->GetFuture());
        ThreadPool::PushJob(pointShadowJob);

        FThreadJob<VulkanCommandBuffer, DrawCallBind>* spotShadowJob = IcarianCore::MallocAllocator::Instance->Create<FThreadJob<VulkanCommandBuffer, DrawCallBind>>
        (
            DrawCallBind(this, camIndex, poolIndex + 2, a_index, &VulkanGraphicsEngine::SpotShadowPass),
            JobPriority_EngineUrgent
        );
        futures.emplace_back(spotShadowJob->GetFuture());
        ThreadPool::PushJob(spotShadowJob);

        FThreadJob<VulkanCommandBuffer, DrawCallBind>* drawJob = IcarianCore::MallocAllocator::Instance->Create<FThreadJob<VulkanCommandBuffer, DrawCallBind>>
        (
            DrawCallBind(this, camIndex, poolIndex + 3, a_index, &VulkanGraphicsEngine::DrawPass),
            JobPriority_EngineUrgent
        );
        futures.emplace_back(drawJob->GetFuture());
        ThreadPool::PushJob(drawJob);

        FThreadJob<VulkanCommandBuffer, DrawCallBind>* lightJob = IcarianCore::MallocAllocator::Instance->Create<FThreadJob<VulkanCommandBuffer, DrawCallBind>>
        (
            DrawCallBind(this, camIndex, poolIndex + 4, a_index, &VulkanGraphicsEngine::LightPass),
            JobPriority_EngineUrgent
        );
        futures.emplace_back(lightJob->GetFuture());
        ThreadPool::PushJob(lightJob);

        FThreadJob<VulkanCommandBuffer, DrawCallBind>* forwardJob = IcarianCore::MallocAllocator::Instance->Create<FThreadJob<VulkanCommandBuffer, DrawCallBind>>
        (
            DrawCallBind(this, camIndex, poolIndex + 5, a_index, &VulkanGraphicsEngine::ForwardPass),
            JobPriority_EngineUrgent
        );
        futures.emplace_back(forwardJob->GetFuture());
        ThreadPool::PushJob(forwardJob);

        FThreadJob<VulkanCommandBuffer, DrawCallBind>* postJob = IcarianCore::MallocAllocator::Instance->Create<FThreadJob<VulkanCommandBuffer, DrawCallBind>>
        (
            DrawCallBind(this, camIndex, poolIndex + 6, a_index, &VulkanGraphicsEngine::PostPass),
            JobPriority_EngineUrgent
        );
        futures.emplace_back(postJob->GetFuture());
        ThreadPool::PushJob(postJob);
    }

    IcarianCore::Array<VulkanCommandBuffer> cmdBuffers = IcarianCore::Array<VulkanCommandBuffer>(blockAllocator);

    IcarianCore::Array<vk::CommandBuffer> uiBuffers = IcarianCore::Array<vk::CommandBuffer>(blockAllocator);
    {
        PROFILESTACK("UI Draw");

        RENDERSCRATCHFRAME;

        const IcarianCore::Array<CanvasRendererBuffer> a = m_canvasRenderers.ToActiveArray(scratchAllocator);

        const uint32_t canvasArrSize = a.Size();
        for (uint32_t i = 0; i < canvasArrSize; ++i)
        {
            const CanvasRendererBuffer& canvasRenderer = a[i];

            const bool valid = canvasRenderer.CanvasAddr != uint32_t(-1);
            if (!valid)
            {
                continue;
            }

            const CanvasBuffer& canvas = UIControl::GetCanvas(canvasRenderer.CanvasAddr);

            vk::CommandBuffer buffer = StartCommandBuffer(camIndexSize * DrawingPassCount + i, a_index);
            IDEFER(buffer.end());

            VULKAN_MARKER_COL(m_vulkanEngine, buffer, "UI Pass", 255, 255, 255);

            const VulkanRenderTexture* renderTexture = GetRenderTexture(canvasRenderer.RenderTextureAddr);

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

            const vk::Rect2D rect = vk::Rect2D({ 0, 0 }, { screenWidth, screenHeight });

            if (renderTexture != nullptr)
            {
                const vk::RenderPass pass = renderTexture->GetRenderPassNoClear();
                const vk::Framebuffer framebuffer = renderTexture->GetFramebuffer();

                const uint32_t clearCount = renderTexture->GetTotalTextureCount();
                const vk::ClearValue* clearValues = renderTexture->GetClearValues();

                const vk::RenderPassBeginInfo renderPassInto = vk::RenderPassBeginInfo
                (
                    pass,
                    framebuffer,
                    rect,
                    clearCount,
                    clearValues
                );

                buffer.beginRenderPass(renderPassInto, vk::SubpassContents::eInline);
            }
            else
            {
                const uint32_t imageIndex = m_vulkanEngine->GetImageIndex();

                const vk::RenderPass pass = m_swapchain->GetRenderPassNoClear();
                const vk::Framebuffer framebuffer = m_swapchain->GetFramebuffer(imageIndex);

                constexpr vk::ClearValue ClearColor = vk::ClearValue();

                const vk::RenderPassBeginInfo renderPassInfo = vk::RenderPassBeginInfo
                (
                    pass,
                    framebuffer,
                    rect,
                    1,
                    &ClearColor
                );

                buffer.beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);
            }
            IDEFER(buffer.endRenderPass());

            const glm::vec2 screenSize = glm::vec2(screenWidth, screenHeight);

            for (uint32_t i = 0; i < canvas.ChildCount; ++i)
            {
                DrawUIElement(buffer, canvas.ChildElements[i], canvas, screenSize, a_index);
            }

            uiBuffers.Push(buffer);
        }
    }

    {
        PROFILESTACK("Draw Wait");

        for (std::future<VulkanCommandBuffer>& f : futures)
        {
            f.wait();

            const VulkanCommandBuffer buffer = f.get();
            if (buffer.GetCommandBuffer() != vk::CommandBuffer(nullptr))
            {
                cmdBuffers.Push(buffer);
            }
        }

        if (!uiBuffers.Empty())
        {
            for (const vk::CommandBuffer& buffer : uiBuffers)
            {
                if (buffer == vk::CommandBuffer(nullptr))
                {
                    continue;
                }

                const VulkanCommandBuffer vBuffer = VulkanCommandBuffer(buffer, VulkanCommandBufferType_Graphics, VulkanCommandBufferStage_UIPass);

                cmdBuffers.Push(vBuffer);
            }
        }
    }

    return cmdBuffers;
}

CameraBuffer VulkanGraphicsEngine::GetCameraBuffer(uint32_t a_addr)
{
    IVERIFY(a_addr < m_cameraBuffers.Size());

    return m_cameraBuffers[a_addr];
}

uint32_t VulkanGraphicsEngine::GenerateMesh
(
    const void* a_vertices,
    uint32_t a_vertexCount,
    uint16_t a_vertexStride,
    const uint32_t* a_meshletVertices,
    uint32_t a_meshletVertexCount,
    const uint8_t* a_meshletTriangles,
    uint32_t a_meshletTriangleCount,
    const IcarianCore::ShaderMeshletBuffer* a_meshlets,
    uint32_t a_meshletCount,
    float a_radius
)
{
    IVERIFY(a_vertices != nullptr);
    IVERIFY(a_vertexCount > 0);
    IVERIFY(a_vertexStride > 0);
    IVERIFY(a_meshletVertices != nullptr);
    IVERIFY(a_meshletVertexCount > 0);
    IVERIFY(a_meshletTriangles != nullptr);
    IVERIFY(a_meshletTriangleCount > 0);
    IVERIFY(a_meshlets != nullptr);
    IVERIFY(a_meshletCount > 0);

    IcarianCore::Allocator* allocator = m_vulkanEngine->GetAllocator();

    VulkanMesh* mesh = allocator->Create<VulkanMesh>
    (
        m_vulkanEngine,
        a_vertices,
        a_vertexCount,
        a_vertexStride,
        a_meshletVertices,
        a_meshletVertexCount,
        a_meshletTriangles,
        a_meshletTriangleCount,
        a_meshlets,
        a_meshletCount,
        a_radius
    );

    return m_meshes.PushVal(mesh);
}
void VulkanGraphicsEngine::DestroyMesh(uint32_t a_addr)
{
    IcarianCore::Allocator* allocator = m_vulkanEngine->GetAllocator();

    if (ISRENDERASSETSTOREADDR(a_addr))
    {
        const uint32_t addr = FROMRENDERSTOREADDR(a_addr);

        const RenderEngine* renderEngine = m_vulkanEngine->GetRenderEngine();
        RenderAssetStore* store = renderEngine->GetRenderAssetStore();

        store->DestroyMesh(addr);
    }
    else
    {
        IVERIFY(m_meshes.Exists(a_addr));

        VulkanMesh* mesh = m_meshes[a_addr];
        IDEFER(allocator->Destroy(mesh));

        m_meshes.Erase(a_addr);
    }
}
VulkanMesh* VulkanGraphicsEngine::GetMesh(uint32_t a_addr)
{
    if (a_addr == uint32_t(-1))
    {
        return nullptr;
    }

    const uint32_t addr = ILAMBDA(
    {
        if (ISRENDERASSETSTOREADDR(a_addr))
        {
            const uint32_t storeAddr = FROMRENDERSTOREADDR(a_addr);

            const RenderEngine* renderEngine = m_vulkanEngine->GetRenderEngine();
            RenderAssetStore* store = renderEngine->GetRenderAssetStore();

            ILRETURN store->GetMesh(storeAddr);
        }

        ILRETURN a_addr;
    });

    IVERIFY(m_meshes.Exists(addr));

    return m_meshes[addr];
}

uint32_t VulkanGraphicsEngine::GenerateModel
(
    const void* a_vertices,
    uint32_t a_vertexCount,
    uint16_t a_vertexStride,
    const uint32_t* a_indices,
    uint32_t a_indexCount,
    float a_radius
)
{
    IVERIFY(a_vertices != nullptr);
    IVERIFY(a_vertexCount > 0);
    IVERIFY(a_indices != nullptr);
    IVERIFY(a_indexCount > 0);
    IVERIFY(a_vertexStride > 0);

    IcarianCore::Allocator* allocator = m_vulkanEngine->GetAllocator();

    VulkanModel* model = allocator->Create<VulkanModel>(m_vulkanEngine, a_vertexCount, a_vertices, a_vertexStride, a_indexCount, a_indices, a_radius);

    return m_models.PushVal(model);
}
void VulkanGraphicsEngine::DestroyModel(uint32_t a_addr)
{
    IcarianCore::Allocator* allocator = m_vulkanEngine->GetAllocator();

    if (ISRENDERASSETSTOREADDR(a_addr))
    {
        const uint32_t addr = FROMRENDERSTOREADDR(a_addr);

        const RenderEngine* renderEngine = m_vulkanEngine->GetRenderEngine();
        RenderAssetStore* store = renderEngine->GetRenderAssetStore();

        store->DestroyModel(addr);
    }
    else
    {
        IVERIFY(m_models.Exists(a_addr));

        VulkanModel* model = m_models[a_addr];
        IDEFER(allocator->Destroy(model));

        m_models.Erase(a_addr);
    }
}
VulkanModel* VulkanGraphicsEngine::GetModel(uint32_t a_addr)
{
    if (a_addr == uint32_t(-1))
    {
        return nullptr;
    }

    uint32_t finalAddr = a_addr;
    if (ISRENDERASSETSTOREADDR(a_addr))
    {
        const uint32_t addr = FROMRENDERSTOREADDR(a_addr);

        const RenderEngine* renderEngine = m_vulkanEngine->GetRenderEngine();
        RenderAssetStore* store = renderEngine->GetRenderAssetStore();

        finalAddr = store->GetModel(addr);
    }

    IVERIFY(finalAddr < m_models.Size());
    IVERIFY(m_models.Exists(finalAddr));

    return m_models[finalAddr];
}

uint32_t VulkanGraphicsEngine::GenerateTexture(uint32_t a_width, uint32_t a_height, e_TextureFormat a_format, const void* a_data)
{
    VulkanTexture* texture = VulkanTexture::CreateTexture(m_vulkanEngine, a_width, a_height, a_format, a_data);

    return m_textures.PushVal(texture);
}
uint32_t VulkanGraphicsEngine::GenerateMipMappedTexture(uint32_t a_width, uint32_t a_height, uint32_t a_levels, const uint64_t* a_offsets, e_TextureFormat a_format, const void* a_data, uint64_t a_dataSize)
{
    VulkanTexture* texture = VulkanTexture::CreateTextureMipMapped(m_vulkanEngine, a_width, a_height, a_levels, a_offsets, a_format, a_data, a_dataSize);

    return m_textures.PushVal(texture);
}
void VulkanGraphicsEngine::DestroyTexture(uint32_t a_addr)
{
    if (ISRENDERASSETSTOREADDR(a_addr))
    {
        const uint32_t addr = FROMRENDERSTOREADDR(a_addr);

        const RenderEngine* renderEngine = m_vulkanEngine->GetRenderEngine();
        RenderAssetStore* store = renderEngine->GetRenderAssetStore();

        store->DestroyTexture(addr);
    }
    else
    {
        IVERIFY(m_textures.Exists(a_addr));

        const VulkanTexture* texture = m_textures[a_addr];
        IDEFER(delete texture);

        m_textures.Erase(a_addr);
    }
}
VulkanTexture* VulkanGraphicsEngine::GetTexture(uint32_t a_addr)
{
    if (a_addr == uint32_t(-1))
    {
        return nullptr;
    }

    uint32_t finalAddr = a_addr;
    if (ISRENDERASSETSTOREADDR(a_addr))
    {
        const uint32_t addr = FROMRENDERSTOREADDR(a_addr);

        const RenderEngine* renderEngine = m_vulkanEngine->GetRenderEngine();
        RenderAssetStore* store = renderEngine->GetRenderAssetStore();

        finalAddr = store->GetTexture(addr);
    }

    IVERIFY(m_textures.Exists(finalAddr));

    return m_textures[finalAddr];
}

uint32_t VulkanGraphicsEngine::GenerateDepthRenderTexture(uint32_t a_width, uint32_t a_height)
{
    IVERIFY(a_width > 0);
    IVERIFY(a_height > 0);

    IcarianCore::Allocator* allocator = m_vulkanEngine->GetAllocator();

    VulkanDepthRenderTexture* texture = allocator->Create<VulkanDepthRenderTexture>(m_vulkanEngine, a_width, a_height);

    return m_depthRenderTextures.PushVal(texture);
}
void VulkanGraphicsEngine::DestroyDepthRenderTexture(uint32_t a_addr)
{
    IVERIFY(m_depthRenderTextures.Exists(a_addr));

    IcarianCore::Allocator* allocator = m_vulkanEngine->GetAllocator();

    VulkanDepthRenderTexture* tex = m_depthRenderTextures[a_addr];
    IDEFER(allocator->Destroy(tex));

    m_depthRenderTextures.Erase(a_addr);
}

VulkanRenderTexture* VulkanGraphicsEngine::GetRenderTexture(uint32_t a_addr)
{
    if (a_addr == uint32_t(-1))
    {
        return nullptr;
    }

    IVERIFY(m_renderTextures.Exists(a_addr));

    return m_renderTextures[a_addr];
}
VulkanDepthRenderTexture* VulkanGraphicsEngine::GetDepthRenderTexture(uint32_t a_addr)
{
    if (a_addr == uint32_t(-1))
    {
        return nullptr;
    }

    IVERIFY(m_depthRenderTextures.Exists(a_addr));

    return m_depthRenderTextures[a_addr];
}
VulkanDepthCubeRenderTexture* VulkanGraphicsEngine::GetDepthCubeRenderTexture(uint32_t a_addr)
{
    if (a_addr == uint32_t(-1))
    {
        return nullptr;
    }

    IVERIFY(m_depthCubeRenderTextures.Exists(a_addr));

    return m_depthCubeRenderTextures[a_addr];
}

AmbientLightBuffer VulkanGraphicsEngine::GetAmbientLight(uint32_t a_addr)
{
    IVERIFY(m_ambientLights.Exists(a_addr));

    return m_ambientLights[a_addr];
}
DirectionalLightBuffer VulkanGraphicsEngine::GetDirectionalLight(uint32_t a_addr)
{
    IVERIFY(m_directionalLights.Exists(a_addr));

    return m_directionalLights[a_addr];
}
PointLightBuffer VulkanGraphicsEngine::GetPointLight(uint32_t a_addr)
{
    IVERIFY(m_pointLights.Exists(a_addr));

    return m_pointLights[a_addr];
}
SpotLightBuffer VulkanGraphicsEngine::GetSpotLight(uint32_t a_addr)
{
    IVERIFY(m_spotLights.Exists(a_addr));

    return m_spotLights[a_addr];
}

uint32_t VulkanGraphicsEngine::GenerateTextureSampler(uint32_t a_textureAddr, e_TextureMode a_textureMode, e_TextureFilter a_filterMode, e_TextureAddress a_addressMode, uint32_t a_slot)
{
    switch (a_textureMode)
    {
    case TextureMode_Texture:
    {
        if (!ISRENDERASSETSTOREADDR(a_textureAddr))
        {
            IVERIFY(m_textures.Exists(a_textureAddr));
        }

        break;
    }
    case TextureMode_RenderTexture:
    {
        IVERIFY(m_renderTextures.Exists(a_textureAddr));
        IVERIFY(a_slot < m_renderTextures[a_textureAddr]->GetTextureCount());

        break;
    }
    case TextureMode_RenderTextureDepth:
    {
        IVERIFY(m_renderTextures.Exists(a_textureAddr));

        break;
    }
    case TextureMode_DepthRenderTexture:
    {
        IVERIFY(m_depthCubeRenderTextures.Exists(a_textureAddr));

        break;
    }
    default:
    {
        IERROR("GenerateTextureSampler Invalid Texture Mode");

        break;
    }
    }

    IcarianCore::Allocator* allocator = m_vulkanEngine->GetAllocator();

    VulkanTextureSampler* texSampler = allocator->TAllocate<VulkanTextureSampler>();

    const TextureSamplerBuffer sampler =
    {
        .Addr = a_textureAddr,
        .Slot = a_slot,
        .TextureMode = a_textureMode,
        .FilterMode = a_filterMode,
        .AddressMode = a_addressMode,
        .Data = texSampler
    };

    const VulkanTextureSamplerBuilder builder =
    {
        .Engine = m_vulkanEngine,
        .Sampler = sampler
    };

    VulkanTextureSampler::GenerateFromBuffer(texSampler, builder);

    return m_textureSampler.PushVal(sampler);
}
void VulkanGraphicsEngine::DestroyTextureSampler(uint32_t a_addr)
{
    IVERIFY(m_textureSampler.Exists(a_addr));

    IcarianCore::Allocator* allocator = m_vulkanEngine->GetAllocator();

    const TextureSamplerBuffer sampler = m_textureSampler[a_addr];
    IDEFER(
    {
        if (sampler.Data != nullptr)
        {
            allocator->Destroy((VulkanTextureSampler*)sampler.Data);
        }
    });

    m_textureSampler.Erase(a_addr);
}
TextureSamplerBuffer VulkanGraphicsEngine::GetTextureSampler(uint32_t a_addr)
{
    IVERIFY(m_textureSampler.Exists(a_addr));

    return m_textureSampler[a_addr];
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
