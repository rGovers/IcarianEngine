#pragma once

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#define GLM_FORCE_SWIZZLE
#include <glm/glm.hpp>

#include "EngineRenderCommandInteropStructures.h"

class VulkanLightData
{
private:
    LightShadowSplit* m_splits;    
    uint32_t          m_splitCount;

protected:

public:
    VulkanLightData();
    ~VulkanLightData();

    inline uint32_t GetSplitCount() const
    {
        return m_splitCount;
    }
    inline const LightShadowSplit* GetSplits() const
    {
        return m_splits;
    }

    void SetLightSplits(const LightShadowSplit* a_splits, uint32_t a_splitCount);
};

#endif