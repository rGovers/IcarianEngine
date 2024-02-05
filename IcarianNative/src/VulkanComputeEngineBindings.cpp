#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/VulkanComputeEngineBindings.h"

#include "DeletionQueue.h"
#include "Flare/IcarianAssert.h"
#include "Flare/IcarianDefer.h"
#include "Rendering/Vulkan/VulkanComputeEngine.h"
#include "Rendering/Vulkan/VulkanComputeParticle2D.h"
#include "Runtime/RuntimeManager.h"

static VulkanComputeEngineBindings* Instance = nullptr;

#define VULKANCOMPUTE_BINDING_FUNCTION_TABLE(F) \
    F(uint32_t, IcarianEngine.Rendering, ParticleSystem, GenerateComputeBuffer, { return Instance->GenerateParticleSystemBuffer(); }) \
    F(void, IcarianEngine.Rendering, ParticleSystem, DestroyComputeBuffer, { Instance->DestroyParticleSystemBuffer(a_addr); }, uint32_t a_addr) \
    F(ComputeParticleBuffer, IcarianEngine.Rendering, ParticleSystem, GetComputeBuffer, { return Instance->GetParticleSystemBuffer(a_addr); }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering, ParticleSystem, SetComputeBuffer, { Instance->SetParticleSystemBuffer(a_addr, a_buffer); }, uint32_t a_addr, ComputeParticleBuffer a_buffer) \
    \
    F(uint32_t, IcarianEngine.Rendering, ParticleSystem2D, GenerateComputeParticleSystem, { return Instance->GenerateParticleSystem2D(a_particleBufferAddr); }, uint32_t a_particleBufferAddr) \
    F(void, IcarianEngine.Rendering, ParticleSystem2D, DestroyComputeParticleSystem, { Instance->DestroyParticleSystem2D(a_addr); }, uint32_t a_addr) \


VULKANCOMPUTE_BINDING_FUNCTION_TABLE(RUNTIME_FUNCTION_DEFINITION)

VulkanComputeEngineBindings::VulkanComputeEngineBindings(VulkanComputeEngine* a_engine)
{
    Instance = this;

    m_engine = a_engine;

    VULKANCOMPUTE_BINDING_FUNCTION_TABLE(RUNTIME_FUNCTION_ATTACH)
}
VulkanComputeEngineBindings::~VulkanComputeEngineBindings()
{
    
}

uint32_t VulkanComputeEngineBindings::GenerateParticleSystemBuffer() const
{
    ComputeParticleBuffer buffer;
    memset(&buffer, 0, sizeof(ComputeParticleBuffer));

    return m_engine->m_particleBuffers.PushVal(buffer);
}
void VulkanComputeEngineBindings::DestroyParticleSystemBuffer(uint32_t a_addr) const
{
    class ParticleSystemBufferDeletionObject : public DeletionObject
    {
    private:
        uint32_t m_addr;

    protected:

    public:
        ParticleSystemBufferDeletionObject(uint32_t a_addr) : m_addr(a_addr) { }
        ~ParticleSystemBufferDeletionObject() { }

        virtual void Destroy()
        {
            ICARIAN_ASSERT_MSG(m_addr < Instance->m_engine->m_particleBuffers.Size(), "DestroyParticleSystemBuffer out of bounds");
            ICARIAN_ASSERT_MSG(Instance->m_engine->m_particleBuffers.Exists(m_addr), "DestroyParticleSystemBuffer already destroyed");

            Instance->m_engine->m_particleBuffers.Erase(m_addr);
        }
    };

    DeletionQueue::Push(new ParticleSystemBufferDeletionObject(a_addr), DeletionIndex_Render);
}
ComputeParticleBuffer VulkanComputeEngineBindings::GetParticleSystemBuffer(uint32_t a_addr) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_engine->m_particleBuffers.Size(), "GetParticleSystemBuffer out of bounds");
    ICARIAN_ASSERT_MSG(m_engine->m_particleBuffers.Exists(a_addr), "GetParticleSystemBuffer already destroyed");

    return m_engine->m_particleBuffers[a_addr];
}
void VulkanComputeEngineBindings::SetParticleSystemBuffer(uint32_t a_addr, const ComputeParticleBuffer& a_buffer) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_engine->m_particleBuffers.Size(), "SetParticleSystemBuffer out of bounds");
    ICARIAN_ASSERT_MSG(m_engine->m_particleBuffers.Exists(a_addr), "SetParticleSystemBuffer already destroyed");

    m_engine->m_particleBuffers.LockSet(a_addr, a_buffer);
}

uint32_t VulkanComputeEngineBindings::GenerateParticleSystem2D(uint32_t a_particleBufferAddr) const
{
    VulkanComputeParticle2D* system = new VulkanComputeParticle2D(m_engine, a_particleBufferAddr);

    return m_engine->m_particle2D.PushVal(system);
}
void VulkanComputeEngineBindings::DestroyParticleSystem2D(uint32_t a_addr) const
{
    class ParticleSystemDeletionObject : public DeletionObject
    {
    private:
        uint32_t m_addr;

    protected:

    public:
        ParticleSystemDeletionObject(uint32_t a_addr) : m_addr(a_addr) { }
        ~ParticleSystemDeletionObject() { }

        virtual void Destroy()
        {
            ICARIAN_ASSERT_MSG(m_addr < Instance->m_engine->m_particle2D.Size(), "DestroyParticleSystemBuffer out of bounds");
            ICARIAN_ASSERT_MSG(Instance->m_engine->m_particle2D.Exists(m_addr), "DestroyParticleSystemBuffer already destroyed");

            const VulkanComputeParticle2D* system = Instance->m_engine->m_particle2D[m_addr];
            IDEFER(delete system);

            Instance->m_engine->m_particle2D.Erase(m_addr);
        }
    };

    DeletionQueue::Push(new ParticleSystemDeletionObject(a_addr), DeletionIndex_Render);
}

#endif