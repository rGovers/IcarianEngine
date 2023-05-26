#pragma once

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

struct CanvasBuffer
{
    static constexpr uint32_t DestroyedBit = 0;
    static constexpr uint32_t CaptureInputBit = 1;

    glm::vec2 ReferenceResolution;
    uint32_t ChildElementCount;
    uint32_t* ChildElements;
    unsigned char Flags;
};