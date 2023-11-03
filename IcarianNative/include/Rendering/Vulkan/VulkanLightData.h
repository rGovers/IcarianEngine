#pragma once

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN
#define GLM_FORCE_SWIZZLE
#include <glm/glm.hpp>

class VulkanLightData
{
private:
    glm::mat4* m_lvp;
    uint32_t   m_lvpCount;

    float*     m_splits;
    uint32_t   m_splitCount;

protected:

public:
    VulkanLightData();
    ~VulkanLightData();

    void SetLVP(const glm::mat4* a_lvp, uint32_t a_lvpCount);
    inline const glm::mat4* GetLVP() const 
    { 
        return m_lvp; 
    }
    inline uint32_t GetLVPCount() const 
    { 
        return m_lvpCount; 
    }

    void SetSplits(const float* a_splits, uint32_t a_splitCount);
    inline const float* GetSplits() const 
    { 
        return m_splits; 
    }
    inline uint32_t GetSplitCount() const 
    { 
        return m_splitCount; 
    }
};
#endif