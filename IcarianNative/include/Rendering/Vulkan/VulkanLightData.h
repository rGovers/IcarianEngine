#pragma once

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN
#define GLM_FORCE_SWIZZLE
#include <glm/glm.hpp>

class VulkanLightData
{
private:
    glm::mat4 m_lvp;
    
protected:

public:
    VulkanLightData();
    ~VulkanLightData();

    inline void SetLVP(const glm::mat4& a_lvp) 
    { 
        m_lvp = a_lvp; 
    }
    inline glm::mat4 GetLVP() const 
    { 
        return m_lvp; 
    }
};
#endif