#pragma once

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include <cstdint>

enum e_LightType : uint16_t
{
    LightType_Directional = 0,
    LightType_Point = 1,
    LightType_Spot = 2,
    LightType_End = 3
};

struct DirectionalLightBuffer
{
    uint32_t TransformAddr;
    uint32_t RenderLayer;
    glm::vec4 Color;
    float Intensity;
    void* Data;
};

struct PointLightBuffer
{
    uint32_t TransformAddr;
    uint32_t RenderLayer;
    glm::vec4 Color;
    float Intensity;
    float Radius;
    void* Data;
};

struct SpotLightBuffer
{
    uint32_t TransformAddr;
    uint32_t RenderLayer;
    glm::vec4 Color;
    float Intensity;
    glm::vec2 CutoffAngle;
    float Radius;
    void* Data;
};