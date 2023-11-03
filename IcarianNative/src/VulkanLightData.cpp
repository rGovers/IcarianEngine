#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/VulkanLightData.h"

VulkanLightData::VulkanLightData()
{
    m_lvpCount = 0;
    m_lvp = nullptr;

    m_splitCount = 0;
    m_splits = nullptr;
}
VulkanLightData::~VulkanLightData()
{
    if (m_lvp != nullptr)
    {
        delete[] m_lvp;
    }

    if (m_splits != nullptr)
    {
        delete[] m_splits;
    }
}

void VulkanLightData::SetLVP(const glm::mat4* a_lvp, uint32_t a_lvpCount)
{
    if (m_lvp != nullptr)
    {
        delete[] m_lvp;
        m_lvp = nullptr;
    }

    m_lvpCount = a_lvpCount;

    if (m_lvpCount > 0)
    {
        m_lvp = new glm::mat4[m_lvpCount];

        for (uint32_t i = 0; i < m_lvpCount; i++)
        {
            m_lvp[i] = a_lvp[i];
        }
    }
}

void VulkanLightData::SetSplits(const float* a_splits, uint32_t a_splitCount)
{
    if (m_splits != nullptr)
    {
        delete[] m_splits;
        m_splits = nullptr;
    }

    m_splitCount = a_splitCount;

    if (m_splitCount > 0)
    {
        m_splits = new float[m_splitCount];

        for (uint32_t i = 0; i < m_splitCount; i++)
        {
            m_splits[i] = a_splits[i];
        }
    }
}
#endif