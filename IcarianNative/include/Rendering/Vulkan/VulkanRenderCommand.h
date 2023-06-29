#pragma once

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include "Rendering/Vulkan/VulkanConstants.h"

#include "Flare/TextureSampler.h"

class VulkanGraphicsEngine;
class VulkanPipeline;
class VulkanRenderEngineBackend;
class VulkanRenderTexture;
class VulkanSwapchain;

class VulkanRenderCommand
{
private:
    constexpr static uint32_t FlushedBit = 0;

    VulkanRenderEngineBackend* m_engine;
    VulkanGraphicsEngine*      m_gEngine;
    VulkanSwapchain*           m_swapchain;

    uint32_t                   m_bufferIndex;

    unsigned char              m_flags;

    uint32_t                   m_renderTexAddr;
    uint32_t                   m_materialAddr;
    uint32_t                   m_cameraAddr;

    vk::CommandBuffer          m_commandBuffer;

    void SetFlushedState(bool a_value);

protected:

public:
    VulkanRenderCommand(VulkanRenderEngineBackend* a_engine, VulkanGraphicsEngine* a_gEngine, VulkanSwapchain* a_swapchain, vk::CommandBuffer a_buffer, uint32_t a_camAddr, uint32_t a_bufferIndex);
    ~VulkanRenderCommand();

    inline bool IsFlushed() const
    {
        return m_flags & 0b1 << FlushedBit;
    }

    void Flush();

    inline uint32_t GetRenderTexutreAddr() const
    {
        return m_renderTexAddr;
    }
    VulkanRenderTexture* GetRenderTexture() const;

    inline uint32_t GetMaterialAddr() const
    {
        return m_materialAddr;
    }
    VulkanPipeline* GetPipeline() const;

    VulkanPipeline* BindMaterial(uint32_t a_materialAddr);

    void PushTexture(uint32_t a_slot, const FlareBase::TextureSampler& a_sampler) const;
    
    void BindRenderTexture(uint32_t a_renderTexAddr);
    
    void Blit(const VulkanRenderTexture* a_src, const VulkanRenderTexture* a_dst);

    void DrawMaterial();
    void DrawModel(const glm::mat4& a_transform, uint32_t a_addr);
};