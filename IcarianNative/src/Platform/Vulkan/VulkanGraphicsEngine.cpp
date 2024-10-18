// Icarian Engine - C# Game Engine
// 
// License at end of file.

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/VulkanGraphicsEngine.h"

#include <future>
#include <glm/gtx/matrix_decompose.hpp>
#include <vulkan/vulkan_handles.hpp>

#include "Core/IcarianAssert.h"
#include "Core/IcarianDefer.h"
#include "Core/ShaderBuffers.h"
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
#include "Rendering/Vulkan/VulkanModel.h"
#include "Rendering/Vulkan/VulkanPipeline.h"
#include "Rendering/Vulkan/VulkanPixelShader.h"
#include "Rendering/Vulkan/VulkanPushPool.h"
#include "Rendering/Vulkan/VulkanRenderCommand.h"
#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"
#include "Rendering/Vulkan/VulkanRenderTexture.h"
#include "Rendering/Vulkan/VulkanShaderData.h"
#include "Rendering/Vulkan/VulkanShaderStorageObject.h"
#include "Rendering/Vulkan/VulkanSwapchain.h"
#include "Rendering/Vulkan/VulkanTexture.h"
#include "Rendering/Vulkan/VulkanTextureSampler.h"
#include "Rendering/Vulkan/VulkanUniformBuffer.h"
#include "Rendering/Vulkan/VulkanVertexShader.h"
#include "Rendering/Vulkan/VulkanVideoTexture.h"
#include "Runtime/RuntimeFunction.h"
#include "Runtime/RuntimeManager.h"
#include "Shaders.h"
#include "Trace.h"
#include "ThreadPool.h"

#include "EngineLightInteropStructures.h"

VulkanGraphicsEngine::VulkanGraphicsEngine(VulkanRenderEngineBackend* a_vulkanEngine)
{
    m_vulkanEngine = a_vulkanEngine;

    m_runtimeBindings = new VulkanGraphicsEngineBindings(this);

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

    const RenderProgram textProgram = 
    {
        .VertexShader = GenerateFVertexShader(UIVertexShader),
        .PixelShader = GenerateFPixelShader(UITextPixelShader),
        .ShadowVertexShader = uint32_t(-1),
        .ColorBlendMode = MaterialBlendMode_Alpha,
        .CullingMode = CullMode_None,
        .PrimitiveMode = PrimitiveMode_TriangleStrip,
        .Flags = 0b1 << RenderProgram::DestroyFlag
    };

    m_textUIPipelineAddr = GenerateRenderProgram(textProgram);

    const RenderProgram imageProgram = 
    {
        .VertexShader = GenerateFVertexShader(UIVertexShader),
        .PixelShader = GenerateFPixelShader(UIImagePixelShader),
        .ShadowVertexShader = uint32_t(-1),
        .ColorBlendMode = MaterialBlendMode_Alpha,
        .CullingMode = CullMode_None,
        .PrimitiveMode = PrimitiveMode_TriangleStrip,
        .Flags = 0b1 << RenderProgram::DestroyFlag
    };

    const uint32_t decodeIndex = m_vulkanEngine->GetVideoDecodeIndex();
    if (decodeIndex != -1)
    {
        TRACE("Allocating video decode command pools");
        const vk::CommandPoolCreateInfo poolInfo = vk::CommandPoolCreateInfo
        (
            vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
            decodeIndex
        );  

        const vk::Device device = m_vulkanEngine->GetLogicalDevice();

        for (uint32_t i = 0; i < VulkanFlightPoolSize; ++i)
        {
            VKRESERRMSG(device.createCommandPool(&poolInfo, nullptr, &m_decodePool[i]), "Failed to create video decode command pool");

            const vk::CommandBufferAllocateInfo commandBufferInfo = vk::CommandBufferAllocateInfo
            (
                m_decodePool[i],
                vk::CommandBufferLevel::ePrimary,
                1
            );

            VKRESERRMSG(device.allocateCommandBuffers(&commandBufferInfo, &m_decodeBuffer[i]), "Failed to allocate video decode command buffer");
        }
    }

    m_imageUIPipelineAddr = GenerateRenderProgram(imageProgram);

    m_timeUniform = new VulkanUniformBuffer(m_vulkanEngine, sizeof(IcarianCore::ShaderTimeBuffer));
}
VulkanGraphicsEngine::~VulkanGraphicsEngine()
{
    TRACE("Checking if shaders where deleted");
    for (uint32_t i = 0; i < m_vertexShaders.Size(); ++i)
    {
        if (m_vertexShaders.Exists(i))
        {
            Logger::Warning("Vertex Shader was not destroyed");

            delete m_vertexShaders[i];
        }
    }

    for (uint32_t i = 0; i < m_pixelShaders.Size(); ++i)
    {
        if (m_pixelShaders.Exists(i))
        {
            Logger::Warning("Pixel Shader was not destroyed");

            delete m_pixelShaders[i];
        }
    }

    TRACE("Deleting Pipelines");
    for (const auto& iter : m_pipelines)
    {
        delete iter.second;
    }
    TRACE("Deleting Shadow Pipelines");
    for (const auto& iter : m_shadowPipelines)
    {
        delete iter.second;
    }
    TRACE("Deleting Cube Shadow Pipelines");
    for (const auto& iter : m_cubeShadowPipelines)
    {
        delete iter.second;
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

            delete (VulkanShaderData*)m_shaderPrograms[i].Data;
            m_shaderPrograms[i].Data = nullptr;
        }
    }

    delete m_runtimeBindings;

    delete m_shadowSetupFunc;
    delete m_preShadowFunc;
    delete m_postShadowFunc;
    delete m_preRenderFunc;
    delete m_postRenderFunc;
    delete m_lightSetupFunc;
    delete m_preShadowLightFunc;
    delete m_postShadowLightFunc;
    delete m_preLightFunc;
    delete m_postLightFunc;
    delete m_preForwardFunc;
    delete m_postForwardFunc;
    delete m_postProcessFunc;
}

void VulkanGraphicsEngine::Cleanup()
{
    delete m_timeUniform;

    const RenderProgram textProgram = m_shaderPrograms[m_textUIPipelineAddr];
    IDEFER(
    if (textProgram.VertexAttributes != nullptr)
    {
        delete[] textProgram.VertexAttributes;
    });

    const RenderProgram imageProgram = m_shaderPrograms[m_imageUIPipelineAddr];
    IDEFER(
    if (imageProgram.VertexAttributes != nullptr)
    {
        delete[] imageProgram.VertexAttributes;
    });

    DestroyRenderProgram(m_textUIPipelineAddr);
    DestroyRenderProgram(m_imageUIPipelineAddr);

    const vk::Device device = m_vulkanEngine->GetLogicalDevice();

    TRACE("Deleting command pool");
    for (uint32_t i = 0; i < VulkanFlightPoolSize; ++i)
    {
        for (const vk::CommandPool& pool : m_commandPool[i])
        {
            device.destroyCommandPool(pool);
        }
    }

    const uint32_t decodeIndex = m_vulkanEngine->GetVideoDecodeIndex();
    if (decodeIndex != -1)
    {
        for (uint32_t i = 0; i < VulkanFlightPoolSize; ++i)
        {
            device.destroyCommandPool(m_decodePool[i]);
        }
    }

    TRACE("Deleting camera ubos");
    for (const VulkanUniformBuffer* uniform : m_cameraUniforms)
    {
        if (uniform != nullptr)
        {
            delete uniform;
        }
    }

    TRACE("Checking is particle emitters where deleted");
    for (uint32_t i = 0; i < m_particleEmitters.Size(); ++i)
    {
        if (m_particleEmitters.Exists(i))
        {
            Logger::Warning("Particle emitter was not destroyed");

            delete m_particleEmitters[i];
        }
    }

    TRACE("Checking if models where deleted");
    for (uint32_t i = 0; i < m_models.Size(); ++i)
    {
        if (m_models[i] != nullptr)
        {
            Logger::Warning("Model was not destroyed");

            delete m_models[i];
            m_models[i] = nullptr;
        }
    }

    TRACE("Checking camera buffer health");
    for (uint32_t i = 0; i < m_cameraBuffers.Size(); ++i)
    {
        if (m_cameraBuffers[i].TransformAddr != -1)
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
        }
    }
    for (uint32_t i = 0; i < m_depthCubeRenderTextures.Size(); ++i)
    {
        if (m_depthCubeRenderTextures.Exists(i))
        {
            Logger::Warning("Depth Cube Render Texture was not destroyed");
        }
    }
    for (uint32_t i = 0; i < m_depthRenderTextures.Size(); ++i)
    {
        if (m_depthRenderTextures.Exists(i))
        {
            Logger::Warning("Depth Render Texture was not destroyed");
        }
    }

    TRACE("Checking if textures where deleted");
    for (uint32_t i = 0; i < m_textures.Size(); ++i)
    {
        if (m_textures[i] != nullptr)
        {
            Logger::Warning("Texture was not destroyed");
            
            delete m_textures[i];
            m_textures[i] = nullptr;
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

            delete (VulkanTextureSampler*)m_textureSampler[i].Data;
            m_textureSampler[i].Data = nullptr;
        }
    }

    TRACE("Checking if video textures where deleted");
    for (uint32_t i = 0; i < m_videoTextures.Size(); ++i)
    {
        if (m_videoTextures[i] != nullptr)
        {
            Logger::Warning("Video Texture was not destroyed");

            delete m_videoTextures[i];
            m_videoTextures[i] = nullptr;
        }
    }
}

