#pragma once

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN
#include "Rendering/Vulkan/VulkanConstants.h"

#include <string_view>

#include "DataTypes/TNCArray.h"

class VulkanComputeEngineBindings;
class VulkanComputeLayout;
class VulkanComputeParticle2D;
class VulkanComputePipeline;
class VulkanComputeShader;
class VulkanRenderEngineBackend;

#include "EngineMaterialInteropStructures.h"
#include "EngineParticleSystemInteropStructures.h"

class VulkanComputeEngine
{
private:
    friend class VulkanComputeEngineBindings;

    VulkanRenderEngineBackend*         m_engine;
    VulkanComputeEngineBindings*       m_bindings;
    
    vk::CommandPool                    m_pools[VulkanFlightPoolSize];
    vk::CommandBuffer                  m_buffers[VulkanFlightPoolSize];

    TNCArray<VulkanComputeShader*>     m_shaders;
    TNCArray<VulkanComputePipeline*>   m_pipelines;
    TNCArray<VulkanComputeLayout*>     m_layouts;
    
    TNCArray<ComputeParticleBuffer>    m_particleBuffers;

    TNCArray<VulkanComputeParticle2D*> m_particle2D;

protected:

public:
    VulkanComputeEngine(VulkanRenderEngineBackend* a_engine);
    ~VulkanComputeEngine();

    inline VulkanRenderEngineBackend* GetRenderEngineBackend() const
    {
        return m_engine;
    }

    vk::CommandBuffer Update(uint32_t a_index);

    ComputeParticleBuffer GetParticleBuffer(uint32_t a_addr);
    void SetParticleBuffer(uint32_t a_addr, const ComputeParticleBuffer& a_buffer);

    uint32_t GenerateComputeFShader(const std::string_view& a_str);
    uint32_t GenerateComputeGLSLShader(const std::string_view& a_str);
    void DestroyComputeShader(uint32_t a_addr);
    VulkanComputeShader* GetComputeShader(uint32_t a_addr);

    uint32_t GenerateComputePipelineLayout(const ShaderBufferInput* a_inputs, uint32_t a_count);
    void DestroyComputePipelineLayout(uint32_t a_addr);
    VulkanComputeLayout* GetComputePipelineLayout(uint32_t a_addr);

    uint32_t GenerateComputePipeline(uint32_t a_shaderAddr, uint32_t a_layoutAddr);
    void DestroyComputePipeline(uint32_t a_addr);
    VulkanComputePipeline* GetComputePipeline(uint32_t a_addr);
};

#endif