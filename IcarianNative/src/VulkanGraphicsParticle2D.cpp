#include "Rendering/Vulkan/VulkanPushPool.h"
#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/VulkanGraphicsParticle2D.h"

#include "Core/IcarianAssert.h"
#include "Core/IcarianDefer.h"
#include "Rendering/ShaderBuffers.h"
#include "Rendering/Vulkan/VulkanComputeEngine.h"
#include "Rendering/Vulkan/VulkanGraphicsEngine.h"
#include "Rendering/Vulkan/VulkanParticleShaderGenerator.h"
#include "Rendering/Vulkan/VulkanPipeline.h"
#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"
#include "Rendering/Vulkan/VulkanShaderData.h"

VulkanGraphicsParticle2D::VulkanGraphicsParticle2D(VulkanRenderEngineBackend* a_backend, VulkanComputeEngine* a_cEngine, VulkanGraphicsEngine* a_gEngine, uint32_t a_computeBufferAddr)
{
    m_backend = a_backend;
    m_cEngine = a_cEngine;
    m_gEngine = a_gEngine;

    m_computeBufferAddr = a_computeBufferAddr;

    const ComputeParticleBuffer buffer = m_cEngine->GetParticleBuffer(m_computeBufferAddr);

    uint16_t set;
    uint16_t slot;

    std::vector<VertexInputAttribute> vertexInputs;

    const std::string vShaderStr = VulkanParticleShaderGenerator::GenerateVertexShader(buffer, &set, &slot, &m_inputs, &vertexInputs);
    const uint32_t vertexShader = m_gEngine->GenerateFVertexShader(vShaderStr);

    const std::string pShaderStr = VulkanParticleShaderGenerator::GeneratePixelShader(buffer, &set, &slot, &m_inputs);
    const uint32_t pixelShader = m_gEngine->GenerateFPixelShader(pShaderStr);

    const uint32_t inputCount = (uint32_t)m_inputs.size();
    const uint32_t vertexInputCount = (uint32_t)vertexInputs.size();

    RenderProgram program;
    memset(&program, 0, sizeof(program));
    program.VertexShader = vertexShader;
    program.PixelShader = pixelShader;
    program.ShadowVertexShader = -1;

    program.ShaderBufferInputCount = inputCount;
    program.ShaderBufferInputs = new ShaderBufferInput[inputCount];
    for (uint32_t i = 0; i < inputCount; ++i)
    {
        program.ShaderBufferInputs[i] = m_inputs[i];
    }

    if (vertexInputCount > 0)
    {
        program.VertexStride = sizeof(ParticleShaderBuffer);
        program.VertexInputCount = vertexInputCount;
        program.VertexAttributes = new VertexInputAttribute[vertexInputCount];
        for (uint32_t i = 0; i < vertexInputCount; ++i)
        {
            program.VertexAttributes[i] = vertexInputs[i];
        }
    }

    program.PrimitiveMode = PrimitiveMode_Triangles;
    program.Flags |= 0b1 << RenderProgram::DestroyFlag;

    m_renderProgramAddr = m_gEngine->GenerateRenderProgram(program);
}
VulkanGraphicsParticle2D::~VulkanGraphicsParticle2D()
{
    const RenderProgram program = m_gEngine->GetRenderProgram(m_renderProgramAddr);
    IDEFER(
    {
        if (program.VertexAttributes != nullptr) 
        {
            delete[] program.VertexAttributes;
        }

        delete[] program.ShaderBufferInputs;
    });

    m_gEngine->DestroyRenderProgram(m_renderProgramAddr);
}

void VulkanGraphicsParticle2D::Update(uint32_t a_index, uint32_t a_bufferIndex, vk::CommandBuffer a_commandBuffer, uint32_t a_renderTextureAddr)
{
    const ComputeParticleBuffer buffer = m_cEngine->GetParticleBuffer(m_computeBufferAddr);
    vk::Buffer computeParticleBuffer = m_cEngine->GetParticleBufferData(m_computeBufferAddr);

    VulkanPipeline* pipeline = m_gEngine->GetPipeline(a_renderTextureAddr, m_renderProgramAddr);
    const VulkanShaderData* data = pipeline->GetShaderData();

    for (const ShaderBufferInput& input : m_inputs)
    {
        switch (input.BufferType)
        {
        case ShaderBufferType_CameraBuffer:
        {
            const VulkanUniformBuffer* camBuffer = m_gEngine->GetCameraUniformBuffer(a_bufferIndex);

            data->PushUniformBuffer(a_commandBuffer, input.Set, camBuffer, a_index);

            break;
        }
        case ShaderBufferType_TimeBuffer:
        {
            const VulkanUniformBuffer* timeBuffer = m_gEngine->GetTimeUniformBuffer();

            data->PushUniformBuffer(a_commandBuffer, input.Set, timeBuffer, a_index);

            break;
        }
        case ShaderBufferType_SSParticleBuffer:
        {
            data->PushShaderStorageObject(a_commandBuffer, input.Set, computeParticleBuffer, a_index);

            break;
        }
        default:
        {
            ICARIAN_ASSERT(0);
        }
        }
    }

    pipeline->Bind(a_index, a_commandBuffer);

    if (buffer.DisplayMode == ParticleDisplayMode_Point)
    {
        constexpr vk::DeviceSize Offsets[] = { 0 };

        a_commandBuffer.bindVertexBuffers(0, 1, &computeParticleBuffer, Offsets);
    }

    a_commandBuffer.draw(buffer.MaxParticles, 1, 0, 0);
}

#endif