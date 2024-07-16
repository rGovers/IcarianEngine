#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN
#include "Rendering/Vulkan/VulkanComputeEngine.h"

#include "Core/IcarianAssert.h"
#include "Core/IcarianDefer.h"
#include "Core/ShaderBuffers.h"
#include "Logger.h"
#include "Rendering/Vulkan/VulkanComputeEngineBindings.h"
#include "Rendering/Vulkan/VulkanComputeLayout.h"
#include "Rendering/Vulkan/VulkanComputeParticle.h"
#include "Rendering/Vulkan/VulkanComputePipeline.h"
#include "Rendering/Vulkan/VulkanComputeShader.h"
#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"
#include "Rendering/Vulkan/VulkanUniformBuffer.h"
#include "Trace.h"

VulkanComputeEngine::VulkanComputeEngine(VulkanRenderEngineBackend* a_engine)
{
    m_engine = a_engine;

    const vk::Device device = m_engine->GetLogicalDevice();

    const vk::CommandPoolCreateInfo poolInfo = vk::CommandPoolCreateInfo
    (
        vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        m_engine->GetComputeQueueIndex()
    );  

    for (uint32_t i = 0; i < VulkanFlightPoolSize; ++i)
    {
        VKRESERRMSG(device.createCommandPool(&poolInfo, nullptr, &m_pools[i]), "Failed to create Compute Command Pool");

        const vk::CommandBufferAllocateInfo commandBufferInfo = vk::CommandBufferAllocateInfo
        (
            m_pools[i],
            vk::CommandBufferLevel::ePrimary,
            1
        );

        VKRESERRMSG(device.allocateCommandBuffers(&commandBufferInfo, &m_buffers[i]), "Failed to create Compute Command Buffer");
    }

    m_timeUniform = new VulkanUniformBuffer(m_engine, sizeof(IcarianCore::ShaderTimeBuffer));

    m_bindings = new VulkanComputeEngineBindings(this);
}
VulkanComputeEngine::~VulkanComputeEngine()
{
    delete m_bindings;

    delete m_timeUniform;

    const vk::Device device = m_engine->GetLogicalDevice();

    TRACE("Deleteing Compute Command Pool");
    for (uint32_t i = 0; i < VulkanFlightPoolSize; ++i)
    {
        device.destroyCommandPool(m_pools[i]);
    }

    TRACE("Checking if compute particle system was deleted");
    for (uint32_t i = 0; i < m_particleBuffers.Size(); ++i)
    {
        if (m_particleBuffers.Exists(i))
        {
            Logger::Warning("Compute Particle was not destroyed");

            VulkanComputeParticle* data = (VulkanComputeParticle*)m_particleBuffers[i].Data;
            if (data != nullptr)
            {
                delete data;
            }
        }
    }

    TRACE("Checking if compute shaders where deleted");
    for (uint32_t i = 0; i < m_shaders.Size(); ++i)
    {
        if (m_shaders.Exists(i))
        {
            Logger::Warning("Compute Shader was not destroyed");

            delete m_shaders[i];
        }
    }

    TRACE("Checking if compute pipelines where deleted");
    for (uint32_t i = 0; i < m_pipelines.Size(); ++i)
    {
        if (m_pipelines.Exists(i))
        {
            Logger::Warning("Compute Pipeline was not destroyed");

            delete m_pipelines[i];
        }
    }

    TRACE("Checking if compute layouts where deleted");
    for (uint32_t i = 0; i < m_layouts.Size(); ++i)
    {
        if (m_layouts.Exists(i))
        {
            Logger::Warning("Compute Layout was not destroyed");

            delete m_layouts[i];
        }
    }    
}

VulkanCommandBuffer VulkanComputeEngine::Update(double a_delta, double a_time, uint32_t a_index)
{
    const vk::Device device = m_engine->GetLogicalDevice();

    device.resetCommandPool(m_pools[a_index]);

    const IcarianCore::ShaderTimeBuffer timeBuffer =
    {
        .Time = glm::vec2((float)a_delta, (float)a_time)
    };
    
    m_timeUniform->SetData(a_index, &timeBuffer);

    const vk::CommandBuffer cmdBuffer = m_buffers[a_index];

    constexpr vk::CommandBufferBeginInfo BeginInfo;
    cmdBuffer.begin(BeginInfo);
    IDEFER(cmdBuffer.end());

    const Array<ComputeParticleBuffer> particleBuffers = m_particleBuffers.ToActiveArray();
    for (const ComputeParticleBuffer& buffer : particleBuffers)
    {
        VulkanComputeParticle* pSys = (VulkanComputeParticle*)buffer.Data;
        if (pSys != nullptr)
        {
            pSys->Update(cmdBuffer, a_index);
        }
    }

    return VulkanCommandBuffer(cmdBuffer, VulkanCommandBufferType_Compute);
}

ComputeParticleBuffer VulkanComputeEngine::GetParticleBuffer(uint32_t a_addr)
{
    IVERIFY(a_addr < m_particleBuffers.Size());
    IVERIFY(m_particleBuffers.Exists(a_addr));

    return m_particleBuffers[a_addr];
}
void VulkanComputeEngine::SetParticleBuffer(uint32_t a_addr, const ComputeParticleBuffer& a_buffer)
{
    IVERIFY(a_addr < m_particleBuffers.Size());
    IVERIFY(m_particleBuffers.Exists(a_addr));

    m_particleBuffers.LockSet(a_addr, a_buffer);
}

vk::Buffer VulkanComputeEngine::GetParticleBufferData(uint32_t a_addr)
{
    IVERIFY(a_addr < m_particleBuffers.Size());
    IVERIFY(m_particleBuffers.Exists(a_addr));

    const ComputeParticleBuffer buffer = m_particleBuffers[a_addr];
    VulkanComputeParticle* data = (VulkanComputeParticle*)buffer.Data; 

    return data->GetComputeBuffer();
}

uint32_t VulkanComputeEngine::GenerateComputeFShader(const std::string_view& a_str)
{
    VulkanComputeShader* shader = VulkanComputeShader::CreateFromFShader(m_engine, a_str);

    return m_shaders.PushVal(shader);
}
void VulkanComputeEngine::DestroyComputeShader(uint32_t a_addr)
{
    IVERIFY(a_addr < m_shaders.Size());
    IVERIFY(m_shaders.Exists(a_addr));

    const VulkanComputeShader* shader = m_shaders[a_addr];
    IDEFER(delete shader);

    m_shaders.Erase(a_addr);
}
VulkanComputeShader* VulkanComputeEngine::GetComputeShader(uint32_t a_addr)
{
    IVERIFY(a_addr < m_shaders.Size());
    IVERIFY(m_shaders.Exists(a_addr));

    return m_shaders[a_addr];
}

uint32_t VulkanComputeEngine::GenerateComputePipelineLayout(const ShaderBufferInput* a_inputs, uint32_t a_count)
{
    VulkanComputeLayout* layout = new VulkanComputeLayout(m_engine, a_inputs, a_count);

    return m_layouts.PushVal(layout);
}
void VulkanComputeEngine::DestroyComputePipelineLayout(uint32_t a_addr)
{
    IVERIFY(a_addr < m_layouts.Size());
    IVERIFY(m_layouts.Exists(a_addr));

    const VulkanComputeLayout* layout = m_layouts[a_addr];
    IDEFER(delete layout);

    m_layouts.Erase(a_addr);
}
VulkanComputeLayout* VulkanComputeEngine::GetComputePipelineLayout(uint32_t a_addr)
{
    IVERIFY(a_addr < m_layouts.Size());
    IVERIFY(m_layouts.Exists(a_addr));

    return m_layouts[a_addr];
}

uint32_t VulkanComputeEngine::GenerateComputePipeline(uint32_t a_shaderAddr, uint32_t a_layoutAddr)
{
    VulkanComputePipeline* pipeline = VulkanComputePipeline::CreatePipeline(this, a_layoutAddr, a_shaderAddr);
    IVERIFY(pipeline != nullptr);

    return m_pipelines.PushVal(pipeline);
}
void VulkanComputeEngine::DestroyComputePipeline(uint32_t a_addr)
{
    IVERIFY(a_addr < m_pipelines.Size());
    IVERIFY(m_pipelines.Exists(a_addr));

    const VulkanComputePipeline* pipeline = m_pipelines[a_addr];
    IDEFER(delete pipeline);

    m_pipelines.Erase(a_addr);
}
VulkanComputePipeline* VulkanComputeEngine::GetComputePipeline(uint32_t a_addr)
{
    IVERIFY(a_addr < m_pipelines.Size());
    IVERIFY(m_pipelines.Exists(a_addr));

    return m_pipelines[a_addr];
}

#endif