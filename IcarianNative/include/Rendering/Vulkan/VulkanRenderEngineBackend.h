#pragma once

#include "Rendering/RenderEngineBackend.h"

#include <mutex>

#include "Rendering/Vulkan/VulkanConstants.h"

class AppWindow;
class RuntimeManager;
class VulkanGraphicsEngine;
class VulkanSwapchain;

class VulkanRenderEngineBackend : public RenderEngineBackend
{
private:
    RuntimeManager*                               m_runtime;
    VulkanGraphicsEngine*                         m_graphicsEngine;
    VulkanSwapchain*                              m_swapchain = nullptr;
                
    VmaAllocator                                  m_allocator;
                
    vk::Instance                                  m_instance;
    vk::DebugUtilsMessengerEXT                    m_messenger;
    vk::PhysicalDevice                            m_pDevice;
    vk::Device                                    m_lDevice;
                        
    std::mutex                                    m_graphicsQueueMutex;
    vk::Queue                                     m_graphicsQueue = nullptr;
    vk::Queue                                     m_presentQueue = nullptr;
    
    vk::PhysicalDevicePushDescriptorPropertiesKHR m_pushDescriptorProperties;

    std::vector<vk::Semaphore>                    m_interSemaphore[VulkanMaxFlightFrames];
    vk::Semaphore                                 m_imageAvailable[VulkanMaxFlightFrames];
    vk::Fence                                     m_inFlight[VulkanMaxFlightFrames];
            
    vk::CommandPool                               m_commandPool;
                
    uint32_t                                      m_imageIndex = -1;
    uint32_t                                      m_currentFrame = 0;
    uint32_t                                      m_currentFlightFrame = 0;
            
    uint32_t                                      m_graphicsQueueIndex = -1;
    uint32_t                                      m_presentQueueIndex = -1;

protected:

public:
    VulkanRenderEngineBackend(RuntimeManager* a_runtime, RenderEngine* a_engine);
    virtual ~VulkanRenderEngineBackend();

    virtual void Update(double a_delta, double a_time);

    vk::CommandBuffer CreateCommandBuffer(vk::CommandBufferLevel a_level);
    void DestroyCommandBuffer(const vk::CommandBuffer& a_buffer) const;

    vk::CommandBuffer BeginSingleCommand();
    void EndSingleCommand(const vk::CommandBuffer& a_buffer);

    virtual uint32_t GenerateAlphaTexture(uint32_t a_width, uint32_t a_height, const void* a_data);
    virtual void DestroyTexture(uint32_t a_addr);

    virtual uint32_t GenerateTextureSampler(uint32_t a_textureAddr, FlareBase::e_TextureMode a_textureMode, FlareBase::e_TextureFilter a_filterMode, FlareBase::e_TextureAddress a_addressMode, uint32_t a_slot = 0);
    virtual void DestroyTextureSampler(uint32_t a_addr);

    virtual Font* GetFont(uint32_t a_addr);

    inline VmaAllocator GetAllocator() const
    {
        return m_allocator;
    }

    inline vk::Instance GetInstance() const
    {
        return m_instance;
    }

    inline vk::Device GetLogicalDevice() const
    {
        return m_lDevice;
    }
    inline vk::PhysicalDevice GetPhysicalDevice() const
    {
        return m_pDevice;
    }

    inline uint32_t GetPresentQueueIndex() const
    {
        return m_presentQueueIndex;
    }
    inline uint32_t GetGraphicsQueueIndex() const
    {
        return m_graphicsQueueIndex;
    }
    inline vk::Queue GetPresentQueue() const
    {
        return m_presentQueue;
    }
    inline std::mutex& GetGraphicsQueueMutex()
    {
        return m_graphicsQueueMutex;
    }
    inline vk::Queue GetGraphicsQueue() const
    {
        return m_graphicsQueue;
    }

    inline uint32_t GetImageIndex() const
    {
        return m_imageIndex;
    }
    inline uint32_t GetCurrentFrame() const
    {
        return m_currentFrame;
    }
    inline uint32_t GetCurrentFlightFrame() const
    {
        return m_currentFlightFrame;
    }
};