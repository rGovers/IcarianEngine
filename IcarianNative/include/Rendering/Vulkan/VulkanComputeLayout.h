#pragma once

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN
#include "Rendering/Vulkan/VulkanConstants.h"

class VulkanComputeLayout
{
private:
    vk::PipelineLayout m_layout;

protected:

public:
    VulkanComputeLayout();
    ~VulkanComputeLayout();

    inline vk::PipelineLayout GetLayout() const
    {
        return m_layout;
    }
};

#endif