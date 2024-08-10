#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/VulkanComputeEngineBindings.h"

#include "DeletionQueue.h"
#include "Core/IcarianAssert.h"
#include "Core/IcarianDefer.h"
#include "Rendering/Vulkan/VulkanComputeEngine.h"
#include "Rendering/Vulkan/VulkanComputeParticle.h"
#include "Runtime/RuntimeManager.h"

static VulkanComputeEngineBindings* Instance = nullptr;

#define VULKANCOMPUTE_BINDING_FUNCTION_TABLE(F) \
    F(uint32_t, IcarianEngine.Rendering, ParticleSystem, GenerateComputeBuffer, { return Instance->GenerateParticleSystemBuffer(a_tranformAddr); }, uint32_t a_tranformAddr) \
    F(void, IcarianEngine.Rendering, ParticleSystem, DestroyComputeBuffer, { IPUSHDELETIONFUNC(Instance->DestroyParticleSystemBuffer(a_addr), DeletionIndex_Render); }, uint32_t a_addr) \
    F(ComputeParticleBuffer, IcarianEngine.Rendering, ParticleSystem, GetComputeBuffer, { return Instance->GetParticleSystemBuffer(a_addr); }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering, ParticleSystem, SetComputeBuffer, { Instance->SetParticleSystemBuffer(a_addr, a_buffer); }, uint32_t a_addr, ComputeParticleBuffer a_buffer) \
    \
    F(uint32_t, IcarianEngine.Rendering, ParticleSystem2D, GenerateComputeParticleSystem, { return Instance->GenerateParticleSystem(a_particleBufferAddr); }, uint32_t a_particleBufferAddr) \
    F(void, IcarianEngine.Rendering, ParticleSystem2D, DestroyComputeParticleSystem, { IPUSHDELETIONFUNC(Instance->DestroyParticleSystem(a_addr), DeletionIndex_Render); }, uint32_t a_addr) \

VULKANCOMPUTE_BINDING_FUNCTION_TABLE(RUNTIME_FUNCTION_DEFINITION);

VulkanComputeEngineBindings::VulkanComputeEngineBindings(VulkanComputeEngine* a_engine)
{
    Instance = this;

    m_engine = a_engine;

    VULKANCOMPUTE_BINDING_FUNCTION_TABLE(RUNTIME_FUNCTION_ATTACH);
}
VulkanComputeEngineBindings::~VulkanComputeEngineBindings()
{
    
}

uint32_t VulkanComputeEngineBindings::GenerateParticleSystemBuffer(uint32_t a_transformAddr) const
{
    const ComputeParticleBuffer buffer = 
    { 
        .TransformAddr = a_transformAddr,
        .RenderLayer = 1,
        .Gravity = glm::vec3(0.0f, 9.807f, 0.0f),
        .Colour = glm::vec4(1.0),
    };

    return m_engine->m_particleBuffers.PushVal(buffer);
}
void VulkanComputeEngineBindings::DestroyParticleSystemBuffer(uint32_t a_addr) const
{
    IVERIFY(a_addr < m_engine->m_particleBuffers.Size());
    IVERIFY(m_engine->m_particleBuffers.Exists(a_addr));

    m_engine->m_particleBuffers.Erase(a_addr);
}
ComputeParticleBuffer VulkanComputeEngineBindings::GetParticleSystemBuffer(uint32_t a_addr) const
{
    IVERIFY(a_addr < m_engine->m_particleBuffers.Size());
    IVERIFY(m_engine->m_particleBuffers.Exists(a_addr));

    return m_engine->m_particleBuffers[a_addr];
}
void VulkanComputeEngineBindings::SetParticleSystemBuffer(uint32_t a_addr, const ComputeParticleBuffer& a_buffer) const
{
    IVERIFY(a_addr < m_engine->m_particleBuffers.Size());
    IVERIFY(m_engine->m_particleBuffers.Exists(a_addr));

    m_engine->m_particleBuffers.LockSet(a_addr, a_buffer);
}

uint32_t VulkanComputeEngineBindings::GenerateParticleSystem(uint32_t a_particleBufferAddr) const
{
    IVERIFY(a_particleBufferAddr < Instance->m_engine->m_particleBuffers.Size());
    IVERIFY(Instance->m_engine->m_particleBuffers.Exists(a_particleBufferAddr));

    VulkanComputeParticle* system = new VulkanComputeParticle(m_engine, a_particleBufferAddr);

    TLockArray<ComputeParticleBuffer> a = Instance->m_engine->m_particleBuffers.ToLockArray();

    ComputeParticleBuffer& buffer = a[a_particleBufferAddr];
    IVERIFY(buffer.Data == nullptr);

    buffer.Data = system;

    return a_particleBufferAddr;
}
void VulkanComputeEngineBindings::DestroyParticleSystem(uint32_t a_addr) const
{
    IVERIFY(a_addr < m_engine->m_particleBuffers.Size());
    IVERIFY(m_engine->m_particleBuffers.Exists(a_addr));

    TLockArray<ComputeParticleBuffer> a = Instance->m_engine->m_particleBuffers.ToLockArray();

    ComputeParticleBuffer& buffer = a[a_addr];
    IVERIFY(buffer.Data != nullptr);

    const VulkanComputeParticle* data = (VulkanComputeParticle*)buffer.Data;
    IDEFER(delete data);
    buffer.Data = nullptr;
}

#endif