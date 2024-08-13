// Icarian Engine - C# Game Engine
// 
// License at end of file.

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/VulkanComputeParticle.h"

#include <cstring>

#include "Core/Bitfield.h"
#include "Core/IcarianAssert.h"
#include "Core/IcarianDefer.h"
#include "Core/ShaderBuffers.h"
#include "Random.h"
#include "Rendering/Vulkan/VulkanComputeEngine.h"
#include "Rendering/Vulkan/VulkanComputeLayout.h"
#include "Rendering/Vulkan/VulkanComputePipeline.h"
#include "Rendering/Vulkan/VulkanParticleShaderGenerator.h"
#include "Rendering/Vulkan/VulkanPushPool.h"
#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"
#include "Rendering/Vulkan/VulkanUniformBuffer.h"
#include "Trace.h"

class VulkanParticleBufferDeletionObject : public VulkanDeletionObject
{
private:
    VulkanRenderEngineBackend* m_engine;

    VmaAllocation              m_allocation;
    vk::Buffer                 m_buffer;

protected:

public:
    VulkanParticleBufferDeletionObject(VulkanRenderEngineBackend* a_engine, vk::Buffer a_buffer, VmaAllocation a_allocation)
    {
        m_engine = a_engine;

        m_buffer = a_buffer;
        m_allocation = a_allocation;
    }
    virtual ~VulkanParticleBufferDeletionObject()
    {

    }

    virtual void Destroy()
    {
        const VmaAllocator allocator = m_engine->GetAllocator();

        vmaDestroyBuffer(allocator, m_buffer, m_allocation);
    }
};

VulkanComputeParticle::VulkanComputeParticle(VulkanComputeEngine* a_engine, uint32_t a_particleBufferAddr)
{
    m_engine = a_engine;

    m_bufferIndex = 0;

    m_computeShader = -1;
    m_computeLayout = -1;
    m_computePipeline = -1;
    m_particleBufferAddr = a_particleBufferAddr;

    for (uint32_t i = 0; i < MaxParticleBuffers; ++i)
    {
        m_particleBuffers[i] = nullptr;
        m_allocations[i] = NULL;
    }
}
VulkanComputeParticle::~VulkanComputeParticle()
{
    Clear();
}

