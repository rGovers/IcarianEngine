#include "Rendering/Vulkan/VulkanGraphicsEngine.h"

#include <future>
#include <mutex>
#include <vulkan/vulkan_handles.hpp>

#include "Flare/IcarianAssert.h"
#include "Flare/IcarianDefer.h"
#include "Logger.h"
#include "ObjectManager.h"
#include "Profiler.h"
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
#include "Rendering/Vulkan/VulkanRenderCommand.h"
#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"
#include "Rendering/Vulkan/VulkanRenderTexture.h"
#include "Rendering/Vulkan/VulkanShaderData.h"
#include "Rendering/Vulkan/VulkanSwapchain.h"
#include "Rendering/Vulkan/VulkanTexture.h"
#include "Rendering/Vulkan/VulkanTextureSampler.h"
#include "Rendering/Vulkan/VulkanUniformBuffer.h"
#include "Rendering/Vulkan/VulkanVertexShader.h"
#include "Runtime/RuntimeFunction.h"
#include "Runtime/RuntimeManager.h"
#include "Shaders/UIVertex.h"
#include "Shaders/UIImagePixel.h"
#include "Shaders/UITextPixel.h"
#include "Trace.h"
#include "ThreadPool.h"

VulkanGraphicsEngine::VulkanGraphicsEngine(RuntimeManager* a_runtime, VulkanRenderEngineBackend* a_vulkanEngine)
{
    m_vulkanEngine = a_vulkanEngine;
    m_runtimeManager = a_runtime;

    m_runtimeBindings = new VulkanGraphicsEngineBindings(m_runtimeManager, this);

    TRACE("Getting RenderPipeline Functions");
    m_shadowSetupFunc = m_runtimeManager->GetFunction("IcarianEngine.Rendering", "RenderPipeline", ":ShadowSetupS(uint)");
    m_preShadowFunc = m_runtimeManager->GetFunction("IcarianEngine.Rendering", "RenderPipeline", ":PreShadowS(uint,uint,uint,uint)");
    m_postShadowFunc = m_runtimeManager->GetFunction("IcarianEngine.Rendering", "RenderPipeline", ":PostShadowS(uint,uint,uint,uint)");
    m_preRenderFunc = m_runtimeManager->GetFunction("IcarianEngine.Rendering", "RenderPipeline", ":PreRenderS(uint)");
    m_postRenderFunc = m_runtimeManager->GetFunction("IcarianEngine.Rendering", "RenderPipeline", ":PostRenderS(uint)");
    m_lightSetupFunc = m_runtimeManager->GetFunction("IcarianEngine.Rendering", "RenderPipeline", ":LightSetupS(uint)");
    m_preLightFunc = m_runtimeManager->GetFunction("IcarianEngine.Rendering", "RenderPipeline", ":PreLightS(uint,uint)");
    m_postLightFunc = m_runtimeManager->GetFunction("IcarianEngine.Rendering", "RenderPipeline", ":PostLightS(uint,uint)");
    m_postProcessFunc = m_runtimeManager->GetFunction("IcarianEngine.Rendering", "RenderPipeline", ":PostProcessS(uint)"); 

    FlareBase::RenderProgram textProgram;
    textProgram.VertexShader = GenerateFVertexShader(UIVERTEX);
    textProgram.PixelShader = GenerateFPixelShader(UITEXTPIXEL);
    textProgram.RenderLayer = 0;
    textProgram.VertexStride = 0;
    textProgram.VertexInputCount = 0;
    textProgram.VertexAttribs = nullptr;
    textProgram.ShaderBufferInputCount = 2;
    textProgram.ShaderBufferInputs = new FlareBase::ShaderBufferInput[2];
    textProgram.ShaderBufferInputs[0] = FlareBase::ShaderBufferInput(-1, FlareBase::ShaderBufferType_UIBuffer, FlareBase::ShaderSlot_Pixel);
    textProgram.ShaderBufferInputs[1] = FlareBase::ShaderBufferInput(0, FlareBase::ShaderBufferType_PushTexture, FlareBase::ShaderSlot_Pixel, 0);
    textProgram.EnableColorBlending = true;
    textProgram.CullingMode = FlareBase::CullMode_None;
    textProgram.PrimitiveMode = FlareBase::PrimitiveMode_TriangleStrip;
    textProgram.Flags |= 0b1 << FlareBase::RenderProgram::DestroyFlag;

    m_textUIPipelineAddr = GenerateRenderProgram(textProgram);

    FlareBase::RenderProgram imageProgram;
    imageProgram.VertexShader = GenerateFVertexShader(UIVERTEX);
    imageProgram.PixelShader = GenerateFPixelShader(UIIMAGEPIXEL);
    imageProgram.RenderLayer = 0;
    imageProgram.VertexStride = 0;
    imageProgram.VertexInputCount = 0;
    imageProgram.VertexAttribs = nullptr;
    imageProgram.ShaderBufferInputCount = 2;
    imageProgram.ShaderBufferInputs = new FlareBase::ShaderBufferInput[2];
    imageProgram.ShaderBufferInputs[0] = FlareBase::ShaderBufferInput(-1, FlareBase::ShaderBufferType_UIBuffer, FlareBase::ShaderSlot_Pixel);
    imageProgram.ShaderBufferInputs[1] = FlareBase::ShaderBufferInput(0, FlareBase::ShaderBufferType_PushTexture, FlareBase::ShaderSlot_Pixel, 0);
    imageProgram.EnableColorBlending = true;
    imageProgram.CullingMode = FlareBase::CullMode_None;
    imageProgram.PrimitiveMode = FlareBase::PrimitiveMode_TriangleStrip;
    imageProgram.Flags |= 0b1 << FlareBase::RenderProgram::DestroyFlag;

    m_imageUIPipelineAddr = GenerateRenderProgram(imageProgram);
}
VulkanGraphicsEngine::~VulkanGraphicsEngine()
{
    const FlareBase::RenderProgram textProgram = m_shaderPrograms[m_textUIPipelineAddr];
    IDEFER(
    {
        if (textProgram.VertexAttribs != nullptr)
        {
            delete[] textProgram.VertexAttribs;
        }

        if (textProgram.ShaderBufferInputs != nullptr)
        {
            delete[] textProgram.ShaderBufferInputs;
        }
    });

    const FlareBase::RenderProgram imageProgram = m_shaderPrograms[m_imageUIPipelineAddr];
    IDEFER(
    {
        if (imageProgram.VertexAttribs != nullptr)
        {
            delete[] imageProgram.VertexAttribs;
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

    TRACE("Deleting directional light ubos");
    for (const VulkanUniformBuffer* uniform : m_directionalLightUniforms)
    {
        if (uniform != nullptr)
        {
            delete uniform;
        }
    }
    TRACE("Deleting point light ubos");
    for (const VulkanUniformBuffer* uniform : m_pointLightUniforms)
    {
        if (uniform != nullptr)
        {
            delete uniform;
        }
    }
    TRACE("Deleting spot light ubos");
    for (const VulkanUniformBuffer* uniform : m_spotLightUniforms)
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
        if (!(m_shaderPrograms[i].Flags & 0b1 << FlareBase::RenderProgram::FreeFlag))
        {
            Logger::Warning("Shader buffer was not destroyed");
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
        if (m_textureSampler[i].TextureMode != FlareBase::TextureMode_Null)
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

uint32_t VulkanGraphicsEngine::GenerateRenderProgram(const FlareBase::RenderProgram& a_program)
{
    ICARIAN_ASSERT_MSG(a_program.VertexShader < m_vertexShaders.Size(), "GenerateRenderProgram vertex shader out of bounds");
    ICARIAN_ASSERT_MSG(a_program.PixelShader < m_pixelShaders.Size(), "GenerateRenderProgram pixel shader out of bounds");

    TRACE("Creating Shader Program");
    {
        TLockArray<FlareBase::RenderProgram> a = m_shaderPrograms.ToLockArray();

        const uint32_t size = a.Size();
        for (uint32_t i = 0; i < size; ++i)
        {
            if (a[i].Flags & 0b1 << FlareBase::RenderProgram::FreeFlag)
            {
                ICARIAN_ASSERT_MSG(a[i].Data == nullptr, "GenerateRenderProgram shader data not null");

                a[i] = a_program;
                a[i].Data = new VulkanShaderData(m_vulkanEngine, this, i);

                return i;
            }
        }
    }

    const uint32_t addr = m_shaderPrograms.PushVal(a_program);
    m_shaderPrograms[addr].Data = new VulkanShaderData(m_vulkanEngine, this, addr);

    return addr;
}
void VulkanGraphicsEngine::DestroyRenderProgram(uint32_t a_addr)
{
    ICARIAN_ASSERT_MSG(a_addr < m_shaderPrograms.Size(), "DestroyRenderProgram out of bounds");
    ICARIAN_ASSERT_MSG(!(m_shaderPrograms[a_addr].Flags & 0b1 << FlareBase::RenderProgram::FreeFlag), "DestroyRenderProgram already destroyed");

    TRACE("Destroying Shader Program");
    
    FlareBase::RenderProgram nullProgram;
    nullProgram.Data = nullptr;
    nullProgram.Flags |= 0b1 << FlareBase::RenderProgram::FreeFlag;

    const FlareBase::RenderProgram program = m_shaderPrograms[a_addr];
    IDEFER(
    if (program.Data != nullptr)
    {
        delete (VulkanShaderData*)program.Data;
    });
    
    m_shaderPrograms.LockSet(a_addr, nullProgram);

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

        for (auto iter = m_pipelines.begin(); iter != m_pipelines.end(); ++iter)
        {
            const uint32_t val = (uint32_t)(iter->first >> 32);
            if (val == a_addr)
            {
                const VulkanPipeline* pipeline = iter->second;
                IDEFER(delete pipeline);

                const auto vIter = iter--;
                m_pipelines.erase(vIter);
            }
        }
    }

    if (program.Flags & 0b1 << FlareBase::RenderProgram::DestroyFlag)
    {
        DestroyVertexShader(program.VertexShader);
        DestroyPixelShader(program.PixelShader);
    }
}

FlareBase::RenderProgram VulkanGraphicsEngine::GetRenderProgram(uint32_t a_addr)
{
    ICARIAN_ASSERT_MSG(a_addr < m_shaderPrograms.Size(), "GetRenderProgram out of bounds");

    return m_shaderPrograms[a_addr];
}
VulkanPipeline* VulkanGraphicsEngine::GetPipeline(uint32_t a_renderTexture, uint32_t a_pipeline)
{
    ICARIAN_ASSERT_MSG(a_pipeline < m_shaderPrograms.Size(), "GetPipeline pipeline out of bounds");

    const uint64_t addr = (uint64_t)a_renderTexture | (uint64_t)a_pipeline << 32;

    {
        const std::shared_lock g = std::shared_lock(m_pipeLock);
        const auto iter = m_pipelines.find(addr);
        if (iter != m_pipelines.end())
        {
            return iter->second;
        }
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

    VulkanPipeline* pipeline = new VulkanPipeline(m_vulkanEngine, this, pass, hasDepth, textureCount, a_pipeline);
    {
        const std::unique_lock g = std::unique_lock(m_pipeLock);
        m_pipelines.emplace(addr, pipeline);
    }

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
    const RenderEngine* renderEngine = m_vulkanEngine->GetRenderEngine();
    ObjectManager* objectManager = renderEngine->GetObjectManager();

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
            TReadLockArray<DirectionalLightBuffer> a = m_directionalLights.ToReadLockArray();
            const std::vector<bool> state = m_directionalLights.ToStateVector();

            const uint32_t size = a.Size();
            for (uint32_t j = 0; j < size; ++j)
            {
                if (state[j])
                {
                    const DirectionalLightBuffer& buffer = a[j];

                    if (buffer.RenderLayer & camBuffer.RenderLayer)
                    {
                        const VulkanLightBuffer* lightBuffer = (VulkanLightBuffer*)buffer.Data;
                        ICARIAN_ASSERT(lightBuffer != nullptr);

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
        }
    }

    renderCommand.Flush();

    commandBuffer.end();

    return commandBuffer;
}
vk::CommandBuffer VulkanGraphicsEngine::DrawPass(uint32_t a_camIndex, uint32_t a_bufferIndex, uint32_t a_index) 
{
    const RenderEngine* renderEngine = m_vulkanEngine->GetRenderEngine();
    ObjectManager* objectManager = renderEngine->GetObjectManager();

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
    const Frustum frustum = camBuffer.ToFrustum(screenSize, objectManager);

    const TReadLockArray<MaterialRenderStack*> stacks = m_renderStacks.ToReadLockArray();

    for (const MaterialRenderStack* renderStack : stacks)
    {
        const uint32_t matAddr = renderStack->GetMaterialAddr();
        const FlareBase::RenderProgram& program = m_shaderPrograms[matAddr];

        if (camBuffer.RenderLayer & program.RenderLayer)
        {
            const uint32_t modelCount = renderStack->GetModelBufferCount();
            const ModelBuffer* modelBuffers = renderStack->GetModelBuffers();

            const VulkanPipeline* pipeline = renderCommand.BindMaterial(matAddr);
            ICARIAN_ASSERT(pipeline != nullptr);
            const VulkanShaderData* shaderData = (VulkanShaderData*)program.Data;
            ICARIAN_ASSERT(shaderData != nullptr);

            bool bound = false;

            for (uint32_t i = 0; i < modelCount; ++i)
            {
                const ModelBuffer& modelBuffer = modelBuffers[i];
                if (modelBuffer.ModelAddr != -1)
                {
                    const VulkanModel* model = m_models[modelBuffer.ModelAddr];
                    
                    if (model != nullptr)
                    {
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
                                const glm::mat4 transform = objectManager->GetGlobalMatrix(modelBuffers[i].TransformAddr[j]);
                                const glm::vec3 position = transform[3];

                                if (frustum.CompareSphere(position, radius))
                                {
                                    transforms.emplace_back(transform);
                                }
                            }
                        }

                        if (!transforms.empty())
                        {
                            if (!bound)
                            {
                                pipeline->Bind(a_index, commandBuffer);

                                bound = true;
                            }

                            model->Bind(commandBuffer);

                            for (const glm::mat4& mat : transforms)
                            {
                                shaderData->UpdateTransformBuffer(commandBuffer, mat);

                                commandBuffer.drawIndexed(indexCount, 1, 0, 0, 0);
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

    void* lightSetupArgs[] =
    {
        &a_camIndex
    };

    m_lightSetupFunc->Exec(lightSetupArgs);

    for (uint32_t i = 0; i < LightType_End; ++i)
    // for (uint32_t i = 0; i < 1; ++i)
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

        // TODO: Could probably batch this down the line
        switch ((e_LightType)i)
        {
        case LightType_Directional:
        {
            const std::vector<DirectionalLightBuffer> lights = m_directionalLights.ToVector();

            const FlareBase::ShaderBufferInput dirLightInput = data->GetDirectionalLightInput();

            if (dirLightInput.BufferType == FlareBase::ShaderBufferType_DirectionalLightBuffer)
            {
                const uint32_t dirLightCount = (uint32_t)lights.size();
                for (uint32_t i = 0; i < dirLightCount; ++i)
                {
                    const DirectionalLightBuffer& dirLight = lights[i];

                    if (dirLight.TransformAddr != -1 && camBuffer.RenderLayer & dirLight.RenderLayer)
                    {
                        data->PushUniformBuffer(commandBuffer, dirLightInput.Set, m_directionalLightUniforms[i], a_index);

                        commandBuffer.draw(4, 1, 0, 0);
                    }
                }
            }
            else
            {
                for (const DirectionalLightBuffer& dirLight : lights)
                {
                    if (dirLight.TransformAddr != -1 && camBuffer.RenderLayer & dirLight.RenderLayer)
                    {
                        commandBuffer.draw(4, 1, 0, 0);
                    }
                }
            }
            
            break;
        }
        case LightType_Point:
        {
            const std::vector<PointLightBuffer> lights = m_pointLights.ToVector();

            const FlareBase::ShaderBufferInput pointLightInput = data->GetPointLightInput();

            if (pointLightInput.BufferType == FlareBase::ShaderBufferType_PointLightBuffer)
            {
                const uint32_t pointLightCount = (uint32_t)lights.size();
                for (uint32_t i = 0; i < pointLightCount; ++i)
                {
                    const PointLightBuffer& pointLight = lights[i];

                    if (pointLight.TransformAddr != -1 && camBuffer.RenderLayer & pointLight.RenderLayer)
                    {
                        data->PushUniformBuffer(commandBuffer, pointLightInput.Set, m_pointLightUniforms[i], a_index);

                        commandBuffer.draw(4, 1, 0, 0);
                    }
                }
            }
            else
            {
                for (const PointLightBuffer& pointLight : lights)
                {
                    if (pointLight.TransformAddr != -1 && camBuffer.RenderLayer & pointLight.RenderLayer)
                    {
                        commandBuffer.draw(4, 1, 0, 0);
                    }
                }
            }

            break;
        }
        case LightType_Spot:
        {
            const std::vector<SpotLightBuffer> lights = m_spotLights.ToVector();

            const FlareBase::ShaderBufferInput spotLightInput = data->GetSpotLightInput();

            if (spotLightInput.BufferType == FlareBase::ShaderBufferType_SpotLightBuffer)
            {
                const uint32_t spotLightCount = (uint32_t)lights.size();
                for (uint32_t i = 0; i < spotLightCount; ++i)
                {
                    const SpotLightBuffer& spotLight = lights[i];

                    if (spotLight.TransformAddr != -1 && camBuffer.RenderLayer & spotLight.RenderLayer)
                    {
                        data->PushUniformBuffer(commandBuffer, spotLightInput.Set, m_spotLightUniforms[i], a_index);

                        commandBuffer.draw(4, 1, 0, 0);
                    }
                }
            }
            else
            {
                for (const SpotLightBuffer& spotLight : lights)
                {
                    if (spotLight.TransformAddr != -1 && camBuffer.RenderLayer & spotLight.RenderLayer)
                    {
                        commandBuffer.draw(4, 1, 0, 0);
                    }
                }
            }

            break;
        }
        default:
        {
            Logger::Warning("IcarianEngine: Invalid light type when drawing");

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
                const FlareBase::TextureSampler& sampler = m_textureSampler[text->GetSamplerAddr()];

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
            const FlareBase::TextureSampler& sampler = m_textureSampler[image->GetSamplerAddr()];

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

    const vk::Device device = m_vulkanEngine->GetLogicalDevice();

    ObjectManager* objectManager = m_vulkanEngine->GetRenderEngine()->GetObjectManager();

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

    const uint32_t directionalLightSize = m_directionalLights.Size();
    const uint32_t directionalLightUniformSize = (uint32_t)m_directionalLightUniforms.size();
    if (directionalLightUniformSize < directionalLightSize)
    {
        TRACE("Allocating directional light ubos");
        const uint32_t diff = directionalLightSize - directionalLightUniformSize;
        for (uint32_t i = 0; i < diff; ++i)
        {
            m_directionalLightUniforms.emplace_back(new VulkanUniformBuffer(m_vulkanEngine, sizeof(DirectionalLightShaderBuffer)));
        }
    }

    for (uint32_t i = 0; i < directionalLightSize; ++i)
    {
        const DirectionalLightBuffer& dirLight = m_directionalLights[i];

        if (dirLight.TransformAddr != -1)
        {
            const glm::mat4 tMat = objectManager->GetGlobalMatrix(dirLight.TransformAddr);

            const glm::vec3 forward = glm::normalize(tMat[2].xyz());

            DirectionalLightShaderBuffer buffer;
            buffer.LightDir = glm::vec4(forward, dirLight.Intensity);
            buffer.LightColor = dirLight.Color;

            VulkanUniformBuffer* uniformBuffer = m_directionalLightUniforms[i];
            uniformBuffer->SetData(a_index, &buffer);
        }
    }

    const uint32_t pointLightSize = m_pointLights.Size();
    const uint32_t pointLightUniformSize = (uint32_t)m_pointLightUniforms.size();
    if (pointLightUniformSize < pointLightSize)
    {
        TRACE("Allocating point light ubos");
        const uint32_t diff = pointLightSize - pointLightUniformSize;
        for (uint32_t i = 0; i < diff; ++i)
        {
            m_pointLightUniforms.emplace_back(new VulkanUniformBuffer(m_vulkanEngine, sizeof(PointLightShaderBuffer)));
        }
    }

    for (uint32_t i = 0; i < pointLightSize; ++i)
    {
        const PointLightBuffer& pointLight = m_pointLights[i];

        if (pointLight.TransformAddr != -1)
        {
            const glm::mat4 tMat = objectManager->GetGlobalMatrix(pointLight.TransformAddr);

            const glm::vec3 pos = tMat[3].xyz();

            PointLightShaderBuffer buffer;
            buffer.LightPos = glm::vec4(pos, pointLight.Intensity);
            buffer.LightColor = pointLight.Color;
            buffer.Radius = pointLight.Radius;

            VulkanUniformBuffer* uniformBuffer = m_pointLightUniforms[i];
            uniformBuffer->SetData(a_index, &buffer);
        }
    }

    const uint32_t spotLightSize = m_spotLights.Size();
    const uint32_t spotLightUniformSize = (uint32_t)m_spotLightUniforms.size();
    if (spotLightUniformSize < spotLightSize)
    {
        TRACE("Allocating spot light ubos");
        const uint32_t diff = spotLightSize - spotLightUniformSize;
        for (uint32_t i = 0; i < diff; ++i)
        {
            m_spotLightUniforms.emplace_back(new VulkanUniformBuffer(m_vulkanEngine, sizeof(SpotLightShaderBuffer)));
        }
    }

    for (uint32_t i = 0; i < spotLightSize; ++i)
    {
        const SpotLightBuffer& spotLight = m_spotLights[i];

        if (spotLight.TransformAddr != -1)
        {
            const glm::mat4 tMat = objectManager->GetGlobalMatrix(spotLight.TransformAddr);

            const glm::vec3 pos = tMat[3].xyz();
            const glm::vec3 forward = glm::normalize(tMat[2].xyz());

            SpotLightShaderBuffer buffer;
            buffer.LightPos = pos;
            buffer.LightDir = glm::vec4(forward, spotLight.Intensity);
            buffer.LightColor = spotLight.Color;
            buffer.CutoffAngle = glm::vec3(spotLight.CutoffAngle, spotLight.Radius);

            VulkanUniformBuffer* uniformBuffer = m_spotLightUniforms[i];
            uniformBuffer->SetData(a_index, &buffer);
        }
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

    ICARIAN_ASSERT_MSG(a_addr < m_renderTextures.Size(), "GetDepthRenderTexture out of bounds");

    return m_depthRenderTextures[a_addr];
}

uint32_t VulkanGraphicsEngine::GenerateTextureSampler(uint32_t a_textureAddr, FlareBase::e_TextureMode a_textureMode, FlareBase::e_TextureFilter a_filterMode, FlareBase::e_TextureAddress a_addressMode, uint32_t a_slot)
{
    switch (a_textureMode) 
    {
    case FlareBase::TextureMode_Texture:
    {
        ICARIAN_ASSERT_MSG(a_textureAddr < m_textures.Size(), "GenerateTextureSampler Texture out of bounds");
        ICARIAN_ASSERT_MSG(m_textures[a_textureAddr] != nullptr, "GenerateTextureSampler Texture already destroyed");

        break;
    }
    case FlareBase::TextureMode_RenderTexture:
    {
        ICARIAN_ASSERT_MSG(a_textureAddr < m_renderTextures.Size(), "GenerateTextureSampler Render Texture out of bounds");
        ICARIAN_ASSERT_MSG(m_renderTextures[a_textureAddr] != nullptr, "GenerateTextureSampler Render Texture already destroyed");
        ICARIAN_ASSERT_MSG(a_slot < m_renderTextures[a_textureAddr]->GetTextureCount(), "GenerateTextureSampler Render Texture slot out of bounds");

        break;
    }
    case FlareBase::TextureMode_RenderTextureDepth:
    {
        ICARIAN_ASSERT_MSG(a_textureAddr < m_renderTextures.Size(), "GenerateTextureSampler Render Texture out of bounds");
        ICARIAN_ASSERT_MSG(m_renderTextures[a_textureAddr] != nullptr, "GenerateTextureSampler Render Texture already destroyed");

        break;
    }
    case FlareBase::TextureMode_DepthRenderTexture:
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
    
    FlareBase::TextureSampler sampler;
    sampler.Addr = a_textureAddr;
    sampler.TSlot = a_slot;
    sampler.TextureMode = a_textureMode;
    sampler.AddressMode = a_addressMode;
    sampler.FilterMode = a_filterMode;
    sampler.Data = new VulkanTextureSampler(m_vulkanEngine, sampler);

    {
        TLockArray<FlareBase::TextureSampler> a = m_textureSampler.ToLockArray();

        const uint32_t size = a.Size();
        for (uint32_t i = 0; i < size; ++i)
        {
            if (a[i].TextureMode == FlareBase::TextureMode_Null)
            {
                ICARIAN_ASSERT_MSG(a[i].Data == nullptr, "GenerateTextureSampler sampler data already exists");

                a[i] = sampler;

                return i;
            }
        }
    }

    return m_textureSampler.PushVal(sampler);
}
void VulkanGraphicsEngine::DestroyTextureSampler(uint32_t a_addr) 
{
    ICARIAN_ASSERT_MSG(a_addr < m_textureSampler.Size(), "DestroyTextureSampler Texture out of bounds");
    ICARIAN_ASSERT_MSG(m_textureSampler[a_addr].TextureMode != FlareBase::TextureMode_Null, "DestroyTextureSampler already destroyed");

    FlareBase::TextureSampler nullSampler;
    nullSampler.TextureMode = FlareBase::TextureMode_Null;
    nullSampler.Data = nullptr;

    const FlareBase::TextureSampler sampler = m_textureSampler[a_addr];
    IDEFER(
    if (sampler.Data != nullptr) 
    { 
        delete (VulkanTextureSampler*)sampler.Data; 
    });

    m_textureSampler.LockSet(a_addr, nullSampler);
}

Font* VulkanGraphicsEngine::GetFont(uint32_t a_addr)
{
    ICARIAN_ASSERT_MSG(a_addr < m_fonts.Size(), "GetFont out of bounds");

    return m_fonts[a_addr];
}