// Icarian Engine - C# Game Engine
// 
// License at end of file.

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/VulkanGraphicsParticle2D.h"

#include "Core/Bitfield.h"
#include "Rendering/Vulkan/VulkanComputeEngine.h"
#include "Rendering/Vulkan/VulkanGraphicsEngine.h"
#include "Rendering/Vulkan/VulkanParticleShaderGenerator.h"
#include "Rendering/Vulkan/VulkanPipeline.h"
#include "Rendering/Vulkan/VulkanPushPool.h"
#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"
#include "Rendering/Vulkan/VulkanShaderData.h"
#include "Shaders.h"

void VulkanGraphicsParticle2D::Build(const ComputeParticleBuffer& a_buffer)
{
    m_inputs.Clear();

    uint16_t slot = 0;

    const uint32_t taskShader = m_gEngine->GenerateFTaskShader(ParticleTaskShader);

    const std::string mShaderStr = VulkanParticleShaderGenerator::GenerateMeshShader(a_buffer, &slot, &m_inputs);
    const uint32_t meshShader = m_gEngine->GenerateFMeshShader(mShaderStr);

    const std::string pShaderStr = VulkanParticleShaderGenerator::GeneratePixelShader(a_buffer, &slot, &m_inputs);
    const uint32_t pixelShader = m_gEngine->GenerateFPixelShader(pShaderStr);

    const RenderProgram program = 
    {
        .VertexShader = meshShader,
        .PixelShader = pixelShader,
        .ExtraShader = taskShader,
        .ShadowVertexShader = uint32_t(-1),
        .PrimitiveMode = PrimitiveMode_Triangles,
        .MaterialMode = MaterialMode_BaseMesh,
        .Flags = 0b1 << RenderProgram::DestroyFlag
    };

    m_renderProgramAddr = m_gEngine->GenerateRenderProgram(program);
}
void VulkanGraphicsParticle2D::Destroy()
{
    if (m_renderProgramAddr != uint32_t(-1))
    {
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

    const bool valid = m_renderProgramAddr != uint32_t(-1);
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
            data->PushShaderStorageObject(a_commandBuffer, input.Slot, computeParticleBuffer, 0, a_index);

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

    // Well that was concerning it just letting me do a normal draw call and the Validation layer did not care
    // The only thing was that nothing was rendering
    a_commandBuffer.drawMeshTasksEXT((uint32_t)glm::ceil(buffer.MaxParticles / 256.0), 1, 1);
    // a_commandBuffer.drawMeshTasksEXT(1, 1, 1);
}

#endif

// MIT License
// 
// Copyright (c) 2025 River Govers
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
