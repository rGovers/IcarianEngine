#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Core/IcarianDefer.h"
#include "Rendering/Vulkan/VulkanLightData.h"

VulkanLightData::VulkanLightData()
{
    m_splitCount = 0;
    m_splits = nullptr;
}
VulkanLightData::~VulkanLightData()
{
    if (m_splits != nullptr)
    {
        delete[] m_splits;
    }
}

void VulkanLightData::SetLightSplits(const LightShadowSplit* a_splits, uint32_t a_splitCount)
{
    IDEFER(m_splitCount = a_splitCount);
    if (a_splitCount <= 0)
    {
        return;
    }

    if (m_splits != nullptr)
    {
        if (m_splitCount < a_splitCount)
        {
            delete[] m_splits;
            m_splits = new LightShadowSplit[a_splitCount];
        }
    }
    else
    {
        m_splits = new LightShadowSplit[a_splitCount];
    }

    for (uint32_t i = 0; i < a_splitCount; ++i)
    {
        m_splits[i] = a_splits[i];
    }
}

#endif