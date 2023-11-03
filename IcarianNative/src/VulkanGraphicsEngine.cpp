#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/VulkanGraphicsEngine.h"

#include <future>
#include <mutex>
#include <vulkan/vulkan_handles.hpp>

#include "Flare/IcarianAssert.h"
#include "Flare/IcarianDefer.h"
#include "Logger.h"
#include "ObjectManager.h"
#include "Profiler.h"
#include "Rendering/AnimationController.h"
#include "Rendering/Light.h"
#include "Rendering/RenderEngine.h"
#include "Rendering/ShaderBuffers.h"
#include "Rendering/UI/ImageUIElement.h"
#include "Rendering/UI/TextUIElement.h"
#include "Rendering/UI/UIControl.h"
#include "Rendering/UI/UIElement.h"
#include "Rendering/Vulkan/VulkanGraphicsEngineBindings.h"
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

VulkanGraphicsEngine::VulkanGraphicsEngine(VulkanRenderEngineBackend* a_vulkanEngine)
{
    m_vulkanEngine = a_vulkanEngine;

    m_pushPool = new VulkanPushPool(m_vulkanEngine);

    m_runtimeBindings = new VulkanGraphicsEngineBindings(this);

    TRACE("Getting RenderPipeline Functions");
    m_shadowSetupFunc = RuntimeManager::GetFunction("IcarianEngine.Rendering", "RenderPipeline", ":ShadowSetupS(uint)");
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
    textProgram.EnableColorBlending = 1;
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
    imageProgram.EnableColorBlending = 1;
    imageProgram.CullingMode = CullMode_None;
    imageProgram.PrimitiveMode = PrimitiveMode_TriangleStrip;
    imageProgram.Flags |= 0b1 << RenderProgram::DestroyFlag;

    m_imageUIPipelineAddr = GenerateRenderProgram(imageProgram);
}
VulkanGraphicsEngine::~VulkanGraphicsEngine()
{
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

    const vk::Device device = m_vulkanEngine->GetLogicalDevice();

    TRACE("Deleting command pool");
    for (uint32_t i = 0; i < VulkanFlightPoolSize; ++i)
    {
        const uint32_t poolSize = (uint32_t)m_commandPool[i].size();
        for (uint32_t j = 0; j < poolSize; ++j)
        {
            device.destroyCommandPool(m_commandPool[i][j]);
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

    TRACE("Checking if shaders where deleted");
    for (uint32_t i = 0; i < m_vertexShaders.Size(); ++i)
    {
        if (m_vertexShaders[i] != nullptr)
        {
            Logger::Warning("Vertex Shader was not destroyed");

            delete m_vertexShaders[i];
        }
    }

    for (uint32_t i = 0; i < m_pixelShaders.Size(); ++i)
    {
        if (m_pixelShaders[i] != nullptr)
        {
            Logger::Warning("Pixel Shader was not destroyed");

            delete m_pixelShaders[i];
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

    TRACE("Checking if render textures where deleted");
    for (uint32_t i = 0; i < m_renderTextures.Size(); ++i)
    {
        if (m_renderTextures[i] != nullptr)
        {
            Logger::Warning("Render Texture was not destroyed");

            delete m_renderTextures[i];
            m_renderTextures[i] = nullptr;
        }
    }
    for (uint32_t i = 0; i < m_depthRenderTextures.Size(); ++i)
    {
        if (m_depthRenderTextures[i] != nullptr)
        {
            Logger::Warning("Depth Render Texture was not destroyed");

            delete m_depthRenderTextures[i];
            m_depthRenderTextures[i] = nullptr;
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

    delete m_pushPool;
}

uint32_t VulkanGraphicsEngine::GenerateGLSLVertexShader(const std::string_view& a_source)
{
    ICARIAN_ASSERT_MSG(!a_source.empty(), "GenerateGLSLVertexShader source is empty");

    VulkanVertexShader* shader = VulkanVertexShader::CreateFromGLSL(m_vulkanEngine, a_source);

    {
        TLockArray<VulkanVertexShader*> a = m_vertexShaders.ToLockArray();

        const uint32_t size = a.Size();
        for (uint32_t i = 0; i < size; ++i)
        {
            if (a[i] == nullptr)
            {
                a[i] = shader;

                return i;
            }
        }
    }

    return m_vertexShaders.PushVal(shader);
}
uint32_t VulkanGraphicsEngine::GenerateFVertexShader(const std::string_view& a_source)
{
    ICARIAN_ASSERT_MSG(!a_source.empty(), "GenerateFVertexShader source is empty");

    VulkanVertexShader* shader = VulkanVertexShader::CreateFromFShader(m_vulkanEngine, a_source);

    {
        TLockArray<VulkanVertexShader*> a = m_vertexShaders.ToLockArray();

        const uint32_t size = a.Size();
        for (uint32_t i = 0; i < size; ++i)
        {
            if (a[i] == nullptr)
            {
                a[i] = shader;

                return i;
            }
        }
    }

    return m_vertexShaders.PushVal(shader);
}
void VulkanGraphicsEngine::DestroyVertexShader(uint32_t a_addr)
{
    ICARIAN_ASSERT_MSG(a_addr < m_vertexShaders.Size(), "DestroyVertexShader out of bounds");
    ICARIAN_ASSERT_MSG(m_vertexShaders[a_addr] != nullptr, "DestroyVertexShader already destroyed");

    const VulkanVertexShader* shader = m_vertexShaders[a_addr];
    IDEFER(delete shader);

    m_vertexShaders.LockSet(a_addr, nullptr);
}

uint32_t VulkanGraphicsEngine::GenerateFPixelShader(const std::string_view& a_source)
{
    ICARIAN_ASSERT_MSG(!a_source.empty(), "GenerateFPixelShader source is empty");

    VulkanPixelShader* shader = VulkanPixelShader::CreateFromFShader(m_vulkanEngine, a_source);

    {
        TLockArray<VulkanPixelShader*> a = m_pixelShaders.ToLockArray();

        const uint32_t size = a.Size();
        for (uint32_t i = 0; i < size; ++i)
        {
            if (a[i] == nullptr)
            {
                a[i] = shader;

                return i;
            }
        }
    }

    return m_pixelShaders.PushVal(shader);
}
uint32_t VulkanGraphicsEngine::GenerateGLSLPixelShader(const std::string_view& a_source)
{
    ICARIAN_ASSERT_MSG(!a_source.empty(), "GenerateGLSLPixelShader source is empty");

    VulkanPixelShader* shader = VulkanPixelShader::CreateFromGLSL(m_vulkanEngine, a_source);

    {
        TLockArray<VulkanPixelShader*> a = m_pixelShaders.ToLockArray();

        const uint32_t size = a.Size();
        for (uint32_t i = 0; i < size; ++i)
        {
            if (a[i] == nullptr)
            {
                a[i] = shader;

                return i;
            }
        }
    }

    return m_pixelShaders.PushVal(shader);
}
void VulkanGraphicsEngine::DestroyPixelShader(uint32_t a_addr)
{
    ICARIAN_ASSERT_MSG(a_addr < m_pixelShaders.Size(), "DestroyPixelShader out of bounds");
    ICARIAN_ASSERT_MSG(m_pixelShaders[a_addr] != nullptr, "DestroyPixelShader already destroyed");

    const VulkanPixelShader* shader = m_pixelShaders[a_addr];
    IDEFER(delete shader);

    m_pixelShaders.LockSet(a_addr, nullptr);
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

vk::CommandBuffer VulkanGraphicsEngine::ShadowPass(uint32_t a_camIndex, uint32_t a_bufferIndex, uint32_t a_index)
{
    const CameraBuffer& camBuffer = m_cameraBuffers[a_camIndex];

    const vk::CommandBuffer commandBuffer = StartCommandBuffer(a_bufferIndex, a_index);

    VulkanRenderCommand& renderCommand = m_renderCommands.Push(VulkanRenderCommand(m_vulkanEngine, this, m_swapchain, commandBuffer, -1, a_bufferIndex));
    VulkanLightData& lightData = m_lightData.Push(VulkanLightData());

    void* shadowSetupArgs[] =
    {
        &a_camIndex
    };

    m_shadowSetupFunc->Exec(shadowSetupArgs);

    for (uint32_t i = 0; i < LightType_End; ++i)
    {
        switch ((e_LightType)i)
        {
        case LightType_Directional:
        {
            const TReadLockArray<DirectionalLightBuffer> a = m_directionalLights.ToReadLockArray();
            const std::vector<bool> state = m_directionalLights.ToStateVector();

            const uint32_t size = a.Size();
            for (uint32_t j = 0; j < size; ++j)
            {
                if (state[j])
                {
                    const DirectionalLightBuffer& buffer = a[j];

                    if (buffer.RenderLayer & camBuffer.RenderLayer)
                    {
                        if (buffer.Data == nullptr)
                        {
                            continue;
                        }

                        const VulkanLightBuffer* lightBuffer = (VulkanLightBuffer*)buffer.Data;

                        for (uint32_t k = 0; k < lightBuffer->LightRenderTextureCount; ++k)
                        {
                            void* shadowArgs[] =
                            {
                                &i,
                                &j,
                                &a_camIndex,
                                &k
                            };

                            m_preShadowFunc->Exec(shadowArgs);

                            const VulkanLightRenderTexture& lightRenderTexture = lightBuffer->LightRenderTextures[k];
                            const VulkanDepthRenderTexture* depthRenderTexture = GetDepthRenderTexture(lightRenderTexture.TextureAddr);

                            const glm::vec2 screenSize = glm::vec2(depthRenderTexture->GetWidth(), depthRenderTexture->GetHeight());

                            constexpr vk::ClearValue ClearDepth = vk::ClearValue(vk::ClearDepthStencilValue(1.0f, 0));

                            const vk::Rect2D scissor = vk::Rect2D({ 0, 0 }, { (uint32_t)screenSize.x, (uint32_t)screenSize.y });

                            const vk::RenderPassBeginInfo renderPassInfo = vk::RenderPassBeginInfo
                            (
                                depthRenderTexture->GetRenderPass(),
                                depthRenderTexture->GetFrameBuffer(),
                                scissor,
                                1,
                                &ClearDepth
                            );

                            commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

                            commandBuffer.setScissor(0, 1, &scissor);

                            const vk::Viewport viewport = vk::Viewport(0.0f, 0.0f, screenSize.x, screenSize.y, 0.0f, 1.0f);
                            commandBuffer.setViewport(0, 1, &viewport);

                            const glm::mat4* lvp = lightData.GetLVP();
                            if (lvp == nullptr)
                            {
                                continue;
                            }
                            
                            float split = 0.0f;
                            const float* splits = lightData.GetSplits();
                            if (splits != nullptr)
                            {
                                split = splits[0];
                            }

                            const Frustum frustum = Frustum::FromMat4(lvp[0]);

                            {
                                TReadLockArray<MaterialRenderStack*> stacks = m_renderStacks.ToReadLockArray();

                                for (const MaterialRenderStack* renderStack : stacks)
                                {
                                    const uint32_t materialAddr = renderStack->GetMaterialAddr();
                                    const RenderProgram& program = m_shaderPrograms[materialAddr];
                                    ICARIAN_ASSERT(program.Data != nullptr);

                                    VulkanShaderData* shaderData = (VulkanShaderData*)program.Data;

                                    if (buffer.RenderLayer & program.RenderLayer && program.ShadowVertexShader != -1)
                                    {
                                        VulkanPipeline* pipeline = GetShadowPipeline(lightRenderTexture.TextureAddr, materialAddr);

                                        pipeline->Bind(a_index, commandBuffer);

                                        shaderData->UpdateShadowLightBuffer(commandBuffer, lvp[0], split);

                                        const uint32_t modelCount = renderStack->GetModelBufferCount();
                                        const ModelBuffer* modelBuffers = renderStack->GetModelBuffers();

                                        for (uint32_t l = 0; l < modelCount; ++l)
                                        {
                                            const ModelBuffer& modelBuffer = modelBuffers[l];
                                            if (modelBuffer.ModelAddr != -1)
                                            {
                                                const VulkanModel* model = m_models[modelBuffer.ModelAddr]; 
                                                ICARIAN_ASSERT(model != nullptr);

                                                const uint32_t indexCount = model->GetIndexCount();
                                                const float radius = model->GetRadius();

                                                std::vector<glm::mat4> transforms;
                                                transforms.reserve(modelBuffer.TransformCount);

                                                const uint32_t transformCount = modelBuffer.TransformCount;
                                                for (uint32_t m = 0; m < transformCount; ++m)
                                                {
                                                    const uint32_t transformAddr = modelBuffer.TransformAddr[m];
                                                    if (transformAddr != -1)
                                                    {
                                                        const glm::mat4 transform = ObjectManager::GetGlobalMatrix(modelBuffer.TransformAddr[m]);
                                                        const glm::vec3 pos = transform[3].xyz();

                                                        if (frustum.CompareSphere(pos, radius))
                                                        {
                                                            transforms.emplace_back(transform);
                                                        }
                                                    }
                                                }

                                                if (!transforms.empty())
                                                {
                                                    model->Bind(commandBuffer);

                                                    ShaderBufferInput modelSlot;
                                                    if (shaderData->GetShadowBatchModelBufferInput(&modelSlot))
                                                    {
                                                        const uint32_t count = (uint32_t)transforms.size();

                                                        ModelShaderBuffer* modelBuffer = new ModelShaderBuffer[count];
                                                        IDEFER(delete[] modelBuffer);

                                                        for (uint32_t m = 0; m < count; ++m)
                                                        {
                                                            const glm::mat4& mat = transforms[m];

                                                            modelBuffer[m].Model = mat;
                                                            modelBuffer[m].InvModel = glm::inverse(mat);
                                                        }

                                                        VulkanShaderStorageObject* storage = new VulkanShaderStorageObject(m_vulkanEngine, sizeof(ModelShaderBuffer) * count, count, modelBuffer);
                                                        IDEFER(delete storage);

                                                        shaderData->PushShadowShaderStorageObject(commandBuffer, modelSlot.Set, storage, a_index);

                                                        commandBuffer.drawIndexed(indexCount, count, 0, 0, 0);
                                                    }
                                                    else
                                                    {
                                                        for (const glm::mat4& mat : transforms)
                                                        {
                                                            shaderData->UpdateShadowTransformBuffer(commandBuffer, mat);

                                                            commandBuffer.drawIndexed(indexCount, 1, 0, 0, 0);
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }

                            commandBuffer.endRenderPass();

                            m_postShadowFunc->Exec(shadowArgs);
                        }       
                    }
                }
            }

            break;
        }
        case LightType_Point:
        {
            break;
        }
        case LightType_Spot:
        {
            break;
        }
        default:
        {
            ICARIAN_ASSERT_MSG(0, "Unknown Light Type");

            break;
        }
        }
    }

    renderCommand.Flush();

    commandBuffer.end();

    return commandBuffer;
}
vk::CommandBuffer VulkanGraphicsEngine::DrawPass(uint32_t a_camIndex, uint32_t a_bufferIndex, uint32_t a_index) 
{
    const CameraBuffer& camBuffer = m_cameraBuffers[a_camIndex];
    
    const vk::CommandBuffer commandBuffer = StartCommandBuffer(a_bufferIndex, a_index);

    VulkanRenderCommand& renderCommand = m_renderCommands.Push(VulkanRenderCommand(m_vulkanEngine, this, m_swapchain, commandBuffer, a_camIndex, a_bufferIndex));

    void* camArgs[] = 
    { 
        &a_camIndex 
    };

    m_preRenderFunc->Exec(camArgs);

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
                const ModelBuffer& modelBuffer = modelBuffers[i];
                if (modelBuffer.ModelAddr != -1)
                {
                    const VulkanModel* model = m_models[modelBuffer.ModelAddr];
                    ICARIAN_ASSERT(model != nullptr);
                    
                    const float radius = model->GetRadius();
                    const uint32_t indexCount = model->GetIndexCount();

                    std::vector<glm::mat4> transforms;
                    transforms.reserve(modelBuffer.TransformCount);

                    const uint32_t transformCount = modelBuffer.TransformCount;
                    for (uint32_t j = 0; j < transformCount; ++j)
                    {
                        const uint32_t transformAddr = modelBuffer.TransformAddr[j];
                        if (transformAddr != -1)
                        {
                            const glm::mat4 transform = ObjectManager::GetGlobalMatrix(modelBuffers[i].TransformAddr[j]);
                            const glm::vec3 position = transform[3].xyz();

                            if (frustum.CompareSphere(position, radius))
                            {
                                transforms.emplace_back(transform);
                            }
                        }
                    }

                    if (!transforms.empty())
                    {
                        model->Bind(commandBuffer);

                        ShaderBufferInput modelSlot;
                        if (shaderData->GetBatchModelBufferInput(&modelSlot))
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
                const SkinnedModelBuffer& modelBuffer = skinnedModelBuffers[i];
                if (modelBuffer.ModelAddr != -1)
                {
                    const VulkanModel* model = m_models[modelBuffer.ModelAddr];
                    
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
                                    if (shaderData->GetBoneBufferInput(&boneSlot))
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

                                            const TransformBuffer& buffer = ObjectManager::GetTransformBuffer(bone.TransformIndex);

                                            glm::mat4 transform = buffer.ToMat4();

                                            auto iter = boneMap.find(buffer.Parent);
                                            while (iter != boneMap.end())
                                            {
                                                const uint32_t index = iter->second;

                                                const BoneTransformData& parentBone = skeleton.BoneData[index];
                                                const TransformBuffer& parentBuffer = ObjectManager::GetTransformBuffer(parentBone.TransformIndex);
                                                
                                                transform = parentBuffer.ToMat4() * transform;

                                                iter = boneMap.find(parentBuffer.Parent);
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
    
    m_postRenderFunc->Exec(camArgs);

    renderCommand.Flush();
    
    commandBuffer.end();

    return commandBuffer;
}
vk::CommandBuffer VulkanGraphicsEngine::LightPass(uint32_t a_camIndex, uint32_t a_bufferIndex, uint32_t a_index)
{
    const CameraBuffer& camBuffer = m_cameraBuffers[a_camIndex];

    const vk::CommandBuffer commandBuffer = StartCommandBuffer(a_bufferIndex, a_index);

    VulkanRenderCommand& renderCommand = m_renderCommands.Push(VulkanRenderCommand(m_vulkanEngine, this, m_swapchain, commandBuffer, a_camIndex, a_bufferIndex));
    VulkanLightData& lightData = m_lightData.Push(VulkanLightData());

    void* lightSetupArgs[] =
    {
        &a_camIndex
    };

    m_lightSetupFunc->Exec(lightSetupArgs);

    const VulkanRenderTexture* renderTexture = renderCommand.GetRenderTexture();

    glm::vec2 screenSize = (glm::vec2)m_swapchain->GetSize();
    if (renderTexture != nullptr)
    {
        screenSize = glm::vec2(renderTexture->GetWidth(), renderTexture->GetHeight());
    }

    const Frustum frustum = camBuffer.ToFrustum(screenSize);

    for (uint32_t i = 0; i < LightType_End; ++i)
    {
        switch ((e_LightType)i)
        {
        case LightType_Directional:
        {
            const TReadLockArray<DirectionalLightBuffer> lights = m_directionalLights.ToReadLockArray();
            const uint32_t size = lights.Size();
            const std::vector<bool> state = m_directionalLights.ToStateVector();

            for (uint32_t j = 0; j < size; ++j)
            {
                if (!state[j])
                {
                    continue;
                }

                const DirectionalLightBuffer& buffer = lights[j];

                const VulkanLightBuffer* lightBuffer = (VulkanLightBuffer*)buffer.Data;
                
                const bool isShadowLight = lightBuffer != nullptr && lightBuffer->LightRenderTextureCount > 0;
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

                const glm::mat4* lvp = lightData.GetLVP();
                const float* splits = lightData.GetSplits();
                if (lvp == nullptr || splits == nullptr)
                {
                    continue;
                }

                const VulkanPipeline* pipeline = renderCommand.GetPipeline();
                if (pipeline == nullptr)
                {
                    continue;
                }

                const VulkanShaderData* data = pipeline->GetShaderData();
                ICARIAN_ASSERT(data != nullptr);

                ShaderBufferInput dirLightInput;
                if (data->GetDirectionalLightInput(&dirLightInput))
                {
                    VulkanUniformBuffer* uniformBuffer = m_pushPool->AllocateDirectionalLightUniformBuffer();

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
                if (data->GetShadowLightStorageBufferInput(&shadowLightInput))
                {
                    ShadowLightShaderBuffer* shadowLightBuffer = new ShadowLightShaderBuffer[count];
                    IDEFER(delete[] shadowLightBuffer);

                    for (uint32_t k = 0; k < count; ++k)
                    {
                        const VulkanLightRenderTexture& lightRenderTexture = lightBuffer->LightRenderTextures[k];
                        const VulkanDepthRenderTexture* depthRenderTexture = GetDepthRenderTexture(lightRenderTexture.TextureAddr);

                        shadowLightBuffer[k].LVP = lvp[k];
                        shadowLightBuffer[k].Split = splits[k];
                    }

                    VulkanShaderStorageObject* storage = new VulkanShaderStorageObject(m_vulkanEngine, sizeof(ShadowLightShaderBuffer) * count, count, shadowLightBuffer);
                    IDEFER(delete storage);

                    data->PushShaderStorageObject(commandBuffer, shadowLightInput.Set, storage, a_index);
                }

                ShaderBufferInput shadowTextureInput;
                if (data->GetShadowTextureInput(&shadowTextureInput))
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
                        const VulkanLightRenderTexture& lightRenderTexture = lightBuffer->LightRenderTextures[k];

                        buffer[k].Addr = lightRenderTexture.TextureAddr;
                        buffer[k].Slot = 0;
                        buffer[k].TextureMode = TextureMode_DepthRenderTexture;
                        buffer[k].FilterMode = TextureFilter_Linear;
                        buffer[k].AddressMode = TextureAddress_ClampToEdge;
                        buffer[k].Data = VulkanTextureSampler::GenerateFromBuffer(m_vulkanEngine, this, buffer[k]);
                    }

                    data->PushTextures(commandBuffer, shadowTextureInput.Set, buffer, count, a_index);
                }

                commandBuffer.draw(4, 1, 0, 0);

                m_postShadowLightFunc->Exec(lightArgs);
            }

            break;
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
        case LightType_Directional:
        {
            const TReadLockArray<DirectionalLightBuffer> lights = m_directionalLights.ToReadLockArray();
            const uint32_t size = lights.Size();
            const std::vector<bool> state = m_directionalLights.ToStateVector();

            ShaderBufferInput dirLightInput;
            if (data->GetDirectionalLightInput(&dirLightInput))
            {
                for (uint32_t j = 0; j < size; ++j)
                {
                    if (!state[j])
                    {
                        continue;
                    }

                    const DirectionalLightBuffer& dirLight = lights[j];

                    if (dirLight.TransformAddr != -1 && camBuffer.RenderLayer & dirLight.RenderLayer)
                    {
                        const VulkanLightBuffer* lightData = (VulkanLightBuffer*)dirLight.Data;

                        const bool isShadowLight = lightData != nullptr && lightData->LightRenderTextureCount > 0;
                        if (isShadowLight)
                        {
                            continue;
                        }

                        VulkanUniformBuffer* uniformBuffer = m_pushPool->AllocateDirectionalLightUniformBuffer();

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
            else if (data->GetBatchDirectionalLightInput(&dirLightInput))
            {
                std::vector<DirectionalLightShaderBuffer> buffers;
                buffers.reserve(size);

                for (uint32_t j = 0; j < size; ++j)
                {
                    if (!state[j])
                    {
                        continue;
                    }

                    const DirectionalLightBuffer& dirLight = lights[j];

                    if (dirLight.TransformAddr != -1 && camBuffer.RenderLayer & dirLight.RenderLayer)
                    {
                        const VulkanLightBuffer* lightData = (VulkanLightBuffer*)dirLight.Data;

                        const bool isShadowLight = lightData != nullptr && lightData->LightRenderTextureCount > 0;
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
            const TReadLockArray<PointLightBuffer> lights = m_pointLights.ToReadLockArray();
            const uint32_t size = lights.Size();
            const std::vector<bool> state = m_pointLights.ToStateVector();

            ShaderBufferInput pointLightInput;
            if (data->GetPointLightInput(&pointLightInput))
            {
                for (uint32_t j = 0; j < size; ++j)
                {
                    if (!state[j])
                    {
                        continue;
                    }

                    const PointLightBuffer& pointLight = lights[j];

                    if (pointLight.TransformAddr != -1 && camBuffer.RenderLayer & pointLight.RenderLayer)
                    {
                        const glm::mat4 tMat = ObjectManager::GetGlobalMatrix(pointLight.TransformAddr);
                        const glm::vec3 position = tMat[3].xyz();

                        if (!frustum.CompareSphere(position, pointLight.Radius))
                        {
                            continue;
                        }

                        VulkanUniformBuffer* uniformBuffer = m_pushPool->AllocatePointLightUniformBuffer();

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
            else if (data->GetBatchPointLightInput(&pointLightInput))
            {
                std::vector<PointLightShaderBuffer> buffers;
                buffers.reserve(size);

                for (uint32_t j = 0; j < size; ++j)
                {
                    if (!state[j])
                    {
                        continue;
                    }

                    const PointLightBuffer& pointLight = lights[j];

                    if (pointLight.TransformAddr != -1 && camBuffer.RenderLayer & pointLight.RenderLayer)
                    {
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
            TReadLockArray<SpotLightBuffer> lights = m_spotLights.ToReadLockArray();
            const uint32_t size = lights.Size();
            const std::vector<bool> state = m_spotLights.ToStateVector();

            ShaderBufferInput spotLightInput;
            if (data->GetSpotLightInput(&spotLightInput))
            {
                for (uint32_t j = 0; j < size; ++j)
                {
                    if (!state[j])
                    {
                        continue;
                    }

                    const SpotLightBuffer& spotLight = lights[j];

                    if (spotLight.TransformAddr != -1 && camBuffer.RenderLayer & spotLight.RenderLayer)
                    {
                        const glm::mat4 tMat = ObjectManager::GetGlobalMatrix(spotLight.TransformAddr);
                        const glm::vec3 position = tMat[3].xyz();

                        if (!frustum.CompareSphere(position, spotLight.Radius))
                        {
                            continue;
                        }

                        const glm::vec3 forward = glm::normalize(tMat[2].xyz());

                        VulkanUniformBuffer* uniformBuffer = m_pushPool->AllocateSpotLightUniformBuffer();

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
            else if (data->GetBatchSpotLightInput(&spotLightInput))
            {
                std::vector<SpotLightShaderBuffer> buffers;
                buffers.reserve(size);

                for (uint32_t j = 0; j < size; ++j)
                {
                    if (!state[j])
                    {
                        continue;
                    }

                    const SpotLightBuffer& spotLight = lights[j];

                    if (spotLight.TransformAddr != -1 && camBuffer.RenderLayer & spotLight.RenderLayer)
                    {
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

    return commandBuffer;
}
vk::CommandBuffer VulkanGraphicsEngine::PostPass(uint32_t a_camIndex, uint32_t a_bufferIndex, uint32_t a_index)
{
    const vk::CommandBuffer commandBuffer = StartCommandBuffer(a_bufferIndex, a_index);

    VulkanRenderCommand& renderCommand = m_renderCommands.Push(VulkanRenderCommand(m_vulkanEngine, this, m_swapchain, commandBuffer, a_camIndex, a_bufferIndex));

    void* camArgs[] = 
    { 
        &a_camIndex 
    };

    m_postProcessFunc->Exec(camArgs);

    renderCommand.Flush();

    commandBuffer.end();

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

std::vector<vk::CommandBuffer> VulkanGraphicsEngine::Update(uint32_t a_index)
{
    Profiler::StartFrame("Drawing Setup");
    m_renderCommands.Clear();

    m_pushPool->Reset(a_index);

    {
        TLockArray<RenderProgram> a = m_shaderPrograms.ToLockArray();

        for (RenderProgram& program : a)
        {
            ICARIAN_ASSERT(program.Data != nullptr);

            VulkanShaderData* shaderData = (VulkanShaderData*)program.Data;
            shaderData->Update(a_index, program);
        }
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

    for (uint32_t i = 0; i < glm::min(poolSize, totalPoolSize); ++i)
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

        FThreadJob<vk::CommandBuffer, DrawCallBind>* shadowJob = new FThreadJob<vk::CommandBuffer, DrawCallBind>
        (
            DrawCallBind(this, camIndex, poolIndex + 0, a_index, &VulkanGraphicsEngine::ShadowPass),
            JobPriority_EngineUrgent
        );
        FThreadJob<vk::CommandBuffer, DrawCallBind>* drawJob = new FThreadJob<vk::CommandBuffer, DrawCallBind>
        (   
            DrawCallBind(this, camIndex, poolIndex + 1, a_index, &VulkanGraphicsEngine::DrawPass), 
            JobPriority_EngineUrgent
        );
        FThreadJob<vk::CommandBuffer, DrawCallBind>* lightJob = new FThreadJob<vk::CommandBuffer, DrawCallBind>
        (
            DrawCallBind(this, camIndex, poolIndex + 2, a_index, &VulkanGraphicsEngine::LightPass),
            JobPriority_EngineUrgent
        );
        FThreadJob<vk::CommandBuffer, DrawCallBind>* postJob = new FThreadJob<vk::CommandBuffer, DrawCallBind>
        (
            DrawCallBind(this, camIndex, poolIndex + 3, a_index, &VulkanGraphicsEngine::PostPass),
            JobPriority_EngineUrgent
        );

        futures.emplace_back(shadowJob->GetFuture());
        futures.emplace_back(drawJob->GetFuture());
        futures.emplace_back(lightJob->GetFuture());
        futures.emplace_back(postJob->GetFuture());

        // Quite pleased with myself got a 2x performance improvement by switching to a thread pool over async
        // And that is without using priorities either
        // Now here is hoping that it lasts when doing actual grunt work
        ThreadPool::PushJob(shadowJob);
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

    return m_vertexShaders[a_addr];
}
VulkanPixelShader* VulkanGraphicsEngine::GetPixelShader(uint32_t a_addr)
{
    ICARIAN_ASSERT_MSG(a_addr < m_pixelShaders.Size(), "GetPixelShader out of bounds");

    return m_pixelShaders[a_addr];
}

CameraBuffer VulkanGraphicsEngine::GetCameraBuffer(uint32_t a_addr)
{
    ICARIAN_ASSERT_MSG(a_addr < m_cameraBuffers.Size(), "GetCameraBuffer out of bounds");

    return m_cameraBuffers[a_addr];
}

VulkanModel* VulkanGraphicsEngine::GetModel(uint32_t a_addr)
{
    if (a_addr == -1)
    {
        return nullptr;
    }

    ICARIAN_ASSERT_MSG(a_addr < m_models.Size(), "GetModel out of bounds");

    return m_models[a_addr];
}

uint32_t VulkanGraphicsEngine::GenerateAlphaTexture(uint32_t a_width, uint32_t a_height, const void* a_data)
{
    VulkanTexture* texture = VulkanTexture::CreateAlpha(m_vulkanEngine, a_width, a_height, a_data);

    {
        TLockArray<VulkanTexture*> a = m_textures.ToLockArray();

        const uint32_t size = a.Size();
        for (uint32_t i = 0; i < size; ++i)
        {
            if (a[i] == nullptr)
            {
                a[i] = texture;

                return i;
            }
        }
    }

    return m_textures.PushVal(texture);
}
void VulkanGraphicsEngine::DestroyTexture(uint32_t a_addr)
{
    ICARIAN_ASSERT_MSG(a_addr < m_textures.Size(), "DestroyTexture Texture out of bounds");
    ICARIAN_ASSERT_MSG(m_textures[a_addr] != nullptr, "DestroyTexture already destroyed");
    
    const VulkanTexture* texture = m_textures[a_addr];
    IDEFER(delete texture);

    m_textures.LockSet(a_addr, nullptr);
}
VulkanTexture* VulkanGraphicsEngine::GetTexture(uint32_t a_addr)
{
    if (a_addr == -1)
    {
        return nullptr;
    }

    ICARIAN_ASSERT_MSG(a_addr < m_textures.Size(), "GetTexture out of bounds");

    return m_textures[a_addr];
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

uint32_t VulkanGraphicsEngine::GenerateTextureSampler(uint32_t a_textureAddr, e_TextureMode a_textureMode, e_TextureFilter a_filterMode, e_TextureAddress a_addressMode, uint32_t a_slot)
{
    switch (a_textureMode) 
    {
    case TextureMode_Texture:
    {
        ICARIAN_ASSERT_MSG(a_textureAddr < m_textures.Size(), "GenerateTextureSampler Texture out of bounds");
        ICARIAN_ASSERT_MSG(m_textures[a_textureAddr] != nullptr, "GenerateTextureSampler Texture already destroyed");

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