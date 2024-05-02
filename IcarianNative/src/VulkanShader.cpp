#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/VulkanShader.h"

VulkanShader::VulkanShader(VulkanRenderEngineBackend* a_engine, const ShaderBufferInput* a_inputs, uint32_t a_inputCount)
{
    m_module = nullptr;
    m_engine = a_engine;
    m_inputCount = a_inputCount;

    m_inputs = new ShaderBufferInput[m_inputCount];
    for (uint32_t i = 0; i < m_inputCount; ++i)
    {
        m_inputs[i] = a_inputs[i];
    }
}
VulkanShader::~VulkanShader()
{
    delete[] m_inputs;
}

#endif