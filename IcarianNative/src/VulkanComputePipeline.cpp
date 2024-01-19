#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/VulkanComputePipeline.h"

#include "Flare/IcarianAssert.h"
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

VulkanComputePipeline::VulkanComputePipeline(VulkanRenderEngineBackend* a_engine, VulkanComputeEngine* a_cEngine, uint32_t a_shaderAddr)
{
    m_engine = a_engine;
    m_cEngine = a_cEngine;

    m_shaderAddr = a_shaderAddr;
}
VulkanComputePipeline::~VulkanComputePipeline()
{
    TRACE("Queueing Compute Pipeline for deletion");
    m_engine->PushDeletionObject(new VulkanComputePipelineDeletionObject(m_engine, m_pipeline));
}

VulkanComputePipeline* VulkanComputePipeline::CreatePipeline(VulkanRenderEngineBackend* a_engine, VulkanComputeEngine* a_cEngine, VulkanComputeLayout* a_layout, uint32_t a_shaderAddr)
{
    TRACE("Creating Vulkan Compute Pipeline");
    VulkanComputePipeline* pipeline = new VulkanComputePipeline(a_engine, a_cEngine, a_shaderAddr);

    const vk::Device device = a_engine->GetLogicalDevice();

    VulkanComputeShader* shader = a_cEngine->GetShader(a_shaderAddr);

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
        a_layout->GetLayout()
    );

    // What is love? Vulkan don't hurt me, don't hurt me, no more.
    ICARIAN_ASSERT_MSG_R(device.createComputePipelines(nullptr, 1, &pipelineInfo, nullptr, &pipeline->m_pipeline) == vk::Result::eSuccess, "Failed to create Vulkan Compute Pipeline"); 

    return pipeline;
}

#endif