// Icarian Engine - C# Game Engine
// 
// License at end of file.

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/VulkanGraphicsParticle2D.h"

#include "Core/Bitfield.h"
#include "Core/IcarianDefer.h"
#include "Core/ShaderBuffers.h"
#include "Rendering/Vulkan/VulkanComputeEngine.h"
#include "Rendering/Vulkan/VulkanGraphicsEngine.h"
#include "Rendering/Vulkan/VulkanParticleShaderGenerator.h"
#include "Rendering/Vulkan/VulkanPipeline.h"
#include "Rendering/Vulkan/VulkanPushPool.h"
#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"
#include "Rendering/Vulkan/VulkanShaderData.h"

void VulkanGraphicsParticle2D::Build(const ComputeParticleBuffer& a_buffer)
{
    m_inputs.Clear();

    uint16_t slot = 0;

    Array<VertexInputAttribute> vertexInputs;

    const std::string vShaderStr = VulkanParticleShaderGenerator::GenerateVertexShader(a_buffer, &slot, &m_inputs, &vertexInputs);
    const uint32_t vertexShader = m_gEngine->GenerateFVertexShader(vShaderStr);

    const std::string pShaderStr = VulkanParticleShaderGenerator::GeneratePixelShader(a_buffer, &slot, &m_inputs);
    const uint32_t pixelShader = m_gEngine->GenerateFPixelShader(pShaderStr);

    const uint32_t inputCount = m_inputs.Size();
    const uint32_t vertexInputCount = vertexInputs.Size();

    RenderProgram program = 
    {
        .VertexShader = vertexShader,
        .PixelShader = pixelShader,
        .ShadowVertexShader = uint32_t(-1),
        .PrimitiveMode = PrimitiveMode_Triangles,
        .Flags = 0b1 << RenderProgram::DestroyFlag
    };

    if (vertexInputCount > 0)
    {
        program.VertexStride = sizeof(IcarianCore::ShaderParticleBuffer);
        program.VertexInputCount = vertexInputCount;
        program.VertexAttributes = new VertexInputAttribute[vertexInputCount];

        for (uint32_t i = 0; i < vertexInputCount; ++i)
        {
            program.VertexAttributes[i] = vertexInputs[i];
        }
    }

    m_renderProgramAddr = m_gEngine->GenerateRenderProgram(program);
}
void VulkanGraphicsParticle2D::Destroy()
{
    if (m_renderProgramAddr != -1)
    {
        const RenderProgram program = m_gEngine->GetRenderProgram(m_renderProgramAddr);
        IDEFER(
        if (program.VertexAttributes != nullptr) 
        {
            delete[] program.VertexAttributes;
        });

        m_gEngine->DestroyRenderProgram(m_renderProgramAddr);

        m_renderProgramAddr = -1;
    }
}

VulkanGraphicsParticle2D::VulkanGraphicsParticle2D(VulkanRenderEngineBackend* a_backend, VulkanComputeEngine* a_cEngine, VulkanGraphicsEngine* a_gEngine, uint32_t a_computeBufferAddr)
{
    m_backend = a_backend;
    m_cEngine = a_cEngine;
    m_gEngine = a_gEngine;

    m_computeBufferAddr = a_computeBufferAddr;
    m_renderProgramAddr = -1;
}
VulkanGraphicsParticle2D::~VulkanGraphicsParticle2D()
{
    const ThreadGuard g = ThreadGuard(m_lock);

    Destroy();
}

void VulkanGraphicsParticle2D::Update(uint32_t a_index, uint32_t a_bufferIndex, uint32_t a_renderLayer, vk::CommandBuffer a_commandBuffer, uint32_t a_renderTextureAddr)
{
    ComputeParticleBuffer buffer = m_cEngine->GetParticleBuffer(m_computeBufferAddr);
    IVERIFY(buffer.DisplayMode == ParticleDisplayMode_Quad);

    if ((a_renderLayer & buffer.RenderLayer) == 0)
    {
        return;
    }
    
    if (!IISBITSET(buffer.Flags, ComputeParticleBuffer::PlayingBit))
    {
        return;
    }

    const ThreadGuard g = ThreadGuard(m_lock);

    if (IISBITSET(buffer.Flags, ComputeParticleBuffer::GraphicsRefreshBit))
    {
        Destroy();

        ICLEARBIT(buffer.Flags, ComputeParticleBuffer::GraphicsRefreshBit);

        m_cEngine->SetParticleBuffer(m_computeBufferAddr, buffer);
    }

    const bool valid = m_renderProgramAddr != -1;
    if (!valid)
    {
        Build(buffer);
    }

    vk::Buffer computeParticleBuffer = m_cEngine->GetParticleBufferData(m_computeBufferAddr);

    VulkanPipeline* pipeline = m_gEngine->GetPipeline(a_renderTextureAddr, m_renderProgramAddr);
    const VulkanShaderData* data = pipeline->GetShaderData();

    for (const ShaderBufferInput& input : m_inputs)
    {
        switch (input.BufferType)
        {
        case ShaderBufferType_PModelBuffer:
        {
            const glm::mat4 transform = ObjectManager::GetGlobalMatrix(buffer.TransformAddr);

            data->UpdateTransformBuffer(a_commandBuffer, transform);

            break;
        }
        case ShaderBufferType_CameraBuffer:
        {
            const VulkanUniformBuffer* camBuffer = m_gEngine->GetCameraUniformBuffer(a_bufferIndex);

            data->PushUniformBuffer(a_commandBuffer, input.Slot, camBuffer, a_index);

            break;
        }
        case ShaderBufferType_TimeBuffer:
        {
            const VulkanUniformBuffer* timeBuffer = m_gEngine->GetTimeUniformBuffer();

            data->PushUniformBuffer(a_commandBuffer, input.Slot, timeBuffer, a_index);

            break;
        }
        case ShaderBufferType_SSParticleBuffer:
        {
            data->PushShaderStorageObject(a_commandBuffer, input.Slot, computeParticleBuffer, a_index);

            break;
        }
        default:
        {
            IERROR("Invalid particle graphics input");

            break;
        }
        }
    }

    pipeline->Bind(a_index, a_commandBuffer);

    a_commandBuffer.draw(buffer.MaxParticles * 6, 1, 0, 0);
}

#endif

// MIT License
// 
// Copyright (c) 2024 River Govers
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.