void VulkanComputeParticle::Clear()
{
    if (m_particleBuffers[0] != nullptr)
    {
        VulkanRenderEngineBackend* backend = m_engine->GetRenderEngineBackend();

        TRACE("Queueing Particle Buffers for deletion");
        for (uint32_t i = 0; i < MaxParticleBuffers; ++i)
        {
            backend->PushDeletionObject(new VulkanParticleBufferDeletionObject(backend, m_particleBuffers[i], m_allocations[i]));

            m_particleBuffers[i] = nullptr;
            m_allocations[i] = NULL;
        }
    }

    if (m_computeShader != -1) 
    {
        m_engine->DestroyComputeShader(m_computeShader);
        m_computeShader = -1;
    }

    if (m_computeLayout != -1)
    {
        m_engine->DestroyComputePipelineLayout(m_computeLayout);
        m_computeLayout = -1;
    }

    if (m_computePipeline != -1)
    {
        m_engine->DestroyComputePipeline(m_computePipeline);
        m_computePipeline = -1;
    }

    m_bufferIndex = 0;
}
void VulkanComputeParticle::Rebuild(ComputeParticleBuffer* a_buffer)
{
    Clear();

    IDEFER(ICLEARBIT(a_buffer->Flags, ComputeParticleBuffer::RefreshBit));

    Array<ShaderBufferInput> inputs;
    const std::string shaderStr = VulkanParticleShaderGenerator::GenerateComputeShader(*a_buffer, &inputs);

    m_computeShader = m_engine->GenerateComputeFShader(shaderStr);
    m_computeLayout = m_engine->GenerateComputePipelineLayout(inputs.Data(), inputs.Size());
    m_computePipeline = m_engine->GenerateComputePipeline(m_computeShader, m_computeLayout);

    VulkanRenderEngineBackend* backend = m_engine->GetRenderEngineBackend();
    const VmaAllocator allocator = backend->GetAllocator();

    const uint64_t particleBufferSize = sizeof(IcarianCore::ShaderParticleBuffer) * a_buffer->MaxParticles + 16;

    const VkBufferCreateInfo createInfo = 
    { 
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = particleBufferSize,
        .usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    TLockObj<vk::CommandBuffer, SpinLock>* buffer = backend->BeginSingleCommand();
    IDEFER(backend->EndSingleCommand(buffer));
    const vk::CommandBuffer cmdBuffer = buffer->Get();

    const int32_t value = (int32_t)a_buffer->MaxParticles;
    constexpr vk::DeviceSize CountSize = sizeof(value);
    constexpr vk::DeviceSize ValueSize = 16 - CountSize;

    uint32_t startIndex = 0;

    if (IISBITSET(a_buffer->Flags, ComputeParticleBuffer::BurstBit))
    {
        IDEFER(++startIndex);

        IcarianCore::ShaderParticleBuffer* particles = new IcarianCore::ShaderParticleBuffer[a_buffer->MaxParticles];
        IDEFER(delete[] particles);

        const glm::vec4& colour = a_buffer->Colour;

        switch (a_buffer->EmitterType) 
        {
        case ParticleEmitterType_Point:
        {
            for (uint32_t i = 0; i < a_buffer->MaxParticles; ++i)
            {
                const glm::vec3 vel = glm::vec3
                (
                    Random::Range(-1.0f, 1.0f),
                    Random::Range(-1.0f, 1.0f),
                    Random::Range(-1.0f, 1.0f)
                );

                particles[i] = 
                {
                    .Position = glm::vec4(0.0f, 0.0f, 0.0f, 5.0f),
                    .Velocity = vel,
                    .Color = colour,
                };
            }

            break;
        }
        default:
        {
            IERROR("Invalid emitter type");

            break;
        }
        }

        const VmaAllocationCreateInfo allocCreateInfo = 
        { 
            .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
            .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE
        };

        VkBuffer buffer;
        VmaAllocationInfo bufferInfo;
        VKRESERRMSG(vmaCreateBuffer(allocator, &createInfo, &allocCreateInfo, &buffer, &m_allocations[0], &bufferInfo), "Failed to create particle buffer");
#ifdef DEBUG
        vmaSetAllocationName(allocator, m_allocations[0], "Particle Buffer");
#endif
        m_particleBuffers[0] = buffer;

        VkMemoryPropertyFlags flags;
        vmaGetAllocationMemoryProperties(allocator, m_allocations[0], &flags);

        const bool cpuCanWrite = (flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0;
        if (cpuCanWrite)
        {
            IDEFER(VKRESERR(vmaFlushAllocation(allocator, m_allocations[0], 0, (VkDeviceSize)particleBufferSize)));

            memcpy(bufferInfo.pMappedData, &value, (size_t)CountSize);
            memset((uint8_t*)bufferInfo.pMappedData + CountSize, 0, ValueSize);
            memcpy((uint8_t*)bufferInfo.pMappedData + 16, particles, (size_t)particleBufferSize - 16);

            const vk::BufferMemoryBarrier barrier = vk::BufferMemoryBarrier
            (
                vk::AccessFlagBits::eHostWrite,
                vk::AccessFlagBits::eShaderRead,
                VK_QUEUE_FAMILY_IGNORED,
                VK_QUEUE_FAMILY_IGNORED,
                m_particleBuffers[0],
                0,
                VK_WHOLE_SIZE
            );

            cmdBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eHost, vk::PipelineStageFlagBits::eComputeShader, { }, 0, nullptr, 1, &barrier, 0, nullptr);
        }
        else
        {
            const VkBufferCreateInfo sCreateInfo = 
            {
                .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                .size = particleBufferSize,
                .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                .sharingMode = VK_SHARING_MODE_EXCLUSIVE
            };

            const VmaAllocationCreateInfo sAllocInfo = 
            {
                .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
                .usage = VMA_MEMORY_USAGE_AUTO
            };

            VulkanRenderEngineBackend* backend = m_engine->GetRenderEngineBackend();

            VkBuffer stagingBuffer;
            VmaAllocation stagingAlloc;
            VmaAllocationInfo stagingInfo;
            VKRESERRMSG(vmaCreateBuffer(allocator, &sCreateInfo, &sAllocInfo, &stagingBuffer, &stagingAlloc, &stagingInfo), "Failed to create particle staging buffer");
            IDEFER(backend->PushDeletionObject(new VulkanParticleBufferDeletionObject(backend, stagingBuffer, stagingAlloc)));
            IDEFER(VKRESERR(vmaFlushAllocation(allocator, stagingAlloc, 0, (VkDeviceSize)particleBufferSize)));

#ifdef DEBUG
            vmaSetAllocationName(allocator, stagingAlloc, "Staging Particle Buffer");
#endif

            memcpy(stagingInfo.pMappedData, &value, (size_t)CountSize);
            memset((uint8_t*)stagingInfo.pMappedData + CountSize, 0, ValueSize);
            memcpy((uint8_t*)stagingInfo.pMappedData + 16, particles, (size_t)particleBufferSize - 16);

            const vk::BufferCopy copy = vk::BufferCopy(0, 0, particleBufferSize - ValueSize);
            cmdBuffer.copyBuffer(stagingBuffer, m_particleBuffers[0], 1, &copy);
        }
    }

    const VmaAllocationCreateInfo allocCreateInfo = 
    { 
        .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE
    };

    for (uint32_t i = startIndex; i < MaxParticleBuffers; ++i)
    {
        VkBuffer buffer;
        VKRESERRMSG(vmaCreateBuffer(allocator, &createInfo, &allocCreateInfo, &buffer, &m_allocations[i], nullptr), "Failed to create particle buffer");
#ifdef DEBUG
        vmaSetAllocationName(allocator, m_allocations[i], "Particle Buffer");
#endif

        m_particleBuffers[i] = buffer;

        // Turns out I do not need to fuck around with a staging buffer to set the size and zero initialize the data
        // Note that it needs to be 16 byte aligned so have 12 wasted bytes cause GPUs are annoying like that
        // Eh just means have 12 bytes if I need em for extra data I guess
        cmdBuffer.fillBuffer(m_particleBuffers[i], 0, CountSize, value);
        cmdBuffer.fillBuffer(m_particleBuffers[i], 16, (vk::DeviceSize)particleBufferSize - 16, 0);
    }
}

void VulkanComputeParticle::Update(vk::CommandBuffer a_cmdBuffer, uint32_t a_index)
{
    ComputeParticleBuffer buffer = m_engine->GetParticleBuffer(m_particleBufferAddr);

    if (buffer.MaxParticles <= 0)
    {
        return;
    }

    if (IISBITSET(buffer.Flags, ComputeParticleBuffer::PlayBit))
    {
        const bool generate = IISBITSET(buffer.Flags, ComputeParticleBuffer::DynamicBit) || m_computeShader == -1;
        if (generate)
        {
            Rebuild(&buffer);
        }

        ICLEARBIT(buffer.Flags, ComputeParticleBuffer::PlayBit);
        ISETBIT(buffer.Flags, ComputeParticleBuffer::PlayingBit);
    }

    if (IISBITSET(buffer.Flags, ComputeParticleBuffer::RefreshBit))
    {
        Rebuild(&buffer);
    }

    if (IISBITSET(buffer.Flags, ComputeParticleBuffer::PlayingBit))
    {
        const VulkanRenderEngineBackend* backend = m_engine->GetRenderEngineBackend();
        VulkanPushPool* pushPool = backend->GetPushPool();

        const vk::Device device = backend->GetLogicalDevice();

        const VulkanComputePipeline* pipeline = m_engine->GetComputePipeline(m_computePipeline);
        const VulkanComputeLayout* computeLayout = m_engine->GetComputePipelineLayout(m_computeLayout);

        const vk::Pipeline pipe = pipeline->GetPipeline();
        const vk::PipelineLayout pipeLayout = computeLayout->GetLayout();

        const uint32_t inputCount = computeLayout->GetInputCount();
        const vk::DescriptorSetLayout* descLayouts = computeLayout->GetDescriptorLayouts();
        const ShaderBufferInput* inputs = computeLayout->GetShaderInputs();

        uint32_t particleBufferOffset = 0;

        a_cmdBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, pipe);

        for (uint32_t i = 0; i < inputCount; ++i)
        {
            switch (inputs[i].BufferType)
            {
            case ShaderBufferType_SSParticleBuffer:
            {
                const uint32_t bufferIndex = (m_bufferIndex + particleBufferOffset++) % MaxParticleBuffers;
                
                vk::DescriptorSet set = pushPool->AllocateDescriptor(a_index, vk::DescriptorType::eStorageBuffer, &descLayouts[i]);

                const vk::DescriptorBufferInfo bufferInfo = vk::DescriptorBufferInfo
                (
                    m_particleBuffers[bufferIndex],
                    0,
                    VK_WHOLE_SIZE
                );

                const vk::WriteDescriptorSet descriptorWrite = vk::WriteDescriptorSet
                (
                    set,
                    inputs[i].Slot,
                    0,
                    1,
                    vk::DescriptorType::eStorageBuffer,
                    nullptr,
                    &bufferInfo
                );

                device.updateDescriptorSets(1, &descriptorWrite, 0, nullptr);

                a_cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, pipeLayout, inputs[i].Slot, 1, &set, 0, nullptr);

                break;
            }
            case ShaderBufferType_TimeBuffer:
            {
                const VulkanUniformBuffer* timeUniform = m_engine->GetTimeUniformBuffer();

                const vk::Buffer buffer = timeUniform->GetBuffer(a_index);

                vk::DescriptorSet set = pushPool->AllocateDescriptor(a_index, vk::DescriptorType::eUniformBuffer, &descLayouts[i]);

                const vk::DescriptorBufferInfo bufferInfo = vk::DescriptorBufferInfo
                (
                    buffer,
                    0,
                    VK_WHOLE_SIZE
                );

                const vk::WriteDescriptorSet descriptorWrite = vk::WriteDescriptorSet
                (
                    set,
                    inputs[i].Slot,
                    0,
                    1,
                    vk::DescriptorType::eUniformBuffer,
                    nullptr,
                    &bufferInfo
                );

                device.updateDescriptorSets(1, &descriptorWrite, 0, nullptr);

                a_cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, pipeLayout, inputs[i].Slot, 1, &set, 0, nullptr);

                break;
            }
            default:
            {
                IERROR("Invalid shader buffer type");

                break;
            }
            }
        }        

        a_cmdBuffer.dispatch((uint32_t)glm::ceil(buffer.MaxParticles / 256.0f), 1, 1);
    }

    m_engine->SetParticleBuffer(m_particleBufferAddr, buffer);

    m_bufferIndex = (m_bufferIndex + 1) % MaxParticleBuffers;
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