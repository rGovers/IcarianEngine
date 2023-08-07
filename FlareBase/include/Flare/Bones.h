#pragma once

#define GLM_SWIZZLE
#include <glm/glm.hpp>

#include <string>

struct BoneData
{
    std::string Name;
    glm::mat4 Transform;
    uint32_t Parent;
};