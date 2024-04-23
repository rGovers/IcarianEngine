#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/VulkanGraphicsEngine.h"

#include <future>
#include <mutex>
#include <vulkan/vulkan_handles.hpp>

#include "Core/IcarianAssert.h"
#include "Core/IcarianDefer.h"
#include "Logger.h"
#include "ObjectManager.h"
#include "Profiler.h"
#include "Rendering/AnimationController.h"
#include "Rendering/RenderAssetStore.h"
#include "Rendering/RenderEngine.h"
#include "Rendering/ShaderBuffers.h"
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
    m_postProcessFunc = RuntimeManager::GetFunction("IcarianEngine.Rendering", "RenderPipeline", ":PostProcessS(uint)"); 

    RenderProgram textProgram;
    memset(&textProgram, 0, sizeof(RenderProgram));
    textProgram.VertexShader = GenerateFVertexShader(UIVertexShader);
    textProgram.PixelShader = GenerateFPixelShader(UITextPixelShader);
    textProgram.ShadowVertexShader = -1;
    textProgram.ShaderBufferInputCount = 2;
    textProgram.ShaderBufferInputs = new ShaderBufferInput[2];
    textProgram.ShaderBufferInputs[0].Slot = -1;
    textProgram.ShaderBufferInputs[0].BufferType = ShaderBufferType_PUIBuffer;
    textProgram.ShaderBufferInputs[0].ShaderSlot = ShaderSlot_Pixel;
    textProgram.ShaderBufferInputs[0].Set = -1;
    textProgram.ShaderBufferInputs[1].Slot = 0;
    textProgram.ShaderBufferInputs[1].BufferType = ShaderBufferType_PushTexture;
    textProgram.ShaderBufferInputs[1].ShaderSlot = ShaderSlot_Pixel;
    textProgram.ShaderBufferInputs[1].Set = 0;
    textProgram.ColorBlendMode = MaterialBlendMode_Alpha;
    textProgram.CullingMode = CullMode_None;
    textProgram.PrimitiveMode = PrimitiveMode_TriangleStrip;
    textProgram.Flags |= 0b1 << RenderProgram::DestroyFlag;

    m_textUIPipelineAddr = GenerateRenderProgram(textProgram);

    RenderProgram imageProgram;
    memset(&imageProgram, 0, sizeof(RenderProgram));
    imageProgram.VertexShader = GenerateFVertexShader(UIVertexShader);
    imageProgram.PixelShader = GenerateFPixelShader(UIImagePixelShader);
    imageProgram.ShadowVertexShader = -1;
    imageProgram.ShaderBufferInputCount = 2;
    imageProgram.ShaderBufferInputs = new ShaderBufferInput[2];
    imageProgram.ShaderBufferInputs[0].Slot = -1;
    imageProgram.ShaderBufferInputs[0].BufferType = ShaderBufferType_PUIBuffer;
    imageProgram.ShaderBufferInputs[0].ShaderSlot = ShaderSlot_Pixel;
    imageProgram.ShaderBufferInputs[0].Set = -1;
    imageProgram.ShaderBufferInputs[1].Slot = 0;
    imageProgram.ShaderBufferInputs[1].BufferType = ShaderBufferType_PushTexture;
    imageProgram.ShaderBufferInputs[1].ShaderSlot = ShaderSlot_Pixel;
    imageProgram.ShaderBufferInputs[1].Set = 0;
    imageProgram.ColorBlendMode = MaterialBlendMode_Alpha;
    imageProgram.CullingMode = CullMode_None;
    imageProgram.PrimitiveMode = PrimitiveMode_TriangleStrip;
    imageProgram.Flags |= 0b1 << RenderProgram::DestroyFlag;

    m_imageUIPipelineAddr = GenerateRenderProgram(imageProgram);

    m_timeUniform = new VulkanUniformBuffer(m_vulkanEngine, sizeof(TimeShaderBuffer));
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
    delete m_postProcessFunc;
}