uint32_t VulkanGraphicsEngine::GenerateFVertexShader(const std::string_view& a_source)
{
    IVERIFY(!a_source.empty());

    const SharedThreadGuard g = SharedThreadGuard(m_importLock);
    VulkanVertexShader* shader = VulkanVertexShader::CreateFromFShader(m_vulkanEngine, m_vertexImports, a_source);

    return m_vertexShaders.PushVal(shader);
}
void VulkanGraphicsEngine::DestroyVertexShader(uint32_t a_addr)
{
    IVERIFY(m_vertexShaders.Exists(a_addr));

    const VulkanVertexShader* shader = m_vertexShaders[a_addr];
    IDEFER(delete shader);

    m_vertexShaders.Erase(a_addr);
}

uint32_t VulkanGraphicsEngine::GenerateFPixelShader(const std::string_view& a_source)
{
    IVERIFY(!a_source.empty());

    const SharedThreadGuard g = SharedThreadGuard(m_importLock);
    VulkanPixelShader* shader = VulkanPixelShader::CreateFromFShader(m_vulkanEngine, m_pixelImports, a_source);

    return m_pixelShaders.PushVal(shader);
}
void VulkanGraphicsEngine::DestroyPixelShader(uint32_t a_addr)
{
    IVERIFY(m_pixelShaders.Exists(a_addr));

    const VulkanPixelShader* shader = m_pixelShaders[a_addr];
    IDEFER(delete shader);

    m_pixelShaders.Erase(a_addr);
}

