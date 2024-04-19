#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/VulkanComputeParticle.h"

#include <cstring>

#include "Core/IcarianAssert.h"
#include "Core/IcarianDefer.h"
#include "Rendering/ShaderBuffers.h"
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

    VmaAllocation              m_allocations[VulkanComputeParticle::MaxParticleBuffers];
    vk::Buffer                 m_buffers[VulkanComputeParticle::MaxParticleBuffers];

protected:

public:
    VulkanParticleBufferDeletionObject(VulkanRenderEngineBackend* a_engine, const vk::Buffer* a_buffers, const VmaAllocation* a_allocations)
    {
        m_engine = a_engine;
        
        for (uint32_t i = 0; i < VulkanComputeParticle::MaxParticleBuffers; ++i)
        {
            m_buffers[i] = a_buffers[i];
            m_allocations[i] = a_allocations[i];
        }
    }
    virtual ~VulkanParticleBufferDeletionObject()
    {

    }

    virtual void Destroy()
    {
        TRACE("Destroying Particle Buffers");
        const VmaAllocator allocator = m_engine->GetAllocator();

        for (uint32_t i = 0; i < VulkanComputeParticle::MaxParticleBuffers; ++i)
        {
            vmaDestroyBuffer(allocator, m_buffers[i], m_allocations[i]);
        }
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
        backend->PushDeletionObject(new VulkanParticleBufferDeletionObject(backend, m_particleBuffers, m_allocations));

        for (uint32_t i = 0; i < MaxParticleBuffers; ++i)
        {
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

    std::vector<ShaderBufferInput> inputs;
    const std::string shaderStr = VulkanParticleShaderGenerator::GenerateComputeShader(*a_buffer, &inputs);

    m_computeShader = m_engine->GenerateComputeFShader(shaderStr);
    m_computeLayout = m_engine->GenerateComputePipelineLayout(inputs.data(), (uint32_t)inputs.size());
    m_computePipeline = m_engine->GenerateComputePipeline(m_computeShader, m_computeLayout);

    VulkanRenderEngineBackend* backend = m_engine->GetRenderEngineBackend();
    const VmaAllocator allocator = backend->GetAllocator();

    const uint32_t particleBufferSize = sizeof(ParticleShaderBuffer) * a_buffer->MaxParticles + 16;

    VkBufferCreateInfo createInfo = { };
    createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    createInfo.size = particleBufferSize;
    createInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocCreateInfo = { 0 };
    allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;

    TLockObj<vk::CommandBuffer, std::mutex>* buffer = backend->BeginSingleCommand();
    IDEFER(backend->EndSingleCommand(buffer));
    const vk::CommandBuffer cmdBuffer = buffer->Get();

    for (uint32_t i = 0; i < MaxParticleBuffers; ++i)
    {
        VkBuffer buffer;
        ICARIAN_ASSERT_MSG_R(vmaCreateBuffer(allocator, &createInfo, &allocCreateInfo, &buffer, &m_allocations[i], nullptr) == VK_SUCCESS, "Failed to create particle buffer");

        m_particleBuffers[i] = buffer;

        int32_t value = (int32_t)a_buffer->MaxParticles;
        constexpr vk::DeviceSize countSize = sizeof(value);

        // Turns out I do not need to fuck around with a staging buffer to set the size and zero initialize the data
        // Note that it needs to be 16 byte aligned so have 12 wasted bytes cause GPUs are annoying like that
        // Eh just means have 12 bytes if I need em for extra data I guess
        cmdBuffer.fillBuffer(m_particleBuffers[i], 0, countSize, value);
        cmdBuffer.fillBuffer(m_particleBuffers[i], countSize, (vk::DeviceSize)particleBufferSize - countSize, 0);
    }

    a_buffer->Flags &= ~(0b1 << ComputeParticleBuffer::RefreshBit);
}

void VulkanComputeParticle::Update(vk::CommandBuffer a_cmdBuffer, uint32_t a_index)
{
    ComputeParticleBuffer buffer = m_engine->GetParticleBuffer(m_particleBufferAddr);

    if (buffer.MaxParticles <= 0)
    {
        return;
    }

    if (buffer.Flags & 0b1 << ComputeParticleBuffer::PlayBit)
    {
        const bool generate = (buffer.Flags & 0b1 << ComputeParticleBuffer::DynamicBit) != 0 || m_computeShader == -1;
        if (generate)
        {
            Rebuild(&buffer);
        }

        buffer.Flags &= ~(0b1 << ComputeParticleBuffer::PlayBit);
        buffer.Flags |= 0b1 << ComputeParticleBuffer::PlayingBit;
    }

    if (buffer.Flags & 0b1 << ComputeParticleBuffer::RefreshBit)
    {
        Rebuild(&buffer);
    }

    if (buffer.Flags & 0b1 << ComputeParticleBuffer::PlayingBit)
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

                a_cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, pipeLayout, inputs[i].Set, 1, &set, 0, nullptr);

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

                a_cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, pipeLayout, inputs[i].Set, 1, &set, 0, nullptr);

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