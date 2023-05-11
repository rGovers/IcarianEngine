#pragma once

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

struct Viewport
{
    glm::vec2 Position;
    glm::vec2 Size;
    float MinDepth;
    float MaxDepth;

    constexpr Viewport(const glm::vec2& a_position = glm::vec2(0.0f), const glm::vec2& a_size = glm::vec2(1.0f), float a_minDepth = 0.0f, float a_maxDepth = 1.0f) :
        Position(a_position),
        Size(a_size),
        MinDepth(a_minDepth),
        MaxDepth(a_maxDepth)
    {
        
    }
};
