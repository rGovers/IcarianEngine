#pragma once

#define GLM_FORCE_SWIZZLE
#include <glm/glm.hpp>

#include <cstdint>
#include <string>

namespace IcarianCore
{
    struct BoneData
    {
        std::string Name;
        glm::mat4 Transform;
        uint32_t Parent;
    };
}