void VulkanGraphicsEngine::Cleanup()
{
    delete m_timeUniform;

    const RenderProgram textProgram = m_shaderPrograms[m_textUIPipelineAddr];
    IDEFER(
    {
        if (textProgram.VertexAttributes != nullptr)
        {
            delete[] textProgram.VertexAttributes;
        }

        if (textProgram.ShaderBufferInputs != nullptr)
        {
            delete[] textProgram.ShaderBufferInputs;
        }
    });

    const RenderProgram imageProgram = m_shaderPrograms[m_imageUIPipelineAddr];
    IDEFER(
    {
        if (imageProgram.VertexAttributes != nullptr)
        {
            delete[] imageProgram.VertexAttributes;
        }

        if (imageProgram.ShaderBufferInputs != nullptr)
        {
            delete[] imageProgram.ShaderBufferInputs;
        }
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
}

uint32_t VulkanGraphicsEngine::GenerateGLSLVertexShader(const std::string_view& a_source)
{
    ICARIAN_ASSERT_MSG(!a_source.empty(), "GenerateGLSLVertexShader source is empty");

    VulkanVertexShader* shader = VulkanVertexShader::CreateFromGLSL(m_vulkanEngine, a_source);

    return m_vertexShaders.PushVal(shader);
}
uint32_t VulkanGraphicsEngine::GenerateFVertexShader(const std::string_view& a_source)
{
    ICARIAN_ASSERT_MSG(!a_source.empty(), "GenerateFVertexShader source is empty");

    VulkanVertexShader* shader = VulkanVertexShader::CreateFromFShader(m_vulkanEngine, a_source);

    return m_vertexShaders.PushVal(shader);
}
void VulkanGraphicsEngine::DestroyVertexShader(uint32_t a_addr)
{
    ICARIAN_ASSERT_MSG(a_addr < m_vertexShaders.Size(), "DestroyVertexShader out of bounds");
    ICARIAN_ASSERT_MSG(m_vertexShaders.Exists(a_addr), "DestroyVertexShader already destroyed");

    const VulkanVertexShader* shader = m_vertexShaders[a_addr];
    IDEFER(delete shader);

    m_vertexShaders.Erase(a_addr);
}

uint32_t VulkanGraphicsEngine::GenerateFPixelShader(const std::string_view& a_source)
{
    ICARIAN_ASSERT_MSG(!a_source.empty(), "GenerateFPixelShader source is empty");

    VulkanPixelShader* shader = VulkanPixelShader::CreateFromFShader(m_vulkanEngine, a_source);

    return m_pixelShaders.PushVal(shader);
}
uint32_t VulkanGraphicsEngine::GenerateGLSLPixelShader(const std::string_view& a_source)
{
    ICARIAN_ASSERT_MSG(!a_source.empty(), "GenerateGLSLPixelShader source is empty");

    VulkanPixelShader* shader = VulkanPixelShader::CreateFromGLSL(m_vulkanEngine, a_source);

    return m_pixelShaders.PushVal(shader);
}
void VulkanGraphicsEngine::DestroyPixelShader(uint32_t a_addr)
{
    ICARIAN_ASSERT_MSG(a_addr < m_pixelShaders.Size(), "DestroyPixelShader out of bounds");
    ICARIAN_ASSERT_MSG(m_pixelShaders.Exists(a_addr), "DestroyPixelShader already destroyed");

    const VulkanPixelShader* shader = m_pixelShaders[a_addr];
    IDEFER(delete shader);

    m_pixelShaders.Erase(a_addr);
}

uint32_t VulkanGraphicsEngine::GenerateRenderProgram(const RenderProgram& a_program)
{
    ICARIAN_ASSERT_MSG(a_program.VertexShader < m_vertexShaders.Size(), "GenerateRenderProgram vertex shader out of bounds");
    ICARIAN_ASSERT_MSG(a_program.PixelShader < m_pixelShaders.Size(), "GenerateRenderProgram pixel shader out of bounds");

    TRACE("Creating Shader Program");
    RenderProgram p = a_program;
    p.Data = new VulkanShaderData(m_vulkanEngine, this, a_program);

    return m_shaderPrograms.PushVal(p);
}
void VulkanGraphicsEngine::DestroyRenderProgram(uint32_t a_addr)
{
    ICARIAN_ASSERT_MSG(a_addr < m_shaderPrograms.Size(), "DestroyRenderProgram out of bounds");
    ICARIAN_ASSERT_MSG(m_shaderPrograms.Exists(a_addr), "DestroyRenderProgram shader program does not exist");

    TRACE("Destroying Shader Program");

    const RenderProgram program = m_shaderPrograms[a_addr];
    IDEFER(
    {
        if (program.Data != nullptr)
        {
            delete (VulkanShaderData*)program.Data;
        }

        if (program.UBOData != NULL)
        {
            free(program.UBOData);
        }

        if (program.Flags & 0b1 << RenderProgram::DestroyFlag)
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
        const std::unique_lock g = std::unique_lock(m_pipeLock);
        
        std::vector<uint64_t> keys;
        for (auto iter = m_pipelines.begin(); iter != m_pipelines.end(); ++iter)
        {
            const uint32_t val = (uint32_t)(iter->first >> 32);
            if (val == a_addr)
            {
                keys.emplace_back(iter->first);
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
            const std::unique_lock g = std::unique_lock(m_shadowPipeLock);

            std::vector<uint64_t> keys;
            for (auto iter = m_shadowPipelines.begin(); iter != m_shadowPipelines.end(); ++iter)
            {
                const uint32_t val = (uint32_t)(iter->first >> 32);
                if (val == a_addr)
                {
                    keys.emplace_back(iter->first);
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
            const std::unique_lock g = std::unique_lock(m_cubeShadowPipeLock);

            std::vector<uint64_t> keys;
            for (auto iter = m_cubeShadowPipelines.begin(); iter != m_cubeShadowPipelines.end(); ++iter)
            {
                const uint32_t val = (uint32_t)(iter->first >> 32);
                if (val == a_addr)
                {
                    keys.emplace_back(iter->first);
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
    ICARIAN_ASSERT_MSG(a_addr < m_shaderPrograms.Size(), "GetRenderProgram out of bounds");
    ICARIAN_ASSERT_MSG(m_shaderPrograms.Exists(a_addr), "GetRenderProgram shader program does not exist");

    return m_shaderPrograms[a_addr];
}

VulkanPipeline* VulkanGraphicsEngine::GetShadowPipeline(uint32_t a_renderTexture, uint32_t a_pipeline)
{
    ICARIAN_ASSERT_MSG(a_pipeline < m_shaderPrograms.Size(), "GetShadowPipeline pipeline out of bounds");
    ICARIAN_ASSERT_MSG(m_shaderPrograms.Exists(a_pipeline), "GetShadowPipeline shader program does not exist");

    const uint64_t addr = (uint64_t)a_renderTexture | (uint64_t)a_pipeline << 32;

    const std::unique_lock g = std::unique_lock(m_shadowPipeLock);
    const auto iter = m_shadowPipelines.find(addr);
    if (iter != m_shadowPipelines.end())
    {
        return iter->second;
    }

    TRACE("Allocating Vulkan Shadow Pipeline");
    const VulkanDepthRenderTexture* tex = GetDepthRenderTexture(a_renderTexture);

    ICARIAN_ASSERT_MSG(tex != nullptr, "GetShadowPipeline render texture is null");

    const vk::RenderPass pass = tex->GetRenderPass();

    VulkanPipeline* pipeline = VulkanPipeline::CreateShadowPipeline(m_vulkanEngine, this, pass, a_pipeline);

    m_shadowPipelines.emplace(addr, pipeline);

    return pipeline;
}
VulkanPipeline* VulkanGraphicsEngine::GetCubeShadowPipeline(uint32_t a_renderTexture, uint32_t a_pipeline)
{
    ICARIAN_ASSERT_MSG(a_pipeline < m_shaderPrograms.Size(), "GetCubeShadowPipeline pipeline out of bounds");
    ICARIAN_ASSERT_MSG(m_shaderPrograms.Exists(a_pipeline), "GetCubeShadowPipeline shader program does not exist");

    const uint64_t addr = (uint64_t)a_renderTexture | (uint64_t)a_pipeline << 32;

    const std::unique_lock g = std::unique_lock(m_cubeShadowPipeLock);
    const auto iter = m_cubeShadowPipelines.find(addr);
    if (iter != m_cubeShadowPipelines.end())
    {
        return iter->second;
    }

    TRACE("Allocating Vulkan Cube Shadow Pipeline");
    const VulkanDepthCubeRenderTexture* tex = GetDepthCubeRenderTexture(a_renderTexture);

    ICARIAN_ASSERT_MSG(tex != nullptr, "GetCubeShadowPipeline render texture is null");

    const vk::RenderPass pass = tex->GetRenderPass();

    VulkanPipeline* pipeline = VulkanPipeline::CreateShadowPipeline(m_vulkanEngine, this, pass, a_pipeline);

    m_cubeShadowPipelines.emplace(addr, pipeline);

    return pipeline;    
}
VulkanPipeline* VulkanGraphicsEngine::GetPipeline(uint32_t a_renderTexture, uint32_t a_pipeline)
{
    ICARIAN_ASSERT_MSG(a_pipeline < m_shaderPrograms.Size(), "GetPipeline pipeline out of bounds");
    ICARIAN_ASSERT_MSG(m_shaderPrograms.Exists(a_pipeline), "GetPipeline shader program does not exist");

    const uint64_t addr = (uint64_t)a_renderTexture | (uint64_t)a_pipeline << 32;

    const std::unique_lock g = std::unique_lock(m_pipeLock);
    const auto iter = m_pipelines.find(addr);
    if (iter != m_pipelines.end())
    {
        return iter->second;
    }

    TRACE("Allocating Vulkan Pipeline");
    const VulkanRenderTexture* tex = GetRenderTexture(a_renderTexture);

    vk::RenderPass pass = m_swapchain->GetRenderPass();
    bool hasDepth = false;
    uint32_t textureCount = 1;

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

void VulkanGraphicsEngine::DrawShadow(const glm::mat4& a_lvp, float a_split, uint32_t a_renderLayer, uint32_t a_renderTexture, bool a_cube, vk::CommandBuffer a_commandBuffer, uint32_t a_index)
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
        ICARIAN_ASSERT(program.Data != nullptr);

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

                    ShadowLightShaderBuffer buffer;
                    buffer.LVP = a_lvp;
                    buffer.Split = a_split;

                    shadowLightBuffer->SetData(a_index, &buffer);
                }

                shaderData->PushShadowUniformBuffer(a_commandBuffer, shadowLightInput.Set, shadowLightBuffer, a_index);
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
                        ICARIAN_ASSERT(model != nullptr);

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
                                const glm::vec3 pos = transform[3].xyz();

                                if (frustum.CompareSphere(pos, radius)) 
                                {
                                    // Scale slightly to improve shadows around edges of objects
                                    transforms[finalTransformCount++] = glm::scale(transform, glm::vec3(1.025));
                                }
                            }
                        }

                        PROFILESTACK("Draw");
                        
                        if (finalTransformCount <= 0) 
                        {
                            continue;
                        }

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

                            pipeline->Bind(a_index, a_commandBuffer);
                        }

                        model->Bind(a_commandBuffer);

                        ShaderBufferInput modelSlot;
                        if (shaderData->GetShadowShaderBufferInput(ShaderBufferType_SSModelBuffer, &modelSlot)) 
                        {
                            ModelShaderBuffer* modelBuffer = new ModelShaderBuffer[finalTransformCount];
                            IDEFER(delete[] modelBuffer);

                            for (uint32_t j = 0; j < finalTransformCount; ++j) 
                            {
                                const glm::mat4& mat = transforms[j];

                                modelBuffer[j].Model = mat;
                                modelBuffer[j].InvModel = glm::inverse(mat);
                            }

                            VulkanShaderStorageObject* storage = new VulkanShaderStorageObject(m_vulkanEngine, sizeof(ModelShaderBuffer) * finalTransformCount, finalTransformCount, modelBuffer);
                            IDEFER(delete storage);

                            shaderData->PushShadowShaderStorageObject(a_commandBuffer, modelSlot.Set, storage, a_index);

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

                            pipeline->Bind(a_index, a_commandBuffer);
                        }

                        if (model == nullptr) 
                        {
                            model = GetModel(modelBuffer.ModelAddr);
                            ICARIAN_ASSERT(model != nullptr);

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

                            BoneShaderBuffer* boneBuffer = new BoneShaderBuffer[boneCount];
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

                            const VulkanShaderStorageObject* storage = new VulkanShaderStorageObject(m_vulkanEngine, sizeof(BoneShaderBuffer) * boneCount, boneCount, boneBuffer);
                            IDEFER(delete storage);

                            shaderData->PushShaderStorageObject(a_commandBuffer, boneSlot.Set, storage, a_index);
                        }

                        shaderData->UpdateTransformBuffer(a_commandBuffer, transform);

                        a_commandBuffer.drawIndexed(indexCount, 1, 0, 0, 0);
                    }
                }
            }
        }
    }
}

vk::CommandBuffer VulkanGraphicsEngine::DirectionalShadowPass(uint32_t a_camIndex, uint32_t a_bufferIndex, uint32_t a_index)
{
    // Could possibly reverse the pass order and do culling on the previous pass to speed this up
    Profiler::Start("Dir Shadow Pass");
    IDEFER(Profiler::Stop());

    Profiler::StartFrame("Update");

    if (m_directionalLights.Size() == 0)
    {
        Profiler::StopFrame();

        return nullptr;
    }

    const CameraBuffer& camBuffer = m_cameraBuffers[a_camIndex];

    const vk::CommandBuffer commandBuffer = StartCommandBuffer(a_bufferIndex, a_index);

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

    const std::vector<DirectionalLightBuffer> lights = m_directionalLights.ToVector();
    const std::vector<bool> state = m_directionalLights.ToStateVector();
    const uint32_t size = (uint32_t)lights.size();

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

            const glm::mat4* lvp = lightData.GetLVP();
            const float* splits = lightData.GetSplits();
            ICARIAN_ASSERT(lvp != nullptr);
            ICARIAN_ASSERT(splits != nullptr);

            commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
            IDEFER(commandBuffer.endRenderPass());

            commandBuffer.setScissor(0, 1, &scissor);
            commandBuffer.setViewport(0, 1, &viewport);
            
            DrawShadow(lvp[0], splits[0], buffer.RenderLayer, lightRenderTexture, false, commandBuffer, a_index);

            {
                PROFILESTACK("Post Shadow");

                m_postShadowFunc->Exec(shadowArgs);
            }
        }
    }

    renderCommand.Flush();

    commandBuffer.end();

    Profiler::StopFrame();

    return commandBuffer;
}
vk::CommandBuffer VulkanGraphicsEngine::PointShadowPass(uint32_t a_camIndex, uint32_t a_bufferIndex, uint32_t a_index)
{
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

        return nullptr;
    }

    const CameraBuffer& camBuffer = m_cameraBuffers[a_camIndex];

    const vk::CommandBuffer commandBuffer = StartCommandBuffer(a_bufferIndex, a_index);

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

    const std::vector<PointLightBuffer> lights = m_pointLights.ToActiveVector();

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

            DrawShadow(lvp, 0.0f, buffer.RenderLayer, renderTextureIndex, true, commandBuffer, a_index);
        }
    }

    renderCommand.Flush();

    commandBuffer.end();

    Profiler::StopFrame();

    return commandBuffer;
}
vk::CommandBuffer VulkanGraphicsEngine::SpotShadowPass(uint32_t a_camIndex, uint32_t a_bufferIndex, uint32_t a_index)
{
    Profiler::Start("Spot Shadow Pass");
    IDEFER(Profiler::Stop());

    Profiler::StartFrame("Update");

    if (m_spotLights.Size() == 0)
    {
        Profiler::StopFrame();

        return nullptr;
    }

    const CameraBuffer& camBuffer = m_cameraBuffers[a_camIndex];

    const vk::CommandBuffer commandBuffer = StartCommandBuffer(a_bufferIndex, a_index);

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

    const std::vector<SpotLightBuffer> lights = m_spotLights.ToVector();
    const std::vector<bool> state = m_spotLights.ToStateVector();
    const uint32_t size = (uint32_t)lights.size();

    for (uint32_t i = 0; i < size; ++i)
    {
        if (!state[i])
        {
            continue;
        }

        const SpotLightBuffer& buffer = lights[i];
        ICARIAN_ASSERT(buffer.Data != nullptr);

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

        const glm::mat4* lvp = lightData.GetLVP();
        ICARIAN_ASSERT(lvp != nullptr);

        commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
        IDEFER(commandBuffer.endRenderPass());

        commandBuffer.setScissor(0, 1, &scissor);
        commandBuffer.setViewport(0, 1, &viewport);
        
        DrawShadow(lvp[0], buffer.Radius, buffer.RenderLayer, lightRenderTexture, false, commandBuffer, a_index);

        {
            PROFILESTACK("Post Shadow");

            m_postShadowFunc->Exec(shadowArgs);
        }
    }

    renderCommand.Flush();

    commandBuffer.end();

    Profiler::StopFrame();

    return commandBuffer;
}

