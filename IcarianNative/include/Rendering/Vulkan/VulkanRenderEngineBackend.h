#pragma once

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN
#include "Rendering/Vulkan/VulkanConstants.h"

#include "DataTypes/TArray.h"
#include "DataTypes/TLockObj.h"
#include "Rendering/RenderEngineBackend.h"

#include <mutex>

class AppWindow;
class VulkanComputeEngine;
class VulkanGraphicsEngine;
class VulkanPushPool;
class VulkanSwapchain;

class VulkanDeletionObject
{
private:

protected:

public:
    virtual ~VulkanDeletionObject() { }

    virtual void Destroy() = 0;
};

class VulkanRenderEngineBackend : public RenderEngineBackend
{
private:
    VulkanComputeEngine*                          m_computeEngine;
    VulkanGraphicsEngine*                         m_graphicsEngine;
    VulkanSwapchain*                              m_swapchain = nullptr;
    VulkanPushPool*                               m_pushPool;
                
    VmaAllocator                                  m_allocator;
                
    vk::Instance                                  m_instance;
    vk::DebugUtilsMessengerEXT                    m_messenger;
    vk::PhysicalDevice                            m_pDevice;
    vk::Device                                    m_lDevice;
                        
    std::mutex                                    m_graphicsQueueMutex;
    vk::Queue                                     m_computeQueue = nullptr;
    vk::Queue                                     m_graphicsQueue = nullptr;
    vk::Queue                                     m_presentQueue = nullptr;
    
    vk::PhysicalDevicePushDescriptorPropertiesKHR m_pushDescriptorProperties;

    TArray<VulkanDeletionObject*>                 m_deletionObjects[VulkanDeletionQueueSize];

    std::vector<vk::Semaphore>                    m_interSemaphore[VulkanMaxFlightFrames];
    vk::Semaphore                                 m_imageAvailable[VulkanMaxFlightFrames];
    vk::Fence                                     m_inFlight[VulkanMaxFlightFrames];
            
    vk::CommandPool                               m_commandPool;
                
    uint32_t                                      m_imageIndex = -1;
    uint32_t                                      m_currentFrame = 0;
    uint32_t                                      m_currentFlightFrame = 0;
    uint32_t                                      m_dQueueIndex = 0;

    uint32_t                                      m_computeQueueIndex = -1;
    uint32_t                                      m_graphicsQueueIndex = -1;
    uint32_t                                      m_presentQueueIndex = -1;

protected:

public:
    VulkanRenderEngineBackend(RenderEngine* a_engine);
    virtual ~VulkanRenderEngineBackend();

    virtual void Update(double a_delta, double a_time);

    TLockObj<vk::CommandBuffer, std::mutex>* CreateCommandBuffer(vk::CommandBufferLevel a_level);
    void DestroyCommandBuffer(TLockObj<vk::CommandBuffer, std::mutex>* a_buffer);

    TLockObj<vk::CommandBuffer, std::mutex>* BeginSingleCommand();
    void EndSingleCommand(TLockObj<vk::CommandBuffer, std::mutex>* a_buffer);

    virtual uint32_t GenerateAlphaTexture(uint32_t a_width, uint32_t a_height, const void* a_data);
    virtual void DestroyTexture(uint32_t a_addr);

    virtual uint32_t GenerateTextureSampler(uint32_t a_textureAddr, e_TextureMode a_textureMode, e_TextureFilter a_filterMode, e_TextureAddress a_addressMode, uint32_t a_slot = 0);
    virtual void DestroyTextureSampler(uint32_t a_addr);

    virtual Font* GetFont(uint32_t a_addr);

    inline VulkanPushPool* GetPushPool() const
    {
        return m_pushPool;
    }

    void PushDeletionObject(VulkanDeletionObject* a_object);

    inline vk::Fence GetCurrentFlightFence() const
    {
        return m_inFlight[m_currentFlightFrame];
    }
    inline vk::Semaphore GetImageSemaphore(uint32_t a_index) const
    {
        return m_imageAvailable[a_index];
    }

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
    inline uint32_t GetComputeQueueIndex() const
    {
        return m_computeQueueIndex;
    }
    inline uint32_t GetGraphicsQueueIndex() const
    {
        return m_graphicsQueueIndex;
    }

    inline vk::Queue GetPresentQueue() const
    {
        return m_presentQueue;
    }
    inline vk::Queue GetCopmuteQueue() const
    {
        return m_computeQueue;
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
#endif