uint32_t VulkanGraphicsEngine::GenerateRenderProgram(const RenderProgram& a_program)
{
    IVERIFY(m_vertexShaders.Exists(a_program.VertexShader));
    IVERIFY(m_pixelShaders.Exists(a_program.PixelShader));

    TRACE("Creating Shader Program");
    RenderProgram p = a_program;
    p.Data = new VulkanShaderData(m_vulkanEngine, this, a_program);

    return m_shaderPrograms.PushVal(p);
}
void VulkanGraphicsEngine::DestroyRenderProgram(uint32_t a_addr)
{
    IVERIFY(m_shaderPrograms.Exists(a_addr));

    TRACE("Destroying Shader Program");

    const RenderProgram program = m_shaderPrograms[a_addr];
    IDEFER(
    {
        if (program.Data != nullptr)
        {
            delete (VulkanShaderData*)program.Data;
        }

        if (IISBITSET(program.Flags, RenderProgram::DestroyFlag))
        {
            DestroyVertexShader(program.VertexShader);
            DestroyPixelShader(program.PixelShader);
            
            if (program.ShadowVertexShader != -1)
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
            if (a[i]->GetMaterialAddr() == a_addr)
			{
                m_renderStacks.UErase(i);

                break;
			}
        }
    }

    {
        const ThreadGuard g = ThreadGuard(m_pipeLock);
        
        Array<uint64_t> keys;
        for (auto iter = m_pipelines.begin(); iter != m_pipelines.end(); ++iter)
        {
            const uint32_t val = (uint32_t)(iter->first >> 32);
            if (val == a_addr)
            {
                keys.Push(iter->first);
            }
        }

        for (const uint64_t key : keys)
        {
            const VulkanPipeline* pipeline = m_pipelines[key];
            IDEFER(delete pipeline);

            m_pipelines.erase(key);
        }
    }

    if (program.ShadowVertexShader != -1)
    {
        {
            const ThreadGuard g = ThreadGuard(m_shadowPipeLock);

            Array<uint64_t> keys;
            for (auto iter = m_shadowPipelines.begin(); iter != m_shadowPipelines.end(); ++iter)
            {
                const uint32_t val = (uint32_t)(iter->first >> 32);
                if (val == a_addr)
                {
                    keys.Push(iter->first);
                }
            }

            for (const uint64_t key : keys)
            {
                const VulkanPipeline* pipeline = m_shadowPipelines[key];
                IDEFER(delete pipeline);

                m_shadowPipelines.erase(key);
            }
        }
        
        {
            const ThreadGuard g = ThreadGuard(m_cubeShadowPipeLock);

            Array<uint64_t> keys;
            for (auto iter = m_cubeShadowPipelines.begin(); iter != m_cubeShadowPipelines.end(); ++iter)
            {
                const uint32_t val = (uint32_t)(iter->first >> 32);
                if (val == a_addr)
                {
                    keys.Push(iter->first);
                }
            }

            for (const uint64_t key : keys)
            {
                const VulkanPipeline* pipeline = m_cubeShadowPipelines[key];
                IDEFER(delete pipeline);

                m_cubeShadowPipelines.erase(key);
            }
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

    const uint64_t addr = (uint64_t)a_renderTexture | (uint64_t)a_pipeline << 32;

    const ThreadGuard g = ThreadGuard(m_shadowPipeLock);
    const auto iter = m_shadowPipelines.find(addr);
    if (iter != m_shadowPipelines.end())
    {
        return iter->second;
    }

    TRACE("Allocating Vulkan Shadow Pipeline");
    const VulkanDepthRenderTexture* tex = GetDepthRenderTexture(a_renderTexture);
    IVERIFY(tex != nullptr);

    const vk::RenderPass pass = tex->GetRenderPass();

    VulkanPipeline* pipeline = VulkanPipeline::CreateShadowPipeline(m_vulkanEngine, this, pass, a_pipeline);

    m_shadowPipelines.emplace(addr, pipeline);

    return pipeline;
}
VulkanPipeline* VulkanGraphicsEngine::GetCubeShadowPipeline(uint32_t a_renderTexture, uint32_t a_pipeline)
{
    IVERIFY(m_shaderPrograms.Exists(a_pipeline));

    const uint64_t addr = (uint64_t)a_renderTexture | (uint64_t)a_pipeline << 32;

    const ThreadGuard g = ThreadGuard(m_cubeShadowPipeLock);
    const auto iter = m_cubeShadowPipelines.find(addr);
    if (iter != m_cubeShadowPipelines.end())
    {
        return iter->second;
    }

    TRACE("Allocating Vulkan Cube Shadow Pipeline");
    const VulkanDepthCubeRenderTexture* tex = GetDepthCubeRenderTexture(a_renderTexture);
    IVERIFY(tex != nullptr);

    const vk::RenderPass pass = tex->GetRenderPass();

    VulkanPipeline* pipeline = VulkanPipeline::CreateShadowPipeline(m_vulkanEngine, this, pass, a_pipeline);

    m_cubeShadowPipelines.emplace(addr, pipeline);

    return pipeline;    
}
VulkanPipeline* VulkanGraphicsEngine::GetPipeline(uint32_t a_renderTexture, uint32_t a_pipeline)
{
    IVERIFY(m_shaderPrograms.Exists(a_pipeline));

    const uint64_t addr = (uint64_t)a_renderTexture | (uint64_t)a_pipeline << 32;

    const ThreadGuard g = ThreadGuard(m_pipeLock);
    const auto iter = m_pipelines.find(addr);
    if (iter != m_pipelines.end())
    {
        return iter->second;
    }

    TRACE("Allocating Vulkan Pipeline");
    vk::RenderPass pass = m_swapchain->GetRenderPass();
    bool hasDepth = false;
    uint32_t textureCount = 1;

    const VulkanRenderTexture* tex = GetRenderTexture(a_renderTexture);
    if (tex != nullptr)
    {
        pass = tex->GetRenderPass();
        hasDepth = tex->HasDepthTexture();
        textureCount = tex->GetTextureCount();
    }

    VulkanPipeline* pipeline = VulkanPipeline::CreatePipeline(m_vulkanEngine, this, pass, hasDepth, textureCount, a_pipeline);

    m_pipelines.emplace(addr, pipeline);

    return pipeline;
}

vk::CommandBuffer VulkanGraphicsEngine::StartCommandBuffer(uint32_t a_bufferIndex, uint32_t a_index) const
{
    const vk::CommandBuffer commandBuffer = m_commandBuffers[a_index][a_bufferIndex];

    constexpr vk::CommandBufferBeginInfo BeginInfo;
    commandBuffer.begin(BeginInfo);

    return commandBuffer;
}

void VulkanGraphicsEngine::Draw(bool a_forward, const CameraBuffer& a_camBuffer, const Frustum& a_frustum, VulkanRenderCommand* a_renderCommand, uint32_t a_frameIndex)
{
    vk::CommandBuffer commandBuffer = a_renderCommand->GetCommandBuffer();

    const TReadLockArray<MaterialRenderStack*> stacks = m_renderStacks.ToReadLockArray();
    const Array<RenderProgram> programs = m_shaderPrograms.ToArray();

    for (const MaterialRenderStack* renderStack : stacks)
    {
        const uint32_t matAddr = renderStack->GetMaterialAddr();
        
        const RenderProgram& program = programs.Get(matAddr);
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

        const VulkanPipeline* pipeline = a_renderCommand->BindMaterial(matAddr);
        IVERIFY(pipeline != nullptr);
        const VulkanShaderData* shaderData = (VulkanShaderData*)program.Data;
        IVERIFY(shaderData != nullptr);

        {
            PROFILESTACK("Models");

            const uint32_t modelCount = renderStack->GetModelBufferCount();
            const ModelBuffer* modelBuffers = renderStack->GetModelBuffers();
            for (uint32_t i = 0; i < modelCount; ++i)
            {
                const ModelBuffer& modelBuffer = modelBuffers[i];
                const bool valid = modelBuffer.ModelAddr != -1;
                if (!valid)
                {
                    continue;
                }

                const VulkanModel* model = GetModel(modelBuffer.ModelAddr);
                IVERIFY(model != nullptr);
                    
                const float radius = model->GetRadius();
                const uint32_t indexCount = model->GetIndexCount();

                glm::mat4* transforms = new glm::mat4[modelBuffer.TransformCount];
                IDEFER(delete[] transforms);
                uint32_t transformCount = 0;

                {
                    PROFILESTACK("Culling");
                    for (uint32_t j = 0; j < modelBuffer.TransformCount; ++j)
                    {
                        const uint32_t transformAddr = modelBuffer.TransformAddr[j];
                        if (transformAddr != -1)
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
                    IcarianCore::ShaderModelBuffer* modelBuffer = new IcarianCore::ShaderModelBuffer[transformCount];
                    IDEFER(delete[] modelBuffer);

                    for (uint32_t j = 0; j < transformCount; ++j)
                    {
                        const glm::mat4& mat = transforms[j];
                        modelBuffer[j].Model = mat;
                        modelBuffer[j].InvModel = glm::inverse(mat);
                    }

                    const VulkanShaderStorageObject* storage = new VulkanShaderStorageObject(m_vulkanEngine, sizeof(IcarianCore::ShaderModelBuffer) * transformCount, transformCount, modelBuffer);
                    IDEFER(delete storage);

                    shaderData->PushShaderStorageObject(commandBuffer, modelSlot.Slot, storage, a_frameIndex);
                    
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
        }   
            
        {
            PROFILESTACK("Skinned");

            const uint32_t skinnedModelCount = renderStack->GetSkinnedModelBufferCount();
            const SkinnedModelBuffer* skinnedModelBuffers = renderStack->GetSkinnedModelBuffers();
            for (uint32_t i = 0; i < skinnedModelCount; ++i)
            {
                const SkinnedModelBuffer& modelBuffer = skinnedModelBuffers[i];
                const bool valid = modelBuffer.ModelAddr != -1;
                if (!valid)
                {
                    continue;
                }

                const VulkanModel* model = GetModel(modelBuffer.ModelAddr);
                IVERIFY(model != nullptr);
                    
                bool modelBound = false;

                const float radius = model->GetRadius();
                const uint32_t indexCount = model->GetIndexCount();

                const uint32_t objectCount = modelBuffer.ObjectCount;
                for (uint32_t j = 0; j < objectCount; ++j)
                {
                    const uint32_t transformAddr = modelBuffer.TransformAddr[j];
                    const bool valid = transformAddr != -1;
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

                        std::unordered_map<uint32_t, uint32_t> boneMap;
                        for (uint32_t i = 0; i < boneCount; ++i)
                        {
                            boneMap.emplace(skeleton.BoneData[i].TransformIndex, i);
                        }

                        IcarianCore::ShaderBoneBuffer* boneBuffer = new IcarianCore::ShaderBoneBuffer[boneCount];
                        IDEFER(delete[] boneBuffer);

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

                        const VulkanShaderStorageObject* storage = new VulkanShaderStorageObject(m_vulkanEngine, sizeof(IcarianCore::ShaderBoneBuffer) * boneCount, boneCount, boneBuffer);
                        IDEFER(delete storage);

                        shaderData->PushShaderStorageObject(commandBuffer, boneSlot.Slot, storage, a_frameIndex);
                    }    

                    shaderData->UpdateTransformBuffer(commandBuffer, transform);

                    commandBuffer.drawIndexed(indexCount, 1, 0, 0, 0);    
                }
            }
        }
    }
}
void VulkanGraphicsEngine::DrawShadow(const glm::mat4& a_lvp, float a_split, const glm::vec2& a_bias, uint32_t a_renderLayer, uint32_t a_renderTexture, bool a_cube, vk::CommandBuffer a_commandBuffer, uint32_t a_frameIndex)
{
    PROFILESTACK("Rendering");
    VulkanUniformBuffer* shadowLightBuffer = nullptr;

    VulkanPushPool* pushPool = m_vulkanEngine->GetPushPool();

    const Frustum frustum = Frustum::FromMat4(a_lvp);

    const TReadLockArray<MaterialRenderStack*> stacks = m_renderStacks.ToReadLockArray();
    for (const MaterialRenderStack* renderStack : stacks) 
    {
        const uint32_t materialAddr = renderStack->GetMaterialAddr();
        const RenderProgram& program = m_shaderPrograms[materialAddr];
        IVERIFY(program.Data != nullptr);

        VulkanShaderData* shaderData = (VulkanShaderData*)program.Data;

        if (a_renderLayer & program.RenderLayer && program.ShadowVertexShader != -1) 
        {
            VulkanPipeline* pipeline = nullptr;

            ShaderBufferInput shadowLightInput;
            if (shaderData->GetShadowShaderBufferInput(ShaderBufferType_ShadowLightBuffer, &shadowLightInput)) 
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

                shaderData->PushShadowUniformBuffer(a_commandBuffer, shadowLightInput.Slot, shadowLightBuffer, a_frameIndex);
            } 
            else 
            {
                shaderData->UpdateShadowLightBuffer(a_commandBuffer, a_lvp, a_split);
            }

            {
                PROFILESTACK("Models");

                const uint32_t modelCount = renderStack->GetModelBufferCount();
                const ModelBuffer* modelBuffers = renderStack->GetModelBuffers();

                for (uint32_t i = 0; i < modelCount; ++i) 
                {
                    const ModelBuffer& modelBuffer = modelBuffers[i];
                
                    if (modelBuffer.ModelAddr != -1) 
                    {
                        const VulkanModel* model = GetModel(modelBuffer.ModelAddr);
                        IVERIFY(model != nullptr);

                        const uint32_t indexCount = model->GetIndexCount();
                        const float radius = model->GetRadius();

                        const uint32_t transformCount = modelBuffer.TransformCount;
                    
                        glm::mat4* transforms = new glm::mat4[transformCount];
                        IDEFER(delete[] transforms);
                        uint32_t finalTransformCount = 0;

                        {
                            PROFILESTACK("Culling");
                            for (uint32_t j = 0; j < transformCount; ++j) 
                            {
                                const uint32_t transformAddr = modelBuffer.TransformAddr[j];
                                if (transformAddr == -1) 
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

                        if (pipeline == nullptr) 
                        {
                            if (!a_cube) 
                            {
                                pipeline = GetShadowPipeline(a_renderTexture, materialAddr);
                            } 
                            else 
                            {
                                pipeline = GetCubeShadowPipeline(a_renderTexture, materialAddr);
                            }

                            pipeline->Bind(a_frameIndex, a_commandBuffer);

                            a_commandBuffer.setDepthBias(a_bias.x, 0.0f, a_bias.y);
                        }

                        model->Bind(a_commandBuffer);

                        ShaderBufferInput modelSlot;
                        if (shaderData->GetShadowShaderBufferInput(ShaderBufferType_SSModelBuffer, &modelSlot)) 
                        {
                            IcarianCore::ShaderModelBuffer* modelBuffer = new IcarianCore::ShaderModelBuffer[finalTransformCount];
                            IDEFER(delete[] modelBuffer);

                            for (uint32_t j = 0; j < finalTransformCount; ++j) 
                            {
                                const glm::mat4& mat = transforms[j];

                                modelBuffer[j].Model = mat;
                                modelBuffer[j].InvModel = glm::inverse(mat);
                            }

                            const VulkanShaderStorageObject* storage = new VulkanShaderStorageObject(m_vulkanEngine, sizeof(IcarianCore::ShaderModelBuffer) * finalTransformCount, finalTransformCount, modelBuffer);
                            IDEFER(delete storage);

                            shaderData->PushShadowShaderStorageObject(a_commandBuffer, modelSlot.Slot, storage, a_frameIndex);

                            a_commandBuffer.drawIndexed(indexCount, finalTransformCount, 0, 0, 0);
                        } 
                        else 
                        {
                            for (uint32_t i = 0; i < finalTransformCount; ++i)
                            {
                                shaderData->UpdateShadowTransformBuffer(a_commandBuffer, transforms[i]);

                                a_commandBuffer.drawIndexed(indexCount, 1, 0, 0, 0);
                            }
                        }
                    }
                }
            }
            
            {
                PROFILESTACK("Skinned");

                const uint32_t skinnedModelCount = renderStack->GetSkinnedModelBufferCount();
                const SkinnedModelBuffer* skinnedModelBuffers = renderStack->GetSkinnedModelBuffers();

                for (uint32_t i = 0; i < skinnedModelCount; ++i)
                {
                    const SkinnedModelBuffer& modelBuffer = skinnedModelBuffers[i];

                    if (modelBuffer.ModelAddr == -1)
                    {
                        continue;
                    }

                    const VulkanModel* model = nullptr;

                    const uint32_t indexCount = model->GetIndexCount();
                    const float radius = model->GetRadius();

                    const uint32_t objectCont = modelBuffer.ObjectCount;
                    for (uint32_t j = 0; j < objectCont; ++j)
                    {
                        const uint32_t transformAddr = modelBuffer.TransformAddr[j];
                        if (transformAddr == -1)
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

                        if (pipeline == nullptr) 
                        {
                            if (!a_cube) 
                            {
                                pipeline = GetShadowPipeline(a_renderTexture, materialAddr);
                            } 
                            else 
                            {
                                pipeline = GetCubeShadowPipeline(a_renderTexture, materialAddr);
                            }

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
                            const SkeletonData skeleton = AnimationController::GetSkeleton(modelBuffer.SkeletonAddr[j]);
                            const uint32_t boneCount = (uint32_t)skeleton.BoneData.size();

                            std::unordered_map<uint32_t, uint32_t> boneMap;
                            boneMap.reserve(boneCount);
                            for (uint32_t i = 0; i < boneCount; ++i)
                            {
                                boneMap.emplace(skeleton.BoneData[i].TransformIndex, i);
                            }

                            IcarianCore::ShaderBoneBuffer* boneBuffer = new IcarianCore::ShaderBoneBuffer[boneCount];
                            IDEFER(delete[] boneBuffer);

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

                            const VulkanShaderStorageObject* storage = new VulkanShaderStorageObject(m_vulkanEngine, sizeof(IcarianCore::ShaderBoneBuffer) * boneCount, boneCount, boneBuffer);
                            IDEFER(delete storage);

                            shaderData->PushShaderStorageObject(a_commandBuffer, boneSlot.Slot, storage, a_frameIndex);
                        }

                        shaderData->UpdateTransformBuffer(a_commandBuffer, transform);

                        a_commandBuffer.drawIndexed(indexCount, 1, 0, 0, 0);
                    }
                }
            }
        }
    }
}

VulkanCommandBuffer VulkanGraphicsEngine::DirectionalShadowPass(uint32_t a_camIndex, uint32_t a_bufferIndex, uint32_t a_frameIndex)
{
    VulkanCommandBuffer vCmdBuffer = VulkanCommandBuffer(nullptr, VulkanCommandBufferType_Graphics);

    // Could possibly reverse the pass order and do culling on the previous pass to speed this up
    Profiler::Start("Dir Shadow Pass");
    IDEFER(Profiler::Stop());

    Profiler::StartFrame("Update");

    if (m_directionalLights.Size() == 0)
    {
        Profiler::StopFrame();

        return vCmdBuffer;
    }

    const CameraBuffer& camBuffer = m_cameraBuffers[a_camIndex];

    const vk::CommandBuffer commandBuffer = StartCommandBuffer(a_bufferIndex, a_frameIndex);
    IDEFER(commandBuffer.end());
    vCmdBuffer.SetCommandBuffer(commandBuffer);

    VULKAN_MARKER_COL(m_vulkanEngine, commandBuffer, "Directional Shadow Pass", 255, 0, 0);

    VulkanRenderCommand& renderCommand = m_renderCommands.Push(VulkanRenderCommand(m_vulkanEngine, this, m_swapchain, commandBuffer, -1, a_bufferIndex));
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
    }

    const Array<DirectionalLightBuffer> lights = m_directionalLights.ToArray();
    const Array<bool> state = m_directionalLights.ToStateArray();
    const uint32_t size = lights.Size();

    for (uint32_t i = 0; i < size; ++i)
    {
        if (!state[i])
        {
            continue;
        }

        const DirectionalLightBuffer& buffer = lights[i];
        if (buffer.TransformAddr == -1 || (buffer.RenderLayer & camBuffer.RenderLayer) == 0 || buffer.Intensity <= 0.0f)
        {
            continue;
        }

        ICARIAN_ASSERT(buffer.Data != nullptr);
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

            const uint32_t splitCount = lightData.GetSplitCount();
            const LightShadowSplit* splits = lightData.GetSplits();
            ICARIAN_ASSERT(splitCount > 0);
            ICARIAN_ASSERT(splits != nullptr);

            commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
            IDEFER(commandBuffer.endRenderPass());

            commandBuffer.setScissor(0, 1, &scissor);
            commandBuffer.setViewport(0, 1, &viewport);
            
            DrawShadow(splits[0].LVP, splits[0].Split, buffer.ShadowBias, buffer.RenderLayer, lightRenderTexture, false, commandBuffer, a_frameIndex);

            {
                PROFILESTACK("Post Shadow");

                m_postShadowFunc->Exec(shadowArgs);
            }
        }
    }

    renderCommand.Flush();

    Profiler::StopFrame();

    return vCmdBuffer;
}
VulkanCommandBuffer VulkanGraphicsEngine::PointShadowPass(uint32_t a_camIndex, uint32_t a_bufferIndex, uint32_t a_frameIndex)
{
    VulkanCommandBuffer vCmdBuffer = VulkanCommandBuffer(nullptr, VulkanCommandBufferType_Graphics);

    // ASAN hamstrings rendering performance have seen 2-4x performance improvements on linux with it disabled so make sure using release and not releasewithdebug for final build...
    // Probably want to do more effective culling and caching for point light shadow rendering as well

    // May switch this to omnidirectional image on 2d texture down the line not sure need more research
    // Also may want to do multi pass culling for point lights
    // This assumes that down the line I dont just switch everything to a compute shader with compute driven rendering
    // This is a bit janky and slow but it works for now
    Profiler::Start("Point Shadow Pass");
    IDEFER(Profiler::Stop());

    Profiler::StartFrame("Update");
    
    if (m_pointLights.Size() == 0)
    {
        Profiler::StopFrame();

        return vCmdBuffer;
    }

    const CameraBuffer& camBuffer = m_cameraBuffers[a_camIndex];

    const vk::CommandBuffer commandBuffer = StartCommandBuffer(a_bufferIndex, a_frameIndex);
    IDEFER(commandBuffer.end());
    vCmdBuffer.SetCommandBuffer(commandBuffer);

    VULKAN_MARKER_COL(m_vulkanEngine, commandBuffer, "Point Shadow Pass", 255, 0, 0);

    VulkanRenderCommand& renderCommand = m_renderCommands.Push(VulkanRenderCommand(m_vulkanEngine, this, m_swapchain, commandBuffer, -1, a_bufferIndex));
    VulkanLightData& lightData = m_lightData.Push(VulkanLightData());

    e_LightType lightType = LightType_Point;

    void* shadowSetupArgs[] =
    {
        &lightType,
        &a_camIndex
    };

    {
        PROFILESTACK("Shadow Setup");

        m_shadowSetupFunc->Exec(shadowSetupArgs);
    }

    const Frustum cameraFrustum = camBuffer.ToFrustum((glm::vec2)m_swapchain->GetSize());

    const Array<PointLightBuffer> lights = m_pointLights.ToActiveArray();

    for (const PointLightBuffer& buffer : lights)
    {
        if (buffer.TransformAddr == -1 || (buffer.RenderLayer & camBuffer.RenderLayer) == 0 || buffer.Radius <= 0.0f || buffer.Intensity <= 0.0f)
        {
            continue;
        }

        ICARIAN_ASSERT(buffer.Data != nullptr);
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

            DrawShadow(lvp, buffer.Radius, buffer.ShadowBias, buffer.RenderLayer, renderTextureIndex, true, commandBuffer, a_frameIndex);
        }
    }

    renderCommand.Flush();

    Profiler::StopFrame();

    return vCmdBuffer;
}
VulkanCommandBuffer VulkanGraphicsEngine::SpotShadowPass(uint32_t a_camIndex, uint32_t a_bufferIndex, uint32_t a_frameIndex)
{
    VulkanCommandBuffer vCmdBuffer = VulkanCommandBuffer(nullptr, VulkanCommandBufferType_Graphics);

    Profiler::Start("Spot Shadow Pass");
    IDEFER(Profiler::Stop());

    Profiler::StartFrame("Update");

    if (m_spotLights.Size() == 0)
    {
        Profiler::StopFrame();

        return vCmdBuffer;
    }

    const CameraBuffer& camBuffer = m_cameraBuffers[a_camIndex];

    const vk::CommandBuffer commandBuffer = StartCommandBuffer(a_bufferIndex, a_frameIndex);
    IDEFER(commandBuffer.end());
    vCmdBuffer.SetCommandBuffer(commandBuffer);

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
    }

    const Frustum cameraFrustum = camBuffer.ToFrustum((glm::vec2)m_swapchain->GetSize());

    const Array<SpotLightBuffer> lights = m_spotLights.ToArray();
    const Array<bool> state = m_spotLights.ToStateArray();
    const uint32_t size = lights.Size();

    for (uint32_t i = 0; i < size; ++i)
    {
        if (!state[i])
        {
            continue;
        }

        const SpotLightBuffer& buffer = lights[i];
        IVERIFY(buffer.Data != nullptr);

        if (buffer.TransformAddr == -1 || (buffer.RenderLayer & camBuffer.RenderLayer) == 0 || buffer.Intensity <= 0.0f || buffer.Radius <= 0.0f)
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
        IVERIFY(splitCount > 0);
        IVERIFY(splits != nullptr);

        commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
        IDEFER(commandBuffer.endRenderPass());

        commandBuffer.setScissor(0, 1, &scissor);
        commandBuffer.setViewport(0, 1, &viewport);
        
        DrawShadow(splits[0].LVP, buffer.Radius, buffer.ShadowBias, buffer.RenderLayer, lightRenderTexture, false, commandBuffer, a_frameIndex);

        {
            PROFILESTACK("Post Shadow");

            m_postShadowFunc->Exec(shadowArgs);
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
    const VulkanCommandBuffer vCmdBuffer = VulkanCommandBuffer(commandBuffer, VulkanCommandBufferType_Graphics);

    VULKAN_MARKER_COL(m_vulkanEngine, commandBuffer, "Draw Pass", 0, 255, 0);

    VulkanRenderCommand& renderCommand = m_renderCommands.Push(VulkanRenderCommand(m_vulkanEngine, this, m_swapchain, commandBuffer, a_camIndex, a_bufferIndex));

    void* camArgs[] = 
    { 
        &a_camIndex 
    };

    {
        PROFILESTACK("Pre Render");

        m_preRenderFunc->Exec(camArgs);
    }

    const VulkanRenderTexture* renderTexture = renderCommand.GetRenderTexture();
    glm::vec2 screenSize = (glm::vec2)m_swapchain->GetSize();
    if (renderTexture != nullptr)
    {
        screenSize = glm::vec2(renderTexture->GetWidth(), renderTexture->GetHeight());
    }
    const Frustum frustum = camBuffer.ToFrustum(screenSize);

    Draw(false, camBuffer, frustum, &renderCommand, a_frameIndex);

    {
        PROFILESTACK("Post Render");

        m_postRenderFunc->Exec(camArgs);
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

    VulkanPushPool* pushPool = m_vulkanEngine->GetPushPool();

    const CameraBuffer& camBuffer = m_cameraBuffers[a_camIndex];

    const vk::CommandBuffer commandBuffer = StartCommandBuffer(a_bufferIndex, a_frameIndex);
    IDEFER(commandBuffer.end());
    const VulkanCommandBuffer vCmdBuffer = VulkanCommandBuffer(commandBuffer, VulkanCommandBufferType_Graphics);

    VULKAN_MARKER_COL(m_vulkanEngine, commandBuffer, "Light Pass", 0, 0, 255);

    VulkanRenderCommand& renderCommand = m_renderCommands.Push(VulkanRenderCommand(m_vulkanEngine, this, m_swapchain, commandBuffer, a_camIndex, a_bufferIndex));
    VulkanLightData& lightData = m_lightData.Push(VulkanLightData());

    void* lightSetupArgs[] =
    {
        &a_camIndex
    };

    {
        PROFILESTACK("Light Setup");

        m_lightSetupFunc->Exec(lightSetupArgs);
    }

    const VulkanRenderTexture* renderTexture = renderCommand.GetRenderTexture();

    glm::vec2 screenSize = (glm::vec2)m_swapchain->GetSize();
    if (renderTexture != nullptr)
    {
        screenSize = glm::vec2(renderTexture->GetWidth(), renderTexture->GetHeight());
    }

    const Frustum frustum = camBuffer.ToFrustum(screenSize);

    for (uint32_t i = 0; i < LightType_ShadowEnd; ++i)
    {
        switch ((e_LightType)i)
        {
        case LightType_Directional:
        {
            PROFILESTACK("S Dir Light");

            VULKAN_MARKER(m_vulkanEngine, commandBuffer, "Shadow Directional Light");

            const Array<DirectionalLightBuffer> lights = m_directionalLights.ToArray();
            const Array<bool> state = m_directionalLights.ToStateArray();
            const uint32_t size = lights.Size();

            for (uint32_t j = 0; j < size; ++j)
            {
                if (!state[j])
                {
                    continue;
                }

                const DirectionalLightBuffer& buffer = lights[j];

                const bool isValid = buffer.TransformAddr != -1 && buffer.Intensity > 0.0f;
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
                IVERIFY(splitCount > 0);
                IVERIFY(splits != nullptr);

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
                    renderCommand.PushLight(dirLightInput.Slot, LightType_Directional, j);
                }

                const uint32_t count = lightBuffer->LightRenderTextureCount;

                ShaderBufferInput shadowLightInput;
                if (data->GetShaderBufferInput(ShaderBufferType_SSShadowLightBuffer, &shadowLightInput))
                {
                    renderCommand.PushLightSplits(shadowLightInput.Slot, splits, splitCount);
                }

                ShaderBufferInput shadowTextureInput;
                if (data->GetShaderBufferInput(ShaderBufferType_AShadowTexture2D, &shadowTextureInput))
                {
                    renderCommand.PushShadowTextureArray(shadowTextureInput.Slot, j);
                }

                commandBuffer.draw(4, 1, 0, 0);
            }

            break;
        }
        case LightType_Point:
        {
            PROFILESTACK("S Point Light");

            VULKAN_MARKER(m_vulkanEngine, commandBuffer, "Shadow Point Light");

            const Array<PointLightBuffer> lights = m_pointLights.ToArray();
            const Array<bool> state = m_pointLights.ToStateArray();
            const uint32_t size = lights.Size();

            for (uint32_t j = 0; j < size; ++j)
            {
                if (!state[j])
                {
                    continue;
                }

                const PointLightBuffer& buffer = lights[j];

                const bool isValid = buffer.TransformAddr != -1 && buffer.Radius > 0.0f && buffer.Intensity > 0.0f;
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

                    data->PushUniformBuffer(commandBuffer, pointLightInput.Slot, uniformBuffer, a_frameIndex);
                }

                ShaderBufferInput shadowTextureInput;
                if (data->GetShaderBufferInput(ShaderBufferType_ShadowTextureCube, &shadowTextureInput))
                {
                    const TextureSamplerBuffer buffer =
                    {
                        .Addr = lightBuffer->LightRenderTextures[0],
                        .Slot = 0,
                        .TextureMode = TextureMode_DepthCubeRenderTexture,
                        .FilterMode = TextureFilter_Linear,
                        .AddressMode = TextureAddress_ClampToEdge,
                        .Data = VulkanTextureSampler::GenerateFromBuffer(m_vulkanEngine, this, buffer)
                    };
                    IDEFER(delete (VulkanTextureSampler*)buffer.Data);

                    data->PushTexture(commandBuffer, shadowTextureInput.Slot, buffer, a_frameIndex);
                }
                
                commandBuffer.draw(4, 1, 0, 0);
            }

            break;
        }
        case LightType_Spot:
        {
            VULKAN_MARKER(m_vulkanEngine, commandBuffer, "Shadow Spot Light");

            PROFILESTACK("S Spot Light");

            const Array<SpotLightBuffer> lights = m_spotLights.ToArray();
            const Array<bool> state = m_spotLights.ToStateArray();
            const uint32_t size = lights.Size();

            for (uint32_t j = 0; j < size; ++j)
            {
                if (!state[j])
                {
                    continue;
                }

                const SpotLightBuffer& buffer = lights[j];
                
                const bool isValid = buffer.TransformAddr != -1 && buffer.Radius > 0 && buffer.Intensity > 0;
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
                IVERIFY(splitCount > 0);
                IVERIFY(splits != nullptr);

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

                    data->PushUniformBuffer(commandBuffer, spotLightInput.Slot, uniformBuffer, a_frameIndex);
                }

                ShaderBufferInput shadowTextureInput;
                if (data->GetShaderBufferInput(ShaderBufferType_ShadowTexture2D, &shadowTextureInput))
                {
                    const TextureSamplerBuffer buffer =
                    {
                        .Addr = lightBuffer->LightRenderTextures[0],
                        .Slot = 0,
                        .TextureMode = TextureMode_DepthRenderTexture,
                        .FilterMode = TextureFilter_Linear,
                        .AddressMode = TextureAddress_ClampToEdge,
                        .Data = VulkanTextureSampler::GenerateFromBuffer(m_vulkanEngine, this, buffer)
                    };
                    IDEFER(delete (VulkanTextureSampler*)buffer.Data);

                    data->PushTexture(commandBuffer, shadowTextureInput.Slot, buffer, a_frameIndex);
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

                    data->PushUniformBuffer(commandBuffer, shadowBufferInput.Slot, uniformBuffer, a_frameIndex);
                }

                commandBuffer.draw(4, 1, 0, 0);
            }

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
        ICARIAN_ASSERT(data != nullptr);

        switch ((e_LightType)i)
        {
        case LightType_Ambient:
        {
            PROFILESTACK("Ambient Light");

            VULKAN_MARKER(m_vulkanEngine, commandBuffer, "Ambient Light");

            const Array<AmbientLightBuffer> lights = m_ambientLights.ToActiveArray();
            const uint32_t size = (uint32_t)lights.Size();

            ShaderBufferInput ambientLightInput;
            if (data->GetShaderBufferInput(ShaderBufferType_AmbientLightBuffer, &ambientLightInput))
            {
                for (const AmbientLightBuffer& ambientLight : lights)
                {
                    if (camBuffer.RenderLayer & ambientLight.RenderLayer && ambientLight.Intensity > 0.0f)
                    {
                        VulkanUniformBuffer* uniformBuffer = pushPool->AllocateAmbientLightUniformBuffer();

                        const IcarianCore::ShaderAmbientLightBuffer buffer =
                        {
                            .LightColor = glm::vec4(ambientLight.Color.xyz(), ambientLight.Intensity)
                        };

                        uniformBuffer->SetData(a_frameIndex, &buffer);

                        data->PushUniformBuffer(commandBuffer, ambientLightInput.Slot, uniformBuffer, a_frameIndex);

                        commandBuffer.draw(4, 1, 0, 0);
                    }
                }
            }
            else if (data->GetShaderBufferInput(ShaderBufferType_SSAmbientLightBuffer, &ambientLightInput))
            {
                IcarianCore::ShaderAmbientLightBuffer* buffers = new IcarianCore::ShaderAmbientLightBuffer[size];
                IDEFER(delete[] buffers);
                uint32_t count = 0;

                for (const AmbientLightBuffer& ambientLight : lights)
                {
                    if (camBuffer.RenderLayer & ambientLight.RenderLayer && ambientLight.Intensity > 0.0f)
                    {
                        buffers[count++].LightColor = glm::vec4(ambientLight.Color.xyz(), ambientLight.Intensity);
                    }
                }

                if (count > 0)
                {
                    const VulkanShaderStorageObject* storage = new VulkanShaderStorageObject(m_vulkanEngine, sizeof(IcarianCore::ShaderAmbientLightBuffer) * count, count, buffers);
                    IDEFER(delete storage);

                    data->PushShaderStorageObject(commandBuffer, ambientLightInput.Slot, storage, a_frameIndex);

                    commandBuffer.draw(4, 1, 0, 0);
                }
            }

            break;
        }
        case LightType_Directional:
        {
            PROFILESTACK("Dir Light");

            VULKAN_MARKER(m_vulkanEngine, commandBuffer, "Directional Light");

            const Array<DirectionalLightBuffer> lights = m_directionalLights.ToActiveArray();

            ShaderBufferInput dirLightInput;
            if (data->GetShaderBufferInput(ShaderBufferType_DirectionalLightBuffer, &dirLightInput))
            {
                for (const DirectionalLightBuffer& dirLight : lights)
                {
                    if (dirLight.TransformAddr != -1 && camBuffer.RenderLayer & dirLight.RenderLayer && dirLight.Intensity > 0.0f)
                    {
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
                        
                        data->PushUniformBuffer(commandBuffer, dirLightInput.Slot, uniformBuffer, a_frameIndex);

                        commandBuffer.draw(4, 1, 0, 0);
                    }
                }
            }
            else if (data->GetShaderBufferInput(ShaderBufferType_SSDirectionalLightBuffer, &dirLightInput))
            {
                const uint32_t size = lights.Size();
                IcarianCore::ShaderDirectionalLightBuffer* buffers = new IcarianCore::ShaderDirectionalLightBuffer[size];
                IDEFER(delete[] buffers);
                uint32_t count = 0;

                for (const DirectionalLightBuffer& dirLight : lights)
                {
                    if (dirLight.TransformAddr != -1 && camBuffer.RenderLayer & dirLight.RenderLayer && dirLight.Intensity > 0.0f)
                    {
                        const VulkanLightBuffer* lightData = (VulkanLightBuffer*)dirLight.Data;
                        IVERIFY(lightData != nullptr);

                        const bool isShadowLight = lightData->LightRenderTextureCount > 0;
                        if (isShadowLight)
                        {
                            continue;
                        }

                        const glm::mat4 tMat = ObjectManager::GetGlobalMatrix(dirLight.TransformAddr);
                        const glm::vec3 forward = glm::normalize(tMat[2].xyz());

                        IDEFER(++count);

                        buffers[count].LightDir = glm::vec4(forward, dirLight.Intensity);
                        buffers[count].LightColor = dirLight.Color;
                    }
                }

                if (count > 0)
                {
                    const VulkanShaderStorageObject* storage = new VulkanShaderStorageObject(m_vulkanEngine, sizeof(IcarianCore::ShaderDirectionalLightBuffer) * count, count, buffers);
                    IDEFER(delete storage);

                    data->PushShaderStorageObject(commandBuffer, dirLightInput.Slot, storage, a_frameIndex);

                    commandBuffer.draw(4, 1, 0, 0);
                }
            }
            
            break;
        }
        case LightType_Point:
        {
            PROFILESTACK("Point Light");

            VULKAN_MARKER(m_vulkanEngine, commandBuffer, "Point Light");

            const Array<PointLightBuffer> lights = m_pointLights.ToActiveArray();

            ShaderBufferInput pointLightInput;
            if (data->GetShaderBufferInput(ShaderBufferType_PointLightBuffer, &pointLightInput))
            {
                for (const PointLightBuffer& pointLight : lights)
                {
                    if (pointLight.TransformAddr != -1 && camBuffer.RenderLayer & pointLight.RenderLayer && pointLight.Radius > 0.0f && pointLight.Intensity > 0.0f)
                    {
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

                        data->PushUniformBuffer(commandBuffer, pointLightInput.Slot, uniformBuffer, a_frameIndex);

                        commandBuffer.draw(4, 1, 0, 0);
                    }
                }
            }
            else if (data->GetShaderBufferInput(ShaderBufferType_SSPointLightBuffer, &pointLightInput))
            {
                const uint32_t size = lights.Size();
                IcarianCore::ShaderPointLightBuffer* buffers = new IcarianCore::ShaderPointLightBuffer[size];
                IDEFER(delete[] buffers);
                uint32_t count = 0;

                for (const PointLightBuffer& pointLight : lights)
                {
                    if (pointLight.TransformAddr != -1 && camBuffer.RenderLayer & pointLight.RenderLayer && pointLight.Radius > 0.0f && pointLight.Intensity > 0.0f)
                    {
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
                }

                if (count > 0)
                {
                    const VulkanShaderStorageObject* storage = new VulkanShaderStorageObject(m_vulkanEngine, sizeof(IcarianCore::ShaderPointLightBuffer) * count, count, buffers);
                    IDEFER(delete storage);

                    data->PushShaderStorageObject(commandBuffer, pointLightInput.Slot, storage, a_frameIndex);

                    commandBuffer.draw(4, 1, 0, 0);
                }
            }

            break;
        }
        case LightType_Spot:
        {
            PROFILESTACK("Spot Light");

            VULKAN_MARKER(m_vulkanEngine, commandBuffer, "Spot Light");

            const Array<SpotLightBuffer> lights = m_spotLights.ToActiveArray();

            ShaderBufferInput spotLightInput;
            if (data->GetShaderBufferInput(ShaderBufferType_SpotLightBuffer, &spotLightInput))
            {
                for (const SpotLightBuffer& spotLight : lights)
                {
                    if (spotLight.TransformAddr != -1 && camBuffer.RenderLayer & spotLight.RenderLayer)
                    {
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

                        data->PushUniformBuffer(commandBuffer, spotLightInput.Slot, uniformBuffer, a_frameIndex);

                        commandBuffer.draw(4, 1, 0, 0);
                    }
                }
            }
            else if (data->GetShaderBufferInput(ShaderBufferType_SSSpotLightBuffer, &spotLightInput))
            {
                const uint32_t size = lights.Size();

                IcarianCore::ShaderSpotLightBuffer* buffers = new IcarianCore::ShaderSpotLightBuffer[size];
                IDEFER(delete[] buffers);
                uint32_t count = 0;

                for (const SpotLightBuffer& spotLight : lights)
                {
                    if (spotLight.TransformAddr != -1 && camBuffer.RenderLayer & spotLight.RenderLayer)
                    {
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
                }

                if (count > 0)
                {
                    const VulkanShaderStorageObject* storage = new VulkanShaderStorageObject(m_vulkanEngine, sizeof(IcarianCore::ShaderSpotLightBuffer) * count, count, buffers);
                    IDEFER(delete storage);

                    data->PushShaderStorageObject(commandBuffer, spotLightInput.Slot, storage, a_frameIndex);

                    commandBuffer.draw(4, 1, 0, 0);
                }
            }

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

    const CameraBuffer camBuffer = m_cameraBuffers[a_camIndex];
    
    const vk::CommandBuffer commandBuffer = StartCommandBuffer(a_bufferIndex, a_frameIndex);
    IDEFER(commandBuffer.end());
    const VulkanCommandBuffer vCmdBuffer = VulkanCommandBuffer(commandBuffer, VulkanCommandBufferType_Graphics);

    VULKAN_MARKER_COL(m_vulkanEngine, commandBuffer, "Forward Pass", 0, 255, 255);

    VulkanRenderCommand& renderCommand = m_renderCommands.Push(VulkanRenderCommand(m_vulkanEngine, this, m_swapchain, commandBuffer, a_camIndex, a_bufferIndex));

    void* camArgs[] = 
    { 
        &a_camIndex 
    };

    {
        PROFILESTACK("Pre Forward");

        IVERIFY(m_preForwardFunc != nullptr);
        m_preForwardFunc->Exec(camArgs);
    }

    const VulkanRenderTexture* renderTexture = renderCommand.GetRenderTexture();
    glm::vec2 screenSize = (glm::vec2)m_swapchain->GetSize();
    if (renderTexture != nullptr)
    {
        screenSize = glm::vec2(renderTexture->GetWidth(), renderTexture->GetHeight());
    }
    const Frustum frustum = camBuffer.ToFrustum(screenSize);

    Draw(true, camBuffer, frustum, &renderCommand, a_frameIndex);

    {
        PROFILESTACK("Particles");

        const uint32_t renderTextureAddr = renderCommand.GetRenderTexutreAddr();
        const Array<VulkanGraphicsParticle2D*> particleSystems = m_particleEmitters.ToActiveArray();

        for (VulkanGraphicsParticle2D* pSys : particleSystems)
        {
            IVERIFY(pSys != nullptr);

            pSys->Update(a_frameIndex, a_bufferIndex, camBuffer.RenderLayer, commandBuffer, renderTextureAddr);
        }
    }

    {
        PROFILESTACK("Post Forward");

        IVERIFY(m_postForwardFunc != nullptr);
        m_postForwardFunc->Exec(camArgs);
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
    const VulkanCommandBuffer vCmdBuffer = VulkanCommandBuffer(commandBuffer, VulkanCommandBufferType_Graphics);

    VULKAN_MARKER_COL(m_vulkanEngine, commandBuffer, "Post Pass", 255, 255, 0);

    VulkanRenderCommand& renderCommand = m_renderCommands.Push(VulkanRenderCommand(m_vulkanEngine, this, m_swapchain, commandBuffer, a_camIndex, a_bufferIndex));

    void* camArgs[] = 
    { 
        &a_camIndex 
    };

    m_postProcessFunc->Exec(camArgs);

    renderCommand.Flush();

    Profiler::StopFrame();

    return vCmdBuffer;
}

void VulkanGraphicsEngine::DrawUIElement(vk::CommandBuffer a_commandBuffer, uint32_t a_addr, const CanvasBuffer& a_canvas, const glm::vec2& a_screenSize, uint32_t a_index)
{
    const bool valid = a_addr != -1;
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

    element->Update(m_vulkanEngine->GetRenderEngine());

    VulkanPipeline* pipeline = nullptr;
    VulkanShaderData* shaderData = nullptr;

    switch (element->GetType()) 
    {
    case UIElementType_Base:
    {
        break;
    }
    case UIElementType_Text:
    {
        const TextUIElement* text = (TextUIElement*)element;

        if (text->IsValid())
        {
            const vk::Rect2D scissor = vk::Rect2D({ (int32_t)screenPos.x, (int32_t)screenPos.y }, { (uint32_t)screenSize.x, (uint32_t)screenSize.y });
            a_commandBuffer.setScissor(0, 1, &scissor);
            const vk::Viewport viewport = vk::Viewport(screenPos.x, screenPos.y, screenSize.x, screenSize.y, 0.0f, 1.0f);
            a_commandBuffer.setViewport(0, 1, &viewport);

            pipeline = GetPipeline(-1, m_textUIPipelineAddr);
            IVERIFY(pipeline != nullptr);
            shaderData = pipeline->GetShaderData();
            IVERIFY(shaderData != nullptr);

            const uint32_t samplerAddr = text->GetSamplerAddr();
            const TextureSamplerBuffer& sampler = GetTextureSampler(samplerAddr);

            shaderData->PushTexture(a_commandBuffer, 0, sampler, a_index);
        }
            
        break;
    }
    case UIElementType_Image:
    {
        const ImageUIElement* image = (ImageUIElement*)element;

        const vk::Rect2D scissor = vk::Rect2D({ (int32_t)screenPos.x, (int32_t)screenPos.y }, { (uint32_t)screenSize.x, (uint32_t)screenSize.y });
        a_commandBuffer.setScissor(0, 1, &scissor);
        const vk::Viewport viewport = vk::Viewport(screenPos.x, screenPos.y, screenSize.x, screenSize.y, 0.0f, 1.0f);
        a_commandBuffer.setViewport(0, 1, &viewport);

        pipeline = GetPipeline(-1, m_imageUIPipelineAddr);
        IVERIFY(pipeline != nullptr);
        shaderData = pipeline->GetShaderData();
        IVERIFY(shaderData != nullptr);
        
        const uint32_t samplerAddr = image->GetSamplerAddr();
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

Array<VulkanCommandBuffer> VulkanGraphicsEngine::Update(double a_delta, double a_time, uint32_t a_index)
{
    // TODO: Prebuild camera uniform buffers
    Profiler::StartFrame("Drawing Setup");
    m_renderCommands.Clear();

    {
        const Array<bool> state = m_shaderPrograms.ToStateArray();
        TLockArray<RenderProgram> a = m_shaderPrograms.ToLockArray();
        const uint32_t size = state.Size();

        for (uint32_t i = 0; i < size; ++i)
        {
            if (!state[i])
            {
                continue;
            }

            RenderProgram& program = a[i];
            IVERIFY(program.Data != nullptr);

            VulkanShaderData* shaderData = (VulkanShaderData*)program.Data;
            shaderData->Update(a_index, program);
        }

        const IcarianCore::ShaderTimeBuffer timeBuffer =
        {
            .Time = glm::vec2((float)a_delta, (float)a_time)
        };

        m_timeUniform->SetData(a_index, &timeBuffer);
    }

    const vk::Device device = m_vulkanEngine->GetLogicalDevice();

    const uint32_t camBufferSize = m_cameraBuffers.Size();
    Array<uint32_t> camIndices;
    for (uint32_t i = 0; i < camBufferSize; ++i)
    {
        if (m_cameraBuffers[i].TransformAddr != -1)
        {
            camIndices.Push(i);
        }
    }

    TReadLockArray<CanvasRendererBuffer> canvasBuffer = m_canvasRenderers.ToReadLockArray();
    const uint32_t canvasCount = canvasBuffer.Size();
    const uint32_t camIndexSize = camIndices.Size();
    const uint32_t totalPoolSize = camIndexSize * DrawingPassCount + canvasCount;
    const uint32_t poolSize = (uint32_t)m_commandPool[a_index].Size();

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
            
            m_commandPool[a_index].Push(pool);

            const vk::CommandBufferAllocateInfo commandBufferInfo = vk::CommandBufferAllocateInfo
            (
                pool,
                vk::CommandBufferLevel::ePrimary,
                1
            );

            vk::CommandBuffer buffer;
            VKRESERRMSG(device.allocateCommandBuffers(&commandBufferInfo, &buffer), "Failed to allocate graphics command buffer");

            m_commandBuffers[a_index].Push(buffer);
        }
    }

    const uint32_t camUniformSize = m_cameraUniforms.Size();

    if (camUniformSize < totalPoolSize)
    {
        TRACE("Allocating camera ubos");
        const uint32_t diff = totalPoolSize - camUniformSize;
        for (uint32_t i = 0; i < diff; ++i)
        {
            m_cameraUniforms.Push(new VulkanUniformBuffer(m_vulkanEngine, sizeof(IcarianCore::ShaderCameraBuffer)));
        }
    }

    for (uint32_t i = 0; i < poolSize; ++i)
    {
        device.resetCommandPool(m_commandPool[a_index][i]);
    }

    Profiler::StopFrame();

    PROFILESTACK("Drawing Cmd");

    std::vector<std::future<VulkanCommandBuffer>> futures;
    for (uint32_t i = 0; i < camIndexSize; ++i)
    {
        const uint32_t camIndex = camIndices[i];
        const uint32_t poolIndex = i * DrawingPassCount;

        FThreadJob<VulkanCommandBuffer, DrawCallBind>* dirShadowJob = new FThreadJob<VulkanCommandBuffer, DrawCallBind>
        (
            DrawCallBind(this, camIndex, poolIndex + 0, a_index, &VulkanGraphicsEngine::DirectionalShadowPass),
            JobPriority_EngineUrgent
        );
        futures.emplace_back(dirShadowJob->GetFuture());
        ThreadPool::PushJob(dirShadowJob);

        FThreadJob<VulkanCommandBuffer, DrawCallBind>* pointShadowJob = new FThreadJob<VulkanCommandBuffer, DrawCallBind>
        (
            DrawCallBind(this, camIndex, poolIndex + 1, a_index, &VulkanGraphicsEngine::PointShadowPass),
            JobPriority_EngineUrgent
        );
        futures.emplace_back(pointShadowJob->GetFuture());
        ThreadPool::PushJob(pointShadowJob);

        FThreadJob<VulkanCommandBuffer, DrawCallBind>* spotShadowJob = new FThreadJob<VulkanCommandBuffer, DrawCallBind>
        (
            DrawCallBind(this, camIndex, poolIndex + 2, a_index, &VulkanGraphicsEngine::SpotShadowPass),
            JobPriority_EngineUrgent
        );
        futures.emplace_back(spotShadowJob->GetFuture());
        ThreadPool::PushJob(spotShadowJob);

        FThreadJob<VulkanCommandBuffer, DrawCallBind>* drawJob = new FThreadJob<VulkanCommandBuffer, DrawCallBind>
        (   
            DrawCallBind(this, camIndex, poolIndex + 3, a_index, &VulkanGraphicsEngine::DrawPass), 
            JobPriority_EngineUrgent
        );
        futures.emplace_back(drawJob->GetFuture());
        ThreadPool::PushJob(drawJob);

        FThreadJob<VulkanCommandBuffer, DrawCallBind>* lightJob = new FThreadJob<VulkanCommandBuffer, DrawCallBind>
        (
            DrawCallBind(this, camIndex, poolIndex + 4, a_index, &VulkanGraphicsEngine::LightPass),
            JobPriority_EngineUrgent
        );
        futures.emplace_back(lightJob->GetFuture());
        ThreadPool::PushJob(lightJob);

        FThreadJob<VulkanCommandBuffer, DrawCallBind>* forwardJob = new FThreadJob<VulkanCommandBuffer, DrawCallBind>
        (
            DrawCallBind(this, camIndex, poolIndex + 5, a_index, &VulkanGraphicsEngine::ForwardPass),
            JobPriority_EngineUrgent
        );
        futures.emplace_back(forwardJob->GetFuture());
        ThreadPool::PushJob(forwardJob);

        FThreadJob<VulkanCommandBuffer, DrawCallBind>* postJob = new FThreadJob<VulkanCommandBuffer, DrawCallBind>
        (
            DrawCallBind(this, camIndex, poolIndex + 6, a_index, &VulkanGraphicsEngine::PostPass),
            JobPriority_EngineUrgent
        );
        futures.emplace_back(postJob->GetFuture());
        ThreadPool::PushJob(postJob);
    }
    
    Array<VulkanCommandBuffer> cmdBuffers;
    {
        PROFILESTACK("Video Decode");

        const Array<VulkanVideoTexture*> videoTextures = m_videoTextures.ToActiveArray();
        if (!videoTextures.Empty())
        {
            if (m_vulkanEngine->IsExtensionEnabled(VK_KHR_VIDEO_DECODE_H264_EXTENSION_NAME))
            {   
                device.resetCommandPool(m_decodePool[a_index]);
                
                const vk::CommandBuffer commandBuffer = m_decodeBuffer[a_index];

                constexpr vk::CommandBufferBeginInfo BeginInfo;
                commandBuffer.begin(BeginInfo);
                IDEFER(commandBuffer.end());

                for (VulkanVideoTexture* tex : videoTextures)
                {
                    tex->UpdateVulkan(commandBuffer, a_delta);
                }

                const VulkanCommandBuffer buffer = VulkanCommandBuffer(commandBuffer, VulkanCommandBufferType_VideoDecode);
                cmdBuffers.Push(buffer);
            }
            else 
            {
                IERROR("Software decoding not supported");
            }
        }        
    }

    Array<vk::CommandBuffer> uiBuffers;
    {
        PROFILESTACK("UI Draw");

        const Array<CanvasRendererBuffer> a = m_canvasRenderers.ToActiveArray();

        const uint32_t canvasArrSize = a.Size();
        for (uint32_t i = 0; i < canvasArrSize; ++i)
        {
            const CanvasRendererBuffer& canvasRenderer = a[i];

            const bool valid = canvasRenderer.CanvasAddr != -1;
            if (!valid)
            {
                continue;
            }

            const CanvasBuffer& canvas = UIControl::GetCanvas(canvasRenderer.CanvasAddr);

            vk::CommandBuffer buffer = StartCommandBuffer(camIndexSize * DrawingPassCount + i, a_index);
            IDEFER(buffer.end());

            VULKAN_MARKER_COL(m_vulkanEngine, buffer, "UI Pass", 255, 255, 255);

            const VulkanRenderTexture* renderTexture = GetRenderTexture(canvasRenderer.RenderTextureAddr);

            glm::ivec2 screenSize;
            if (renderTexture != nullptr)
            {
                screenSize = glm::ivec2((int)renderTexture->GetWidth(), (int)renderTexture->GetHeight());
                const vk::RenderPassBeginInfo renderPassInto = vk::RenderPassBeginInfo
                (
                    renderTexture->GetRenderPassNoClear(),
                    renderTexture->GetFramebuffer(),
                    vk::Rect2D({ 0, 0 }, { (uint32_t)screenSize.x, (uint32_t)screenSize.y }),
                    renderTexture->GetTotalTextureCount(),
                    renderTexture->GetClearValues()
                );

                buffer.beginRenderPass(renderPassInto, vk::SubpassContents::eInline);
            }
            else
            {
                screenSize = m_swapchain->GetSize();
                constexpr vk::ClearValue ClearColor = vk::ClearValue();
                const vk::RenderPassBeginInfo renderPassInfo = vk::RenderPassBeginInfo
                (
                    m_swapchain->GetRenderPassNoClear(),
                    m_swapchain->GetFramebuffer(m_vulkanEngine->GetImageIndex()),
                    vk::Rect2D({ 0, 0 }, { (uint32_t)screenSize.x, (uint32_t)screenSize.y }),
                    1,
                    &ClearColor
                );

                buffer.beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);
            }

            for (uint32_t i = 0; i < canvas.ChildCount; ++i)
            {
                DrawUIElement(buffer, canvas.ChildElements[i], canvas, screenSize, a_index);
            }

            buffer.endRenderPass();

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

                const VulkanCommandBuffer vBuffer = VulkanCommandBuffer(buffer, VulkanCommandBufferType_Graphics);

                cmdBuffers.Push(vBuffer);
            }
        }
    }

    return cmdBuffers;
}

VulkanVertexShader* VulkanGraphicsEngine::GetVertexShader(uint32_t a_addr)
{
    ICARIAN_ASSERT_MSG(a_addr < m_vertexShaders.Size(), "GetVertexShader out of bounds");
    ICARIAN_ASSERT_MSG(m_vertexShaders.Exists(a_addr), "GetVertexShader already destroyed");

    return m_vertexShaders[a_addr];
}
VulkanPixelShader* VulkanGraphicsEngine::GetPixelShader(uint32_t a_addr)
{
    ICARIAN_ASSERT_MSG(a_addr < m_pixelShaders.Size(), "GetPixelShader out of bounds");
    ICARIAN_ASSERT_MSG(m_pixelShaders.Exists(a_addr), "GetPixelShader already destroyed");

    return m_pixelShaders[a_addr];
}

CameraBuffer VulkanGraphicsEngine::GetCameraBuffer(uint32_t a_addr)
{
    ICARIAN_ASSERT_MSG(a_addr < m_cameraBuffers.Size(), "GetCameraBuffer out of bounds");

    return m_cameraBuffers[a_addr];
}

uint32_t VulkanGraphicsEngine::GenerateModel(const void* a_vertices, uint32_t a_vertexCount, uint16_t a_vertexStride, const uint32_t* a_indices, uint32_t a_indexCount, float a_radius)
{
    IVERIFY(a_vertices != nullptr);
    IVERIFY(a_vertexCount > 0);
    IVERIFY(a_indices != nullptr);
    IVERIFY(a_indexCount > 0);
    IVERIFY(a_vertexStride > 0);

    VulkanModel* model = new VulkanModel(m_vulkanEngine, a_vertexCount, a_vertices, a_vertexStride, a_indexCount, a_indices, a_radius);

    return m_models.PushVal(model);
}
void VulkanGraphicsEngine::DestroyModel(uint32_t a_addr)
{
    if (ISRENDERASSETSTOREADDR(a_addr))
    {
        const uint32_t addr = FROMRENDERSTOREADDR(a_addr);

        const RenderEngine* renderEngine = m_vulkanEngine->GetRenderEngine();
        RenderAssetStore* store = renderEngine->GetRenderAssetStore();

        store->DestroyModel(addr);
    }
    else
    {
        IVERIFY(a_addr < m_models.Size());
        IVERIFY(m_models.Exists(a_addr));

        const VulkanModel* model = m_models[a_addr];
        IDEFER(delete model);

        m_models.Erase(a_addr);
    }
}
VulkanModel* VulkanGraphicsEngine::GetModel(uint32_t a_addr)
{
    if (a_addr == -1)
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
        ICARIAN_ASSERT_MSG(a_addr < m_textures.Size(), "DestroyTexture Texture out of bounds");
        ICARIAN_ASSERT_MSG(m_textures.Exists(a_addr), "DestroyTexture already destroyed");

        const VulkanTexture* texture = m_textures[a_addr];
        IDEFER(delete texture);

        m_textures.Erase(a_addr);
    }
}
VulkanTexture* VulkanGraphicsEngine::GetTexture(uint32_t a_addr)
{
    if (a_addr == -1)
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

    ICARIAN_ASSERT_MSG(finalAddr < m_textures.Size(), "GetTexture out of bounds");
    ICARIAN_ASSERT_MSG(m_textures.Exists(finalAddr), "GetTexture already destroyed");

    return m_textures[finalAddr];
}

uint32_t VulkanGraphicsEngine::GenerateDepthRenderTexture(uint32_t a_width, uint32_t a_height)
{
    IVERIFY(a_width > 0);
    IVERIFY(a_height > 0);

    VulkanDepthRenderTexture* texture = new VulkanDepthRenderTexture(m_vulkanEngine, a_width, a_height);

    return m_depthRenderTextures.PushVal(texture);
}
void VulkanGraphicsEngine::DestroyDepthRenderTexture(uint32_t a_addr)
{
    IVERIFY(a_addr < m_depthRenderTextures.Size());
    IVERIFY(m_depthRenderTextures.Exists(a_addr));

    const VulkanDepthRenderTexture* tex = m_depthRenderTextures[a_addr];
    IDEFER(delete tex);

    m_depthRenderTextures.Erase(a_addr);
}

VulkanRenderTexture* VulkanGraphicsEngine::GetRenderTexture(uint32_t a_addr)
{
    if (a_addr == -1)
    {
        return nullptr;
    }

    IVERIFY(a_addr < m_renderTextures.Size());
    IVERIFY(m_renderTextures.Exists(a_addr));

    return m_renderTextures[a_addr];
}
VulkanDepthRenderTexture* VulkanGraphicsEngine::GetDepthRenderTexture(uint32_t a_addr)
{
    if (a_addr == -1)
    {
        return nullptr;
    }

    IVERIFY(a_addr < m_depthRenderTextures.Size());
    IVERIFY(m_depthRenderTextures.Exists(a_addr));

    return m_depthRenderTextures[a_addr];
}
VulkanDepthCubeRenderTexture* VulkanGraphicsEngine::GetDepthCubeRenderTexture(uint32_t a_addr)
{
    if (a_addr == -1)
    {
        return nullptr;
    }

    IVERIFY(a_addr < m_depthCubeRenderTextures.Size());
    IVERIFY(m_depthCubeRenderTextures.Exists(a_addr));

    return m_depthCubeRenderTextures[a_addr];
}

AmbientLightBuffer VulkanGraphicsEngine::GetAmbientLight(uint32_t a_addr)
{
    IVERIFY(a_addr < m_ambientLights.Size());
    IVERIFY(m_ambientLights.Exists(a_addr));

    return m_ambientLights[a_addr];
}
DirectionalLightBuffer VulkanGraphicsEngine::GetDirectionalLight(uint32_t a_addr)
{
    IVERIFY(a_addr < m_directionalLights.Size());
    IVERIFY(m_directionalLights.Exists(a_addr));

    return m_directionalLights[a_addr];
}
PointLightBuffer VulkanGraphicsEngine::GetPointLight(uint32_t a_addr)
{
    IVERIFY(a_addr < m_pointLights.Size());
    IVERIFY(m_pointLights.Exists(a_addr));

    return m_pointLights[a_addr];
}
SpotLightBuffer VulkanGraphicsEngine::GetSpotLight(uint32_t a_addr)
{
    IVERIFY(a_addr < m_spotLights.Size());
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
            IVERIFY(a_textureAddr < m_textures.Size());
            IVERIFY(m_textures[a_textureAddr] != nullptr);
        }

        break;
    }
    case TextureMode_RenderTexture:
    {
        IVERIFY(a_textureAddr < m_renderTextures.Size());
        IVERIFY(m_renderTextures[a_textureAddr] != nullptr);
        IVERIFY(a_slot < m_renderTextures[a_textureAddr]->GetTextureCount());

        break;
    }
    case TextureMode_RenderTextureDepth:
    {
        IVERIFY(a_textureAddr < m_renderTextures.Size());
        IVERIFY(m_renderTextures[a_textureAddr] != nullptr);

        break;
    }
    case TextureMode_DepthRenderTexture:
    {
        IVERIFY(a_textureAddr < m_depthRenderTextures.Size());
        IVERIFY(m_depthRenderTextures[a_textureAddr] != nullptr);

        break;
    }
    default:
    {
        IERROR("GenerateTextureSampler Invalid Texture Mode");

        break;
    }
    }
    
    TextureSamplerBuffer sampler =
    {
        .Addr = a_textureAddr,
        .Slot = a_slot,
        .TextureMode = a_textureMode,
        .FilterMode = a_filterMode,
        .AddressMode = a_addressMode,
    };
    sampler.Data = VulkanTextureSampler::GenerateFromBuffer(m_vulkanEngine, this, sampler);

    return m_textureSampler.PushVal(sampler);
}
void VulkanGraphicsEngine::DestroyTextureSampler(uint32_t a_addr) 
{
    IVERIFY(a_addr < m_textureSampler.Size());
    IVERIFY(m_textureSampler.Exists(a_addr));

    const TextureSamplerBuffer sampler = m_textureSampler[a_addr];
    IDEFER(
    if (sampler.Data != nullptr) 
    { 
        delete (VulkanTextureSampler*)sampler.Data; 
    });

    m_textureSampler.Erase(a_addr);
}
TextureSamplerBuffer VulkanGraphicsEngine::GetTextureSampler(uint32_t a_addr)
{
    IVERIFY(a_addr < m_textureSampler.Size());
    IVERIFY(m_textureSampler.Exists(a_addr));

    return m_textureSampler[a_addr];
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