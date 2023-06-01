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
#include "Rendering/UI/TextUIElement.h"
#include "Rendering/UI/UIControl.h"
#include "Rendering/UI/UIElement.h"
#include "Rendering/Vulkan/VulkanGraphicsEngineBindings.h"
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
#include "Shaders/UITextPixel.h"
#include "Trace.h"
#include "ThreadPool.h"

VulkanGraphicsEngine::VulkanGraphicsEngine(RuntimeManager* a_runtime, VulkanRenderEngineBackend* a_vulkanEngine)
{
    m_vulkanEngine = a_vulkanEngine;
    m_runtimeManager = a_runtime;

    m_runtimeBindings = new VulkanGraphicsEngineBindings(m_runtimeManager, this);

    m_preShadowFunc = m_runtimeManager->GetFunction("IcarianEngine.Rendering", "RenderPipeline", ":PreShadowS(uint)");
    m_postShadowFunc = m_runtimeManager->GetFunction("IcarianEngine.Rendering", "RenderPipeline", ":PostShadowS(uint)");
    m_preRenderFunc = m_runtimeManager->GetFunction("IcarianEngine.Rendering", "RenderPipeline", ":PreRenderS(uint)");
    m_postRenderFunc = m_runtimeManager->GetFunction("IcarianEngine.Rendering", "RenderPipeline", ":PostRenderS(uint)");
    m_lightSetupFunc = m_runtimeManager->GetFunction("IcarianEngine.Rendering", "RenderPipeline", ":LightSetupS(uint)");
    m_preLightFunc = m_runtimeManager->GetFunction("IcarianEngine.Rendering", "RenderPipeline", ":PreLightS(uint,uint)");
    m_postLightFunc = m_runtimeManager->GetFunction("IcarianEngine.Rendering", "RenderPipeline", ":PostLightS(uint,uint)");
    m_postProcessFunc = m_runtimeManager->GetFunction("IcarianEngine.Rendering", "RenderPipeline", ":PostProcessS(uint)"); 

    FlareBase::RenderProgram program;
    program.VertexShader = GenerateFVertexShader(UIVERTEX);
    program.PixelShader = GenerateFPixelShader(UITEXTPIXEL);
    program.RenderLayer = 0;
    program.VertexStride = 0;
    program.VertexInputCount = 0;
    program.VertexAttribs = nullptr;
    program.ShaderBufferInputCount = 1;
    program.ShaderBufferInputs = new FlareBase::ShaderBufferInput[2];
    // program.ShaderBufferInputs[0] = FlareBase::ShaderBufferInput(-1, FlareBase::ShaderBufferType_UIBuffer, FlareBase::ShaderSlot_Vertex);
    // program.ShaderBufferInputs[1] = FlareBase::ShaderBufferInput(0, FlareBase::ShaderBufferType_PushTexture, FlareBase::ShaderSlot_Pixel, 0);
    program.ShaderBufferInputs[0] = FlareBase::ShaderBufferInput(0, FlareBase::ShaderBufferType_PushTexture, FlareBase::ShaderSlot_Pixel, 0);
    program.EnableColorBlending = true;
    program.CullingMode = FlareBase::CullMode_None;
    program.PrimitiveMode = FlareBase::PrimitiveMode_TriangleStrip;
    program.Flags |= 0b1 << FlareBase::RenderProgram::DestroyFlag;

    m_textUIPipelineAddr = GenerateRenderProgram(program);
}
VulkanGraphicsEngine::~VulkanGraphicsEngine()
{
    DestroyRenderProgram(m_textUIPipelineAddr);

    delete m_runtimeBindings;

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

    VulkanVertexShader* shader = m_vertexShaders[a_addr];
    ICARIAN_DEFER_del(shader);

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

    VulkanPixelShader* shader = m_pixelShaders[a_addr];
    ICARIAN_DEFER_del(shader);

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

    FlareBase::RenderProgram program = m_shaderPrograms[a_addr];
    ICARIAN_DEFER(program, if (program.Data != nullptr) { delete (VulkanShaderData*)program.Data; });
    
    m_shaderPrograms.LockSet(a_addr, nullProgram);

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
        auto iter = m_pipelines.find(addr);
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

vk::CommandBuffer VulkanGraphicsEngine::DrawPass(uint32_t a_camIndex, uint32_t a_bufferIndex, uint32_t a_index) 
{
    // While there is no code relating to mono in here for now.
    // This is used to fix a crash relating to locking a Thread after going from Mono -> Native 
    // When when another thread tries to aquire a lock after and it is not visible from Mono despite still being native will cause MemMap Crash 
    // 
    // ^ No longer applicable as actually using mono functions on this thread however leaving for future reference and reminder to attach all threads
    m_runtimeManager->AttachThread();
    
    const RenderEngine* renderEngine = m_vulkanEngine->GetRenderEngine();
    ObjectManager* objectManager = renderEngine->GetObjectManager();

    const CameraBuffer& camBuffer = m_cameraBuffers[a_camIndex];
    
    const vk::CommandBuffer commandBuffer = StartCommandBuffer(a_bufferIndex, a_index);

    VulkanRenderCommand& renderCommand = m_renderCommands.Push(VulkanRenderCommand(m_vulkanEngine, this, m_swapchain, commandBuffer, a_bufferIndex));

    renderCommand.SetCameraData(a_camIndex);

    void* camArgs[] = 
    { 
        &a_camIndex 
    };

    m_preRenderFunc->Exec(camArgs);

    const std::vector<MaterialRenderStack> stacks = m_renderStacks.ToVector();

    // TODO: Pre-Culling and batching
    for (const MaterialRenderStack& renderStack : stacks)
    {
        const uint32_t matAddr = renderStack.GetMaterialAddr();
        const FlareBase::RenderProgram& program = m_shaderPrograms[matAddr];
        if (camBuffer.RenderLayer & program.RenderLayer)
        {
            const VulkanPipeline* pipeline = renderCommand.BindMaterial(matAddr);
            ICARIAN_ASSERT(pipeline != nullptr);

            const VulkanShaderData* shaderData = (VulkanShaderData*)program.Data;
            ICARIAN_ASSERT(shaderData != nullptr);
            
            const std::vector<ModelBuffer> modelBuffers = renderStack.GetModelBuffers();
            for (const ModelBuffer& modelBuff : modelBuffers)
            {
                if (modelBuff.ModelAddr != -1)
                {
                    VulkanModel* model = m_models[modelBuff.ModelAddr];
                    if (model != nullptr)
                    {
                        const std::lock_guard mLock = std::lock_guard(model->GetLock());
                        model->Bind(commandBuffer);
                        const uint32_t indexCount = model->GetIndexCount();
                        for (uint32_t tAddr : modelBuff.TransformAddr)
                        {
                            shaderData->UpdateTransformBuffer(commandBuffer, tAddr, objectManager);

                            commandBuffer.drawIndexed(indexCount, 1, 0, 0, 0);
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
    m_runtimeManager->AttachThread();

    const CameraBuffer& camBuffer = m_cameraBuffers[a_camIndex];

    const vk::CommandBuffer commandBuffer = StartCommandBuffer(a_bufferIndex, a_index);

    VulkanRenderCommand& renderCommand = m_renderCommands.Push(VulkanRenderCommand(m_vulkanEngine, this, m_swapchain, commandBuffer, a_bufferIndex));

    void* lightSetupArgs[] =
    {
        &a_camIndex
    };

    m_lightSetupFunc->Exec(lightSetupArgs);

    renderCommand.SetCameraData(a_camIndex);

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
    m_runtimeManager->AttachThread();

    const vk::CommandBuffer commandBuffer = StartCommandBuffer(a_bufferIndex, a_index);

    VulkanRenderCommand& renderCommand = m_renderCommands.Push(VulkanRenderCommand(m_vulkanEngine, this, m_swapchain, commandBuffer, a_bufferIndex));

    renderCommand.SetCameraData(a_camIndex);

    void* camArgs[] = 
    { 
        &a_camIndex 
    };

    m_postProcessFunc->Exec(camArgs);

    renderCommand.Flush();

    commandBuffer.end();

    return commandBuffer;
}

void VulkanGraphicsEngine::DrawUIElement(vk::CommandBuffer a_commandBuffer, uint32_t a_addr, const CanvasBuffer& a_buffer, const glm::vec2& a_screenSize, uint32_t a_index)
{
    if (a_addr == -1)
    {
        return;
    }

    UIElement* element = UIControl::GetUIElement(a_addr);
    if (element != nullptr)
    {
        element->Update(m_vulkanEngine->GetRenderEngine());

        switch (element->GetType()) 
        {
        case UIElementType_Text:
        {
            const TextUIElement* text = (TextUIElement*)element;

            VulkanPipeline* pipeline = GetPipeline(-1, m_textUIPipelineAddr);
            ICARIAN_ASSERT(pipeline != nullptr);

            VulkanShaderData* data = pipeline->GetShaderData();
            ICARIAN_ASSERT(data != nullptr);

            ICARIAN_ASSERT_MSG_R(text->GetSamplerAddr() < m_textureSampler.Size(), "Invalid Sampler Address");
            const FlareBase::TextureSampler& sampler = m_textureSampler[text->GetSamplerAddr()];

            pipeline->Bind(a_index, a_commandBuffer);

            data->PushTexture(a_commandBuffer, 0, sampler, a_index);

            a_commandBuffer.draw(6, 1, 0, 0);

            break;
        }
        default:
        {
            ICARIAN_ASSERT_MSG(0, "Invalid UI Element Type");
        }
        }

        const uint32_t childCount = element->GetChildCount();
        const uint32_t* children = element->GetChildren();
        for (uint32_t i = 0; i < childCount; ++i)
        {
            DrawUIElement(a_commandBuffer, children[i], a_buffer, a_screenSize, a_index);
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
    
    const uint32_t camIndexSize = (uint32_t)camIndices.size();
    const uint32_t totalPoolSize = camIndexSize * DrawingPassCount + 1;
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

        FThreadJob<vk::CommandBuffer, DrawCallBind>* drawJob = new FThreadJob<vk::CommandBuffer, DrawCallBind>
        (   
            DrawCallBind(this, camIndex, poolIndex + 0, a_index, &VulkanGraphicsEngine::DrawPass), 
            JobPriority_EngineUrgent
        );
        FThreadJob<vk::CommandBuffer, DrawCallBind>* lightJob = new FThreadJob<vk::CommandBuffer, DrawCallBind>
        (
            DrawCallBind(this, camIndex, poolIndex + 1, a_index, &VulkanGraphicsEngine::LightPass),
            JobPriority_EngineUrgent
        );
        FThreadJob<vk::CommandBuffer, DrawCallBind>* postJob = new FThreadJob<vk::CommandBuffer, DrawCallBind>
        (
            DrawCallBind(this, camIndex, poolIndex + 2, a_index, &VulkanGraphicsEngine::PostPass),
            JobPriority_EngineUrgent
        );

        futures.emplace_back(drawJob->GetFuture());
        futures.emplace_back(lightJob->GetFuture());
        futures.emplace_back(postJob->GetFuture());

        // Quite pleased with myself got a 2x performance improvement by switching to a thread pool over async
        // And that is without using priorities either
        // Now here is hoping that it lasts when doing actual grunt work
        ThreadPool::PushJob(drawJob);
        ThreadPool::PushJob(lightJob);
        ThreadPool::PushJob(postJob);
    }
    
    // While we wait for other threads can draw UI on main render thread 
    vk::CommandBuffer buffer = StartCommandBuffer(camIndexSize * DrawingPassCount, a_index);
    const glm::ivec2 renderSize = m_swapchain->GetSize();
    const vk::RenderPassBeginInfo renderPassInfo = vk::RenderPassBeginInfo
    (
        m_swapchain->GetRenderPassNoClear(),
        m_swapchain->GetFramebuffer(m_vulkanEngine->GetImageIndex()),
        vk::Rect2D({ 0, 0 }, { (uint32_t)renderSize.x, (uint32_t)renderSize.y }),
        0,
        nullptr
    );

    buffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

    std::vector<CanvasBuffer> canvasBuffers = UIControl::GetCanvases();
    for (const CanvasBuffer& canvas : canvasBuffers)
    {
        if (!(canvas.Flags & 0b1 << canvas.DestroyedBit))
        {
            const vk::Rect2D scissor = vk::Rect2D({ 0, 0 }, { (uint32_t)renderSize.x, (uint32_t)renderSize.y });
            buffer.setScissor(0, 1, &scissor);

            const vk::Viewport viewport = vk::Viewport(0.0f, 0.0f, renderSize.x, renderSize.y, 0.0f, 1.0f);
            buffer.setViewport(0, 1, &viewport);

            for (uint32_t i = 0; i < canvas.ChildElementCount; ++i)
            {
                DrawUIElement(buffer, canvas.ChildElements[i], canvas, renderSize, a_index);
            }
        }
    }

    buffer.endRenderPass();

    buffer.end();

    std::vector<vk::CommandBuffer> cmdBuffers;

    for (std::future<vk::CommandBuffer>& f : futures)
    {
        f.wait();
        vk::CommandBuffer buffer = f.get();
        if (buffer != vk::CommandBuffer(nullptr))
        {
            cmdBuffers.emplace_back(buffer);
        }
    }

    cmdBuffers.emplace_back(buffer);

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
    
    VulkanTexture* texture = m_textures[a_addr];
    ICARIAN_DEFER_del(texture);

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
    ICARIAN_DEFER(sampler, if (sampler.Data != nullptr) { delete (VulkanTextureSampler*)sampler.Data; });

    m_textureSampler.LockSet(a_addr, nullSampler);
}

Font* VulkanGraphicsEngine::GetFont(uint32_t a_addr)
{
    ICARIAN_ASSERT_MSG(a_addr < m_fonts.Size(), "GetFont out of bounds");

    return m_fonts[a_addr];
}