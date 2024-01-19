#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/VulkanComputeEngine.h"

#include "Flare/IcarianAssert.h"

VulkanComputeEngine::VulkanComputeEngine(VulkanRenderEngineBackend* a_engine)
{
    m_engine = a_engine;
}
VulkanComputeEngine::~VulkanComputeEngine()
{

}

vk::CommandBuffer VulkanComputeEngine::Update()
{
    return nullptr;
}

VulkanComputeShader* VulkanComputeEngine::GetShader(uint32_t a_shaderAddr)
{
    ICARIAN_ASSERT_MSG(a_shaderAddr < m_shaders.Size(), "Compute Shader address out of range");
    ICARIAN_ASSERT_MSG(m_shaders.Exists(a_shaderAddr), "Compute Shader does not exist");

    return m_shaders[a_shaderAddr];
}

#endif