vk::CommandBuffer VulkanGraphicsEngine::DrawPass(uint32_t a_camIndex, uint32_t a_bufferIndex, uint32_t a_index) 
{
    Profiler::Start("Draw Pass");
    IDEFER(Profiler::Stop());

    Profiler::StartFrame("Update");

    const CameraBuffer& camBuffer = m_cameraBuffers[a_camIndex];
    
    const vk::CommandBuffer commandBuffer = StartCommandBuffer(a_bufferIndex, a_index);

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

    const TReadLockArray<MaterialRenderStack*> stacks = m_renderStacks.ToReadLockArray();
    for (const MaterialRenderStack* renderStack : stacks)
    {
        const uint32_t matAddr = renderStack->GetMaterialAddr();
        const TReadLockArray<RenderProgram> programs = m_shaderPrograms.ToReadLockArray();

        const RenderProgram& program = programs.Get(matAddr);
        if (camBuffer.RenderLayer & program.RenderLayer)
        {
            const VulkanPipeline* pipeline = renderCommand.BindMaterial(matAddr);
            ICARIAN_ASSERT(pipeline != nullptr);
            const VulkanShaderData* shaderData = (VulkanShaderData*)program.Data;
            ICARIAN_ASSERT(shaderData != nullptr);

            const uint32_t modelCount = renderStack->GetModelBufferCount();
            const ModelBuffer* modelBuffers = renderStack->GetModelBuffers();
            for (uint32_t i = 0; i < modelCount; ++i)
            {
                PROFILESTACK("Models");

                const ModelBuffer& modelBuffer = modelBuffers[i];
                if (modelBuffer.ModelAddr != -1)
                {
                    const VulkanModel* model = GetModel(modelBuffer.ModelAddr);
                    ICARIAN_ASSERT(model != nullptr);
                    
                    const float radius = model->GetRadius();
                    const uint32_t indexCount = model->GetIndexCount();

                    std::vector<glm::mat4> transforms;
                    transforms.reserve(modelBuffer.TransformCount);

                    const uint32_t transformCount = modelBuffer.TransformCount;
                    {
                        PROFILESTACK("Culling");
                        for (uint32_t j = 0; j < transformCount; ++j)
                        {
                            const uint32_t transformAddr = modelBuffer.TransformAddr[j];
                            if (transformAddr != -1)
                            {
                                const glm::mat4 transform = ObjectManager::GetGlobalMatrix(transformAddr);
                                const glm::vec3 position = transform[3].xyz();

                                if (frustum.CompareSphere(position, radius))
                                {
                                    transforms.emplace_back(transform);
                                }
                            }
                        }
                    }

                    if (!transforms.empty())
                    {
                        PROFILESTACK("Draw");

                        model->Bind(commandBuffer);

                        ShaderBufferInput modelSlot;
                        if (shaderData->GetShaderBufferInput(ShaderBufferType_SSModelBuffer, &modelSlot))
                        {
                            const uint32_t count = (uint32_t)transforms.size();

                            ModelShaderBuffer* modelBuffer = new ModelShaderBuffer[count];
                            IDEFER(delete[] modelBuffer);

                            for (uint32_t j = 0; j < count; ++j)
                            {
                                const glm::mat4& mat = transforms[j];

                                modelBuffer[j].Model = mat;
                                modelBuffer[j].InvModel = glm::inverse(mat);
                            }

                            VulkanShaderStorageObject* storage = new VulkanShaderStorageObject(m_vulkanEngine, sizeof(ModelShaderBuffer) * count, count, modelBuffer);
                            IDEFER(delete storage);

                            shaderData->PushShaderStorageObject(commandBuffer, modelSlot.Set, storage, a_index);

                            commandBuffer.drawIndexed(indexCount, count, 0, 0, 0);
                        }
                        else 
                        {
                            for (const glm::mat4& mat : transforms)
                            {
                                shaderData->UpdateTransformBuffer(commandBuffer, mat);

                                commandBuffer.drawIndexed(indexCount, 1, 0, 0, 0);
                            }
                        }
                    }
                }
            }

            const uint32_t skinnedModelCount = renderStack->GetSkinnedModelBufferCount();
            const SkinnedModelBuffer* skinnedModelBuffers = renderStack->GetSkinnedModelBuffers();
            for (uint32_t i = 0; i < skinnedModelCount; ++i)
            {
                PROFILESTACK("Skinned");

                const SkinnedModelBuffer& modelBuffer = skinnedModelBuffers[i];
                if (modelBuffer.ModelAddr != -1)
                {
                    const VulkanModel* model = GetModel(modelBuffer.ModelAddr);
                    
                    if (model != nullptr)
                    {
                        bool modelBound = false;

                        const float radius = model->GetRadius();
                        const uint32_t indexCount = model->GetIndexCount();

                        const uint32_t objectCount = modelBuffer.ObjectCount;
                        for (uint32_t j = 0; j < objectCount; ++j)
                        {
                            const uint32_t transformAddr = modelBuffer.TransformAddr[j];
                            if (transformAddr != -1)
                            {
                                const glm::mat4 transform = ObjectManager::GetGlobalMatrix(transformAddr);
                                const glm::vec3 position = transform[3].xyz();

                                if (frustum.CompareSphere(position, radius))
                                {
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

                                        BoneShaderBuffer* boneBuffer = new BoneShaderBuffer[boneCount];
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

                                        const VulkanShaderStorageObject* storage = new VulkanShaderStorageObject(m_vulkanEngine, sizeof(BoneShaderBuffer) * boneCount, boneCount, boneBuffer);
                                        IDEFER(delete storage);

                                        shaderData->PushShaderStorageObject(commandBuffer, boneSlot.Set, storage, a_index);
                                    }      

                                    shaderData->UpdateTransformBuffer(commandBuffer, transform);

                                    commandBuffer.drawIndexed(indexCount, 1, 0, 0, 0);                              
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    // Temporary until I get a forward render pass
    {
        const uint32_t renderTextureAddr = renderCommand.GetRenderTexutreAddr();
        const std::vector<VulkanGraphicsParticle2D*> particleSystems = m_particleEmitters.ToActiveVector();

        for (VulkanGraphicsParticle2D* pSys : particleSystems)
        {
            pSys->Update(a_index, a_bufferIndex, commandBuffer, renderTextureAddr);
        }
    }

    {
        PROFILESTACK("Post Render");

        m_postRenderFunc->Exec(camArgs);
    }

    renderCommand.Flush();
    
    commandBuffer.end();

    Profiler::StopFrame();

    return commandBuffer;
}
vk::CommandBuffer VulkanGraphicsEngine::LightPass(uint32_t a_camIndex, uint32_t a_bufferIndex, uint32_t a_index)
{
    Profiler::Start("Light Pass");
    IDEFER(Profiler::Stop());

    Profiler::StartFrame("Update");

    VulkanPushPool* pushPool = m_vulkanEngine->GetPushPool();

    const CameraBuffer& camBuffer = m_cameraBuffers[a_camIndex];

    const vk::CommandBuffer commandBuffer = StartCommandBuffer(a_bufferIndex, a_index);

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

            const std::vector<DirectionalLightBuffer> lights = m_directionalLights.ToVector();
            const std::vector<bool> state = m_directionalLights.ToStateVector();
            const uint32_t size = (uint32_t)lights.size();

            for (uint32_t j = 0; j < size; ++j)
            {
                if (!state[j])
                {
                    continue;
                }

                const DirectionalLightBuffer& buffer = lights[j];
                if (buffer.TransformAddr == -1 || buffer.Intensity <= 0.0f)
                {
                    continue;
                }

                const VulkanLightBuffer* lightBuffer = (VulkanLightBuffer*)buffer.Data;
                ICARIAN_ASSERT(lightBuffer != nullptr);
                
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

                const glm::mat4* lvp = lightData.GetLVP();
                const float* splits = lightData.GetSplits();
                ICARIAN_ASSERT(lvp != nullptr);
                ICARIAN_ASSERT(splits != nullptr);

                const VulkanPipeline* pipeline = renderCommand.GetPipeline();
                if (pipeline == nullptr)
                {
                    continue;
                }

                const VulkanShaderData* data = pipeline->GetShaderData();
                ICARIAN_ASSERT(data != nullptr);

                ShaderBufferInput dirLightInput;
                if (data->GetShaderBufferInput(ShaderBufferType_DirectionalLightBuffer, &dirLightInput))
                {
                    VulkanUniformBuffer* uniformBuffer = pushPool->AllocateDirectionalLightUniformBuffer();

                    const glm::mat4 tMat = ObjectManager::GetGlobalMatrix(buffer.TransformAddr);
                    const glm::vec3 forward = glm::normalize(tMat[2].xyz());

                    DirectionalLightShaderBuffer shaderBuffer;
                    shaderBuffer.LightDir = glm::vec4(forward, buffer.Intensity);
                    shaderBuffer.LightColor = buffer.Color;

                    uniformBuffer->SetData(a_index, &shaderBuffer);

                    data->PushUniformBuffer(commandBuffer, dirLightInput.Set, uniformBuffer, a_index);
                }

                const uint32_t count = lightBuffer->LightRenderTextureCount;

                ShaderBufferInput shadowLightInput;
                if (data->GetShaderBufferInput(ShaderBufferType_SSShadowLightBuffer, &shadowLightInput))
                {
                    ShadowLightShaderBuffer* shadowLightBuffer = new ShadowLightShaderBuffer[count];
                    IDEFER(delete[] shadowLightBuffer);

                    for (uint32_t k = 0; k < count; ++k)
                    {
                        const uint32_t lightRenderTexture = lightBuffer->LightRenderTextures[k];
                        const VulkanDepthRenderTexture* depthRenderTexture = GetDepthRenderTexture(lightRenderTexture);

                        shadowLightBuffer[k].LVP = lvp[k];
                        shadowLightBuffer[k].Split = splits[k];
                    }

                    VulkanShaderStorageObject* storage = new VulkanShaderStorageObject(m_vulkanEngine, sizeof(ShadowLightShaderBuffer) * count, count, shadowLightBuffer);
                    IDEFER(delete storage);

                    data->PushShaderStorageObject(commandBuffer, shadowLightInput.Set, storage, a_index);
                }

                ShaderBufferInput shadowTextureInput;
                if (data->GetShaderBufferInput(ShaderBufferType_AShadowTexture2D, &shadowTextureInput))
                {
                    TextureSamplerBuffer* buffer = new TextureSamplerBuffer[count];
                    IDEFER(
                    {
                        for (uint32_t k = 0; k < count; ++k)
                        {
                            delete (VulkanTextureSampler*)buffer[k].Data;
                        }

                        delete[] buffer;
                    });

                    for (uint32_t k = 0; k < count; ++k)
                    {
                        const uint32_t lightRenderTexture = lightBuffer->LightRenderTextures[k];

                        buffer[k].Addr = lightRenderTexture;
                        buffer[k].Slot = 0;
                        buffer[k].TextureMode = TextureMode_DepthRenderTexture;
                        buffer[k].FilterMode = TextureFilter_Linear;
                        buffer[k].AddressMode = TextureAddress_ClampToEdge;
                        buffer[k].Data = VulkanTextureSampler::GenerateFromBuffer(m_vulkanEngine, this, buffer[k]);
                    }

                    data->PushTextures(commandBuffer, shadowTextureInput.Set, buffer, count, a_index);
                }

                commandBuffer.draw(4, 1, 0, 0);
            }

            break;
        }
        case LightType_Point:
        {
            PROFILESTACK("S Point Light");

            const std::vector<PointLightBuffer> lights = m_pointLights.ToVector();
            const std::vector<bool> state = m_pointLights.ToStateVector();
            const uint32_t size = (uint32_t)lights.size();

            for (uint32_t j = 0; j < size; ++j)
            {
                if (!state[j])
                {
                    continue;
                }

                const PointLightBuffer& buffer = lights[j];
                if (buffer.TransformAddr == -1 || (buffer.RenderLayer & camBuffer.RenderLayer) == 0 || buffer.Radius <= 0.0f || buffer.Intensity <= 0.0f)
                {
                    continue;
                }

                const VulkanLightBuffer* lightBuffer = (VulkanLightBuffer*)buffer.Data;
                ICARIAN_ASSERT(lightBuffer != nullptr);

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
                ICARIAN_ASSERT(data != nullptr);

                ShaderBufferInput pointLightInput;
                if (data->GetShaderBufferInput(ShaderBufferType_PointLightBuffer, &pointLightInput))
                {
                    VulkanUniformBuffer* uniformBuffer = pushPool->AllocatePointLightUniformBuffer();

                    PointLightShaderBuffer shaderBuffer;
                    shaderBuffer.LightPos = glm::vec4(position, buffer.Intensity);
                    shaderBuffer.LightColor = buffer.Color;
                    shaderBuffer.Radius = buffer.Radius;

                    uniformBuffer->SetData(a_index, &shaderBuffer);

                    data->PushUniformBuffer(commandBuffer, pointLightInput.Set, uniformBuffer, a_index);
                }

                ShaderBufferInput shadowTextureInput;
                if (data->GetShaderBufferInput(ShaderBufferType_ShadowTextureCube, &shadowTextureInput))
                {
                    TextureSamplerBuffer buffer;
                    
                    buffer.Addr = lightBuffer->LightRenderTextures[0];
                    buffer.Slot = 0;
                    buffer.TextureMode = TextureMode_DepthCubeRenderTexture;
                    buffer.FilterMode = TextureFilter_Linear;
                    buffer.AddressMode = TextureAddress_ClampToEdge;
                    buffer.Data = VulkanTextureSampler::GenerateFromBuffer(m_vulkanEngine, this, buffer);
                    IDEFER(delete (VulkanTextureSampler*)buffer.Data);

                    data->PushTexture(commandBuffer, shadowTextureInput.Set, buffer, a_index);
                }
                
                commandBuffer.draw(4, 1, 0, 0);
            }

            break;
        }
        case LightType_Spot:
        {
            PROFILESTACK("S Spot Light");

            const std::vector<SpotLightBuffer> lights = m_spotLights.ToVector();
            const std::vector<bool> state = m_spotLights.ToStateVector();
            const uint32_t size = (uint32_t)lights.size();

            for (uint32_t j = 0; j < size; ++j)
            {
                if (!state[j])
                {
                    continue;
                }

                const SpotLightBuffer& buffer = lights[j];

                const VulkanLightBuffer* lightBuffer = (VulkanLightBuffer*)buffer.Data;
                ICARIAN_ASSERT(lightBuffer != nullptr);

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

                const glm::mat4* lvp = lightData.GetLVP();
                ICARIAN_ASSERT(lvp != nullptr);

                const VulkanPipeline* pipeline = renderCommand.GetPipeline();
                if (pipeline == nullptr)
                {
                    continue;
                }

                const VulkanShaderData* data = pipeline->GetShaderData();
                ICARIAN_ASSERT(data != nullptr);

                ShaderBufferInput spotLightInput;
                if (data->GetShaderBufferInput(ShaderBufferType_SpotLightBuffer, &spotLightInput))
                {
                    VulkanUniformBuffer* uniformBuffer = pushPool->AllocateSpotLightUniformBuffer();

                    const glm::mat4 tMat = ObjectManager::GetGlobalMatrix(buffer.TransformAddr);
                    const glm::vec3 forward = glm::normalize(tMat[2].xyz());

                    SpotLightShaderBuffer shaderBuffer;
                    shaderBuffer.LightPos = position;
                    shaderBuffer.LightDir = glm::vec4(forward, buffer.Intensity);
                    shaderBuffer.LightColor = buffer.Color;
                    shaderBuffer.CutoffAngle = glm::vec3(buffer.CutoffAngle, buffer.Radius);

                    uniformBuffer->SetData(a_index, &shaderBuffer);

                    data->PushUniformBuffer(commandBuffer, spotLightInput.Set, uniformBuffer, a_index);
                }

                ShaderBufferInput shadowTextureInput;
                if (data->GetShaderBufferInput(ShaderBufferType_ShadowTexture2D, &shadowTextureInput))
                {
                    TextureSamplerBuffer buffer;

                    buffer.Addr = lightBuffer->LightRenderTextures[0];
                    buffer.Slot = 0;
                    buffer.TextureMode = TextureMode_DepthRenderTexture;
                    buffer.FilterMode = TextureFilter_Linear;
                    buffer.AddressMode = TextureAddress_ClampToEdge;
                    buffer.Data = VulkanTextureSampler::GenerateFromBuffer(m_vulkanEngine, this, buffer);
                    IDEFER(delete (VulkanTextureSampler*)buffer.Data);

                    data->PushTexture(commandBuffer, shadowTextureInput.Set, buffer, a_index);
                }

                ShaderBufferInput shadowBufferInput;
                if (data->GetShaderBufferInput(ShaderBufferType_ShadowLightBuffer, &shadowBufferInput))
                {
                    VulkanUniformBuffer* uniformBuffer = pushPool->AllocateShadowUniformBuffer();

                    ShadowLightShaderBuffer shaderBuffer;
                    shaderBuffer.LVP = lvp[0];
                    shaderBuffer.Split = buffer.Radius;

                    uniformBuffer->SetData(a_index, &shaderBuffer);

                    data->PushUniformBuffer(commandBuffer, shadowBufferInput.Set, uniformBuffer, a_index);
                }

                commandBuffer.draw(4, 1, 0, 0);
            }

            break;
        }
        default:
        {
            ICARIAN_ASSERT(0);
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

            const std::vector<AmbientLightBuffer> lights = m_ambientLights.ToActiveVector();
            const uint32_t size = (uint32_t)lights.size();

            ShaderBufferInput ambientLightInput;
            if (data->GetShaderBufferInput(ShaderBufferType_AmbientLightBuffer, &ambientLightInput))
            {
                for (const AmbientLightBuffer& ambientLight : lights)
                {
                    if (camBuffer.RenderLayer & ambientLight.RenderLayer && ambientLight.Intensity > 0.0f)
                    {
                        VulkanUniformBuffer* uniformBuffer = pushPool->AllocateAmbientLightUniformBuffer();

                        AmbientLightShaderBuffer buffer;
                        buffer.LightColor = ambientLight.Color;
                        buffer.LightColor.w = ambientLight.Intensity;

                        uniformBuffer->SetData(a_index, &buffer);

                        data->PushUniformBuffer(commandBuffer, ambientLightInput.Set, uniformBuffer, a_index);

                        commandBuffer.draw(4, 1, 0, 0);
                    }
                }
            }
            else if (data->GetShaderBufferInput(ShaderBufferType_SSAmbientLightBuffer, &ambientLightInput))
            {
                std::vector<AmbientLightShaderBuffer> buffers;
                buffers.reserve(size);

                for (const AmbientLightBuffer& ambientLight : lights)
                {
                    if (camBuffer.RenderLayer & ambientLight.RenderLayer && ambientLight.Intensity > 0.0f)
                    {
                        AmbientLightShaderBuffer buffer;
                        buffer.LightColor = ambientLight.Color;
                        buffer.LightColor.w = ambientLight.Intensity;

                        buffers.emplace_back(buffer);
                    }
                }

                if (!buffers.empty())
                {
                    const uint32_t count = (uint32_t)buffers.size();

                    VulkanShaderStorageObject* storage = new VulkanShaderStorageObject(m_vulkanEngine, sizeof(AmbientLightShaderBuffer) * count, count, buffers.data());
                    IDEFER(delete storage);

                    data->PushShaderStorageObject(commandBuffer, ambientLightInput.Set, storage, a_index);

                    commandBuffer.draw(4, 1, 0, 0);
                }
            }

            break;
        }
        case LightType_Directional:
        {
            PROFILESTACK("Dir Light");

            const std::vector<DirectionalLightBuffer> lights = m_directionalLights.ToActiveVector();

            ShaderBufferInput dirLightInput;
            if (data->GetShaderBufferInput(ShaderBufferType_DirectionalLightBuffer, &dirLightInput))
            {
                for (const DirectionalLightBuffer& dirLight : lights)
                {
                    if (dirLight.TransformAddr != -1 && camBuffer.RenderLayer & dirLight.RenderLayer && dirLight.Intensity > 0.0f)
                    {
                        const VulkanLightBuffer* lightData = (VulkanLightBuffer*)dirLight.Data;
                        ICARIAN_ASSERT(lightData != nullptr);

                        const bool isShadowLight = lightData->LightRenderTextureCount > 0;
                        if (isShadowLight)
                        {
                            continue;
                        }

                        VulkanUniformBuffer* uniformBuffer = pushPool->AllocateDirectionalLightUniformBuffer();

                        const glm::mat4 tMat = ObjectManager::GetGlobalMatrix(dirLight.TransformAddr);
                        const glm::vec3 forward = glm::normalize(tMat[2].xyz());

                        DirectionalLightShaderBuffer buffer;
                        buffer.LightDir = glm::vec4(forward, dirLight.Intensity);
                        buffer.LightColor = dirLight.Color;

                        uniformBuffer->SetData(a_index, &buffer);
                        
                        data->PushUniformBuffer(commandBuffer, dirLightInput.Set, uniformBuffer, a_index);

                        commandBuffer.draw(4, 1, 0, 0);
                    }
                }
            }
            else if (data->GetShaderBufferInput(ShaderBufferType_SSDirectionalLightBuffer, &dirLightInput))
            {
                const uint32_t size = (uint32_t)lights.size();
                std::vector<DirectionalLightShaderBuffer> buffers;
                buffers.reserve(size);

                for (const DirectionalLightBuffer& dirLight : lights)
                {
                    if (dirLight.TransformAddr != -1 && camBuffer.RenderLayer & dirLight.RenderLayer && dirLight.Intensity > 0.0f)
                    {
                        const VulkanLightBuffer* lightData = (VulkanLightBuffer*)dirLight.Data;
                        ICARIAN_ASSERT(lightData != nullptr);

                        const bool isShadowLight = lightData->LightRenderTextureCount > 0;
                        if (isShadowLight)
                        {
                            continue;
                        }

                        const glm::mat4 tMat = ObjectManager::GetGlobalMatrix(dirLight.TransformAddr);
                        const glm::vec3 forward = glm::normalize(tMat[2].xyz());

                        DirectionalLightShaderBuffer buffer;
                        buffer.LightDir = glm::vec4(forward, dirLight.Intensity);
                        buffer.LightColor = dirLight.Color;

                        buffers.emplace_back(buffer);
                    }
                }

                if (!buffers.empty())
                {
                    const uint32_t count = (uint32_t)buffers.size();

                    VulkanShaderStorageObject* storage = new VulkanShaderStorageObject(m_vulkanEngine, sizeof(DirectionalLightShaderBuffer) * count, count, buffers.data());
                    IDEFER(delete storage);

                    data->PushShaderStorageObject(commandBuffer, dirLightInput.Set, storage, a_index);

                    commandBuffer.draw(4, 1, 0, 0);
                }
            }
            
            break;
        }
        case LightType_Point:
        {
            PROFILESTACK("Point Light");
            const std::vector<PointLightBuffer> lights = m_pointLights.ToActiveVector();

            ShaderBufferInput pointLightInput;
            if (data->GetShaderBufferInput(ShaderBufferType_PointLightBuffer, &pointLightInput))
            {
                for (const PointLightBuffer& pointLight : lights)
                {
                    if (pointLight.TransformAddr != -1 && camBuffer.RenderLayer & pointLight.RenderLayer && pointLight.Radius > 0.0f && pointLight.Intensity > 0.0f)
                    {
                        const VulkanLightBuffer* lightData = (VulkanLightBuffer*)pointLight.Data;
                        ICARIAN_ASSERT(lightData != nullptr);

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

                        PointLightShaderBuffer buffer;
                        buffer.LightPos = glm::vec4(position, pointLight.Intensity);
                        buffer.LightColor = pointLight.Color;
                        buffer.Radius = pointLight.Radius;

                        uniformBuffer->SetData(a_index, &buffer);

                        data->PushUniformBuffer(commandBuffer, pointLightInput.Set, uniformBuffer, a_index);

                        commandBuffer.draw(4, 1, 0, 0);
                    }
                }
            }
            else if (data->GetShaderBufferInput(ShaderBufferType_SSPointLightBuffer, &pointLightInput))
            {
                const uint32_t size = (uint32_t)lights.size();
                std::vector<PointLightShaderBuffer> buffers;
                buffers.reserve(size);

                for (const PointLightBuffer& pointLight : lights)
                {
                    if (pointLight.TransformAddr != -1 && camBuffer.RenderLayer & pointLight.RenderLayer && pointLight.Radius > 0.0f && pointLight.Intensity > 0.0f)
                    {
                        const VulkanLightBuffer* lightData = (VulkanLightBuffer*)pointLight.Data;
                        ICARIAN_ASSERT(lightData != nullptr);

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

                        PointLightShaderBuffer buffer;
                        buffer.LightPos = glm::vec4(position, pointLight.Intensity);
                        buffer.LightColor = pointLight.Color;
                        buffer.Radius = pointLight.Radius;

                        buffers.emplace_back(buffer);
                    }
                }

                if (!buffers.empty())
                {
                    const uint32_t count = (uint32_t)buffers.size();

                    VulkanShaderStorageObject* storage = new VulkanShaderStorageObject(m_vulkanEngine, sizeof(PointLightShaderBuffer) * count, count, buffers.data());
                    IDEFER(delete storage);

                    data->PushShaderStorageObject(commandBuffer, pointLightInput.Set, storage, a_index);

                    commandBuffer.draw(4, 1, 0, 0);
                }
            }

            break;
        }
        case LightType_Spot:
        {
            PROFILESTACK("Spot Light");

            const std::vector<SpotLightBuffer> lights = m_spotLights.ToActiveVector();

            ShaderBufferInput spotLightInput;
            if (data->GetShaderBufferInput(ShaderBufferType_SpotLightBuffer, &spotLightInput))
            {
                for (const SpotLightBuffer& spotLight : lights)
                {
                    if (spotLight.TransformAddr != -1 && camBuffer.RenderLayer & spotLight.RenderLayer)
                    {
                        const VulkanLightBuffer* lightData = (VulkanLightBuffer*)spotLight.Data;
                        ICARIAN_ASSERT(lightData != nullptr);

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

                        SpotLightShaderBuffer buffer;
                        buffer.LightPos = position;
                        buffer.LightDir = glm::vec4(forward, spotLight.Intensity);
                        buffer.LightColor = spotLight.Color;
                        buffer.CutoffAngle = glm::vec3(spotLight.CutoffAngle, spotLight.Radius);

                        uniformBuffer->SetData(a_index, &buffer);

                        data->PushUniformBuffer(commandBuffer, spotLightInput.Set, uniformBuffer, a_index);

                        commandBuffer.draw(4, 1, 0, 0);
                    }
                }
            }
            else if (data->GetShaderBufferInput(ShaderBufferType_SSSpotLightBuffer, &spotLightInput))
            {
                const uint32_t size = (uint32_t)lights.size();

                std::vector<SpotLightShaderBuffer> buffers;
                buffers.reserve(size);

                for (const SpotLightBuffer& spotLight : lights)
                {
                    if (spotLight.TransformAddr != -1 && camBuffer.RenderLayer & spotLight.RenderLayer)
                    {
                        const VulkanLightBuffer* lightData = (VulkanLightBuffer*)spotLight.Data;
                        ICARIAN_ASSERT(lightData != nullptr);

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

                        SpotLightShaderBuffer buffer;
                        buffer.LightPos = position;
                        buffer.LightDir = glm::vec4(forward, spotLight.Intensity);
                        buffer.LightColor = spotLight.Color;
                        buffer.CutoffAngle = glm::vec3(spotLight.CutoffAngle, spotLight.Radius);

                        buffers.emplace_back(buffer);
                    }
                }

                if (!buffers.empty())
                {
                    const uint32_t count = (uint32_t)buffers.size();

                    VulkanShaderStorageObject* storage = new VulkanShaderStorageObject(m_vulkanEngine, sizeof(SpotLightShaderBuffer) * count, count, buffers.data());
                    IDEFER(delete storage);

                    data->PushShaderStorageObject(commandBuffer, spotLightInput.Set, storage, a_index);

                    commandBuffer.draw(4, 1, 0, 0);
                }
            }

            break;
        }
        default:
        {
            ICARIAN_ASSERT_MSG(0, "Unknown Light Type");

            break;
        }
        }

        m_postLightFunc->Exec(lightArgs);
    }

    renderCommand.Flush();

    commandBuffer.end();

    Profiler::StopFrame();

    return commandBuffer;
}
vk::CommandBuffer VulkanGraphicsEngine::PostPass(uint32_t a_camIndex, uint32_t a_bufferIndex, uint32_t a_index)
{
    Profiler::Start("Post Pass");
    IDEFER(Profiler::Stop());

    Profiler::StartFrame("Update");

    const vk::CommandBuffer commandBuffer = StartCommandBuffer(a_bufferIndex, a_index);

    VulkanRenderCommand& renderCommand = m_renderCommands.Push(VulkanRenderCommand(m_vulkanEngine, this, m_swapchain, commandBuffer, a_camIndex, a_bufferIndex));

    void* camArgs[] = 
    { 
        &a_camIndex 
    };

    m_postProcessFunc->Exec(camArgs);

    renderCommand.Flush();

    commandBuffer.end();

    Profiler::StopFrame();

    return commandBuffer;
}

void VulkanGraphicsEngine::DrawUIElement(vk::CommandBuffer a_commandBuffer, uint32_t a_addr, const CanvasBuffer& a_canvas, const glm::vec2& a_screenSize, uint32_t a_index)
{
    if (a_addr == -1)
    {
        return;
    }

    UIElement* element = UIControl::GetUIElement(a_addr);
    if (element != nullptr)
    {
        const glm::vec2 pos = element->GetCanvasPosition(a_canvas, a_screenSize);
        const glm::vec2 scale = element->GetCanvasScale(a_canvas, a_screenSize);

        const glm::vec2 screenPos = pos * a_canvas.ReferenceResolution;
        const glm::vec2 screenSize = scale * a_canvas.ReferenceResolution;

        const vk::Rect2D scissor = vk::Rect2D({ (int32_t)screenPos.x, (int32_t)screenPos.y }, { (uint32_t)screenSize.x, (uint32_t)screenSize.y });
        a_commandBuffer.setScissor(0, 1, &scissor);

        const vk::Viewport viewport = vk::Viewport(screenPos.x, screenPos.y, screenSize.x, screenSize.y, 0.0f, 1.0f);
        a_commandBuffer.setViewport(0, 1, &viewport);

        element->Update(m_vulkanEngine->GetRenderEngine());

        const glm::vec2 baseSize = element->GetSize();
        const glm::vec2 size = baseSize;

        VulkanPipeline* pipeline = nullptr;
        VulkanShaderData* shaderData = nullptr;

        switch (element->GetType()) 
        {
        case UIElementType_Text:
        {
            const TextUIElement* text = (TextUIElement*)element;

            if (text->IsValid())
            {
                pipeline = GetPipeline(-1, m_textUIPipelineAddr);
                ICARIAN_ASSERT(pipeline != nullptr);
                shaderData = pipeline->GetShaderData();
                ICARIAN_ASSERT(shaderData != nullptr);

                ICARIAN_ASSERT_MSG_R(text->GetSamplerAddr() < m_textureSampler.Size(), "Invalid Sampler Address");
                const TextureSamplerBuffer& sampler = m_textureSampler[text->GetSamplerAddr()];

                shaderData->PushTexture(a_commandBuffer, 0, sampler, a_index);
            }

            break;
        }
        case UIElementType_Image:
        {
            const ImageUIElement* image = (ImageUIElement*)element;

            pipeline = GetPipeline(-1, m_imageUIPipelineAddr);
            ICARIAN_ASSERT(pipeline != nullptr);
            shaderData = pipeline->GetShaderData();
            ICARIAN_ASSERT(shaderData != nullptr);
            
            ICARIAN_ASSERT_MSG_R(image->GetSamplerAddr() < m_textureSampler.Size(), "Invalid Sampler Address");
            const TextureSamplerBuffer& sampler = m_textureSampler[image->GetSamplerAddr()];

            shaderData->PushTexture(a_commandBuffer, 0, sampler, a_index);

            break;
        }
        default:
        {
            ICARIAN_ASSERT_MSG(0, "Invalid UI Element Type");
        }
        }

        if (pipeline != nullptr && shaderData != nullptr)
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
}

typedef vk::CommandBuffer (VulkanGraphicsEngine::*DrawFunc)(uint32_t a_camIndex, uint32_t a_bufferIndex, uint32_t a_index);

struct DrawCallBind 
{
    VulkanGraphicsEngine* Engine;

    uint32_t CamIndex;
    uint32_t BufferIndex;
    uint32_t Index;

    DrawFunc Function;

    DrawCallBind() = default;
    DrawCallBind(const DrawCallBind& a_other) = default;
    DrawCallBind(VulkanGraphicsEngine* a_engine, uint32_t a_camIndex, uint32_t a_bufferIndex, uint32_t a_index, DrawFunc a_function) 
    {
        Engine = a_engine;

        CamIndex = a_camIndex;
        BufferIndex = a_bufferIndex;
        Index = a_index;

        Function = a_function;
    }

    inline vk::CommandBuffer operator()() const 
    {
        return (Engine->*Function)(CamIndex, BufferIndex, Index);
    }
};

std::vector<vk::CommandBuffer> VulkanGraphicsEngine::Update(double a_delta, double a_time, uint32_t a_index)
{
    // TODO: Prebuild camera uniform buffers
    Profiler::StartFrame("Drawing Setup");
    m_renderCommands.Clear();

    {
        const std::vector<bool> state = m_shaderPrograms.ToStateVector();
        TLockArray<RenderProgram> a = m_shaderPrograms.ToLockArray();
        const uint32_t size = (uint32_t)state.size();

        for (uint32_t i = 0; i < size; ++i)
        {
            if (!state[i])
            {
                continue;
            }

            RenderProgram& program = a[i];
            ICARIAN_ASSERT(program.Data != nullptr);

            VulkanShaderData* shaderData = (VulkanShaderData*)program.Data;
            shaderData->Update(a_index, program);
        }

        TimeShaderBuffer timeBuffer;
        timeBuffer.Time.x = (float)a_delta;
        timeBuffer.Time.y = (float)a_time;

        m_timeUniform->SetData(a_index, &timeBuffer);
    }

    const vk::Device device = m_vulkanEngine->GetLogicalDevice();

    const uint32_t camBufferSize = m_cameraBuffers.Size();
    std::vector<uint32_t> camIndices;
    for (uint32_t i = 0; i < camBufferSize; ++i)
    {
        if (m_cameraBuffers[i].TransformAddr != -1)
        {
            camIndices.emplace_back(i);
        }
    }

    TReadLockArray<CanvasRendererBuffer> canvasBuffer = m_canvasRenderers.ToReadLockArray();
    const uint32_t canvasCount = canvasBuffer.Size();
    const uint32_t camIndexSize = (uint32_t)camIndices.size();
    const uint32_t totalPoolSize = camIndexSize * DrawingPassCount + canvasCount;
    const uint32_t poolSize = (uint32_t)m_commandPool[a_index].size();

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
            ICARIAN_ASSERT_MSG_R(device.createCommandPool(&poolInfo, nullptr, &pool) == vk::Result::eSuccess, "Failed to create graphics command pool");
            
            m_commandPool[a_index].emplace_back(pool);

            const vk::CommandBufferAllocateInfo commandBufferInfo = vk::CommandBufferAllocateInfo
            (
                pool,
                vk::CommandBufferLevel::ePrimary,
                1
            );

            vk::CommandBuffer buffer;
            ICARIAN_ASSERT_MSG_R(device.allocateCommandBuffers(&commandBufferInfo, &buffer) == vk::Result::eSuccess, "Failed to allocate graphics command buffer");

            m_commandBuffers[a_index].emplace_back(buffer);
        }
    }

    const uint32_t camUniformSize = (uint32_t)m_cameraUniforms.size();

    if (camUniformSize < totalPoolSize)
    {
        TRACE("Allocating camera ubos");
        const uint32_t diff = totalPoolSize - camUniformSize;
        for (uint32_t i = 0; i < diff; ++i)
        {
            m_cameraUniforms.emplace_back(new VulkanUniformBuffer(m_vulkanEngine, sizeof(CameraShaderBuffer)));
        }
    }

    for (uint32_t i = 0; i < poolSize; ++i)
    {
        device.resetCommandPool(m_commandPool[a_index][i]);
    }

    Profiler::StopFrame();

    PROFILESTACK("Drawing Cmd");

    std::vector<std::future<vk::CommandBuffer>> futures;
    for (uint32_t i = 0; i < camIndexSize; ++i)
    {
        const uint32_t camIndex = camIndices[i];
        const uint32_t poolIndex = i * DrawingPassCount;

        FThreadJob<vk::CommandBuffer, DrawCallBind>* dirShadowJob = new FThreadJob<vk::CommandBuffer, DrawCallBind>
        (
            DrawCallBind(this, camIndex, poolIndex + 0, a_index, &VulkanGraphicsEngine::DirectionalShadowPass),
            JobPriority_EngineUrgent
        );
        FThreadJob<vk::CommandBuffer, DrawCallBind>* pointShadowJob = new FThreadJob<vk::CommandBuffer, DrawCallBind>
        (
            DrawCallBind(this, camIndex, poolIndex + 1, a_index, &VulkanGraphicsEngine::PointShadowPass),
            JobPriority_EngineUrgent
        );
        FThreadJob<vk::CommandBuffer, DrawCallBind>* spotShadowJob = new FThreadJob<vk::CommandBuffer, DrawCallBind>
        (
            DrawCallBind(this, camIndex, poolIndex + 2, a_index, &VulkanGraphicsEngine::SpotShadowPass),
            JobPriority_EngineUrgent
        );
        FThreadJob<vk::CommandBuffer, DrawCallBind>* drawJob = new FThreadJob<vk::CommandBuffer, DrawCallBind>
        (   
            DrawCallBind(this, camIndex, poolIndex + 3, a_index, &VulkanGraphicsEngine::DrawPass), 
            JobPriority_EngineUrgent
        );
        FThreadJob<vk::CommandBuffer, DrawCallBind>* lightJob = new FThreadJob<vk::CommandBuffer, DrawCallBind>
        (
            DrawCallBind(this, camIndex, poolIndex + 4, a_index, &VulkanGraphicsEngine::LightPass),
            JobPriority_EngineUrgent
        );
        FThreadJob<vk::CommandBuffer, DrawCallBind>* postJob = new FThreadJob<vk::CommandBuffer, DrawCallBind>
        (
            DrawCallBind(this, camIndex, poolIndex + 5, a_index, &VulkanGraphicsEngine::PostPass),
            JobPriority_EngineUrgent
        );

        futures.emplace_back(dirShadowJob->GetFuture());
        futures.emplace_back(pointShadowJob->GetFuture());
        futures.emplace_back(spotShadowJob->GetFuture());
        futures.emplace_back(drawJob->GetFuture());
        futures.emplace_back(lightJob->GetFuture());
        futures.emplace_back(postJob->GetFuture());

        // Quite pleased with myself got a 2x performance improvement by switching to a thread pool over async
        // And that is without using priorities either
        // Now here is hoping that it lasts when doing actual grunt work
        ThreadPool::PushJob(dirShadowJob);
        ThreadPool::PushJob(pointShadowJob);
        ThreadPool::PushJob(spotShadowJob);
        ThreadPool::PushJob(drawJob);
        ThreadPool::PushJob(lightJob);
        ThreadPool::PushJob(postJob);
    }
    
    std::vector<vk::CommandBuffer> uiBuffers;
    {
        PROFILESTACK("UI Draw");

        for (uint32_t i = 0; i < canvasCount; ++i)
        {
            const CanvasRendererBuffer& canvasRenderer = canvasBuffer[i];
            if (canvasRenderer.IsDestroyed() || canvasRenderer.CanvasAddr == -1)
            {
                continue;
            }

            const CanvasBuffer& canvas = UIControl::GetCanvas(canvasRenderer.CanvasAddr);
            if (canvas.IsDestroyed())
            {
                continue;
            }

            // While we wait for other threads can draw UI on main render thread 
            vk::CommandBuffer buffer = StartCommandBuffer(camIndexSize * DrawingPassCount + i, a_index);

            glm::ivec2 renderSize;
            const VulkanRenderTexture* renderTexture = GetRenderTexture(canvasRenderer.RenderTextureAddr);

            if (renderTexture != nullptr)
            {
                renderSize = glm::ivec2((int)renderTexture->GetWidth(), (int)renderTexture->GetHeight());
                const vk::RenderPassBeginInfo renderPassInfo = vk::RenderPassBeginInfo
                (
                    renderTexture->GetRenderPassNoClear(),
                    renderTexture->GetFramebuffer(),
                    vk::Rect2D({ 0, 0 }, { (uint32_t)renderSize.x, (uint32_t)renderSize.y }),
                    renderTexture->GetTotalTextureCount(),
                    renderTexture->GetClearValues()
                );

                buffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
            }
            else 
            {
                renderSize = m_swapchain->GetSize();
                constexpr vk::ClearValue ClearColor = vk::ClearValue();
                const vk::RenderPassBeginInfo renderPassInfo = vk::RenderPassBeginInfo
                (
                    m_swapchain->GetRenderPassNoClear(),
                    m_swapchain->GetFramebuffer(m_vulkanEngine->GetImageIndex()),
                    vk::Rect2D({ 0, 0 }, { (uint32_t)renderSize.x, (uint32_t)renderSize.y }),
                    1,
                    &ClearColor
                );

                buffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
            }

            for (uint32_t j = 0; j < canvas.ChildElementCount; ++j)
            {
                DrawUIElement(buffer, canvas.ChildElements[j], canvas, renderSize, a_index);
            }

            buffer.endRenderPass();

            buffer.end();

            uiBuffers.emplace_back(buffer);
        }
    }
    
    std::vector<vk::CommandBuffer> cmdBuffers;
    {
        PROFILESTACK("Draw Wait");

        for (std::future<vk::CommandBuffer>& f : futures)
        {
            f.wait();
            vk::CommandBuffer buffer = f.get();
            if (buffer != vk::CommandBuffer(nullptr))
            {
                cmdBuffers.emplace_back(buffer);
            }
        }

        if (!uiBuffers.empty())
        {
            cmdBuffers.insert(cmdBuffers.end(), uiBuffers.begin(), uiBuffers.end());
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
    ICARIAN_ASSERT_MSG(a_vertices != nullptr, "GenerateModel vertices null");
    ICARIAN_ASSERT_MSG(a_vertexCount > 0, "GenerateModel no vertices");
    ICARIAN_ASSERT_MSG(a_indices != nullptr, "GenerateModel indices null");
    ICARIAN_ASSERT_MSG(a_indexCount > 0, "GenerateModel no indices");
    ICARIAN_ASSERT_MSG(a_vertexStride > 0, "GenerateModel vertex stride 0");

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
        ICARIAN_ASSERT_MSG(a_addr < m_models.Size(), "DestroyModel out of bounds");
        ICARIAN_ASSERT_MSG(m_models.Exists(a_addr), "DestroyModel already destroyed");

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

    ICARIAN_ASSERT_MSG(finalAddr < m_models.Size(), "GetModel out of bounds");
    ICARIAN_ASSERT_MSG(m_models.Exists(finalAddr), "GetModel already destroyed");

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

VulkanRenderTexture* VulkanGraphicsEngine::GetRenderTexture(uint32_t a_addr)
{
    if (a_addr == -1)
    {
        return nullptr;
    }

    ICARIAN_ASSERT_MSG(a_addr < m_renderTextures.Size(), "GetRenderTexture out of bounds");

    return m_renderTextures[a_addr];
}
VulkanDepthRenderTexture* VulkanGraphicsEngine::GetDepthRenderTexture(uint32_t a_addr)
{
    if (a_addr == -1)
    {
        return nullptr;
    }

    ICARIAN_ASSERT_MSG(a_addr < m_depthRenderTextures.Size(), "GetDepthRenderTexture out of bounds");

    return m_depthRenderTextures[a_addr];
}
VulkanDepthCubeRenderTexture* VulkanGraphicsEngine::GetDepthCubeRenderTexture(uint32_t a_addr)
{
    if (a_addr == -1)
    {
        return nullptr;
    }

    ICARIAN_ASSERT_MSG(a_addr < m_depthCubeRenderTextures.Size(), "GetDepthCubeRenderTexture out of bounds");

    return m_depthCubeRenderTextures[a_addr];
}

uint32_t VulkanGraphicsEngine::GenerateTextureSampler(uint32_t a_textureAddr, e_TextureMode a_textureMode, e_TextureFilter a_filterMode, e_TextureAddress a_addressMode, uint32_t a_slot)
{
    switch (a_textureMode) 
    {
    case TextureMode_Texture:
    {
        // ICARIAN_ASSERT_MSG(a_textureAddr < m_textures.Size(), "GenerateTextureSampler Texture out of bounds");
        // ICARIAN_ASSERT_MSG(m_textures[a_textureAddr] != nullptr, "GenerateTextureSampler Texture already destroyed");

        break;
    }
    case TextureMode_RenderTexture:
    {
        ICARIAN_ASSERT_MSG(a_textureAddr < m_renderTextures.Size(), "GenerateTextureSampler Render Texture out of bounds");
        ICARIAN_ASSERT_MSG(m_renderTextures[a_textureAddr] != nullptr, "GenerateTextureSampler Render Texture already destroyed");
        ICARIAN_ASSERT_MSG(a_slot < m_renderTextures[a_textureAddr]->GetTextureCount(), "GenerateTextureSampler Render Texture slot out of bounds");

        break;
    }
    case TextureMode_RenderTextureDepth:
    {
        ICARIAN_ASSERT_MSG(a_textureAddr < m_renderTextures.Size(), "GenerateTextureSampler Render Texture out of bounds");
        ICARIAN_ASSERT_MSG(m_renderTextures[a_textureAddr] != nullptr, "GenerateTextureSampler Render Texture already destroyed");

        break;
    }
    case TextureMode_DepthRenderTexture:
    {
        ICARIAN_ASSERT_MSG(a_textureAddr < m_depthRenderTextures.Size(), "GenerateTextureSampler Depth Render Texture out of bounds");
        ICARIAN_ASSERT_MSG(m_depthRenderTextures[a_textureAddr] != nullptr, "GenerateTextureSampler Depth Render Texture already destroyed");

        break;
    }
    default:
    {
        ICARIAN_ASSERT_MSG(0, "GenerateTextureSampler Invalid Texture Mode");

        break;
    }
    }
    
    TextureSamplerBuffer sampler;
    sampler.Addr = a_textureAddr;
    sampler.Slot = a_slot;
    sampler.TextureMode = a_textureMode;
    sampler.AddressMode = a_addressMode;
    sampler.FilterMode = a_filterMode;
    sampler.Data = VulkanTextureSampler::GenerateFromBuffer(m_vulkanEngine, this, sampler);

    return m_textureSampler.PushVal(sampler);
}
void VulkanGraphicsEngine::DestroyTextureSampler(uint32_t a_addr) 
{
    ICARIAN_ASSERT_MSG(a_addr < m_textureSampler.Size(), "DestroyTextureSampler Texture out of bounds");
    ICARIAN_ASSERT_MSG(m_textureSampler[a_addr].TextureMode != TextureMode_Null, "DestroyTextureSampler already destroyed");

    const TextureSamplerBuffer sampler = m_textureSampler[a_addr];
    IDEFER(
    if (sampler.Data != nullptr) 
    { 
        delete (VulkanTextureSampler*)sampler.Data; 
    });

    m_textureSampler.Erase(a_addr);
}

Font* VulkanGraphicsEngine::GetFont(uint32_t a_addr)
{
    ICARIAN_ASSERT_MSG(a_addr < m_fonts.Size(), "GetFont out of bounds");

    return m_fonts[a_addr];
}
#endif