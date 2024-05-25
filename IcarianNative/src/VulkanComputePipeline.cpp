#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/VulkanComputePipeline.h"

#include "Rendering/Vulkan/VulkanComputeEngine.h"
#include "Rendering/Vulkan/VulkanComputeLayout.h"
#include "Rendering/Vulkan/VulkanComputeShader.h"
#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"
#include "Trace.h"

class VulkanComputePipelineDeletionObject : public VulkanDeletionObject
{
private:
    VulkanRenderEngineBackend* m_engine;

    vk::Pipeline               m_pipeline;

protected:

public:
    VulkanComputePipelineDeletionObject(VulkanRenderEngineBackend* a_engine, const vk::Pipeline& a_pipeline)
    {
        m_engine = a_engine;

        m_pipeline = a_pipeline;
    }

    virtual void Destroy() 
    {
        TRACE("Destroying Compute Pipeline");
        const vk::Device device = m_engine->GetLogicalDevice();

        device.destroyPipeline(m_pipeline);
    }
};

VulkanComputePipeline::VulkanComputePipeline(VulkanComputeEngine* a_engine, vk::Pipeline a_pipeline)
{
    m_engine = a_engine;

    m_pipeline = a_pipeline;
}
VulkanComputePipeline::~VulkanComputePipeline()
{
    VulkanRenderEngineBackend* backend = m_engine->GetRenderEngineBackend();

    TRACE("Queueing Compute Pipeline for deletion");
    backend->PushDeletionObject(new VulkanComputePipelineDeletionObject(backend, m_pipeline));
}

VulkanComputePipeline* VulkanComputePipeline::CreatePipeline(VulkanComputeEngine* a_engine, uint32_t a_layoutAddr, uint32_t a_shaderAddr)
{
    TRACE("Creating Vulkan Compute Pipeline");
    VulkanRenderEngineBackend* backend = a_engine->GetRenderEngineBackend();

    const vk::Device device = backend->GetLogicalDevice();

    VulkanComputeShader* shader = a_engine->GetComputeShader(a_shaderAddr);
    VulkanComputeLayout* layout = a_engine->GetComputePipelineLayout(a_layoutAddr);

    vk::PipelineShaderStageCreateInfo shaderStageInfo = vk::PipelineShaderStageCreateInfo
    (
        { },
        vk::ShaderStageFlagBits::eCompute,
        shader->GetShaderModule(),
        "main"
    );

    vk::ComputePipelineCreateInfo pipelineInfo = vk::ComputePipelineCreateInfo
    (
        { },
        shaderStageInfo,
        layout->GetLayout()
    );

    vk::Pipeline pipe;
    if (device.createComputePipelines(nullptr, 1, &pipelineInfo, nullptr, &pipe) == vk::Result::eSuccess)
    {
        return new VulkanComputePipeline(a_engine, pipe);
    }

    return nullptr;
}

#endif