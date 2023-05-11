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

    constexpr DirectionalLightBuffer(uint32_t a_transformAddr = -1, uint32_t a_renderLayer = 0b1, const glm::vec4& a_color = glm::vec4(1.0f), float a_intensity = 10.0f) :
        TransformAddr(a_transformAddr),
        RenderLayer(a_renderLayer),
        Color(a_color),
        Intensity(a_intensity)
    {

    }
};

struct PointLightBuffer
{
    uint32_t TransformAddr;
    uint32_t RenderLayer;
    glm::vec4 Color;
    float Intensity;
    float Radius;

    constexpr PointLightBuffer(uint32_t a_transformAddr = -1, uint32_t a_renderLayer = 0b1, const glm::vec4& a_color = glm::vec4(1.0f), float a_intensity = 10.0f, float a_radius = 1.0f) :
        TransformAddr(a_transformAddr),
        RenderLayer(a_renderLayer),
        Color(a_color),
        Intensity(a_intensity),
        Radius(a_radius)
    {

    }
};

struct SpotLightBuffer
{
    uint32_t TransformAddr;
    uint32_t RenderLayer;
    glm::vec4 Color;
    float Intensity;
    glm::vec2 CutoffAngle;
    float Radius;

    constexpr SpotLightBuffer(uint32_t a_transformAddr = -1, uint32_t a_renderLayer = 0b1, const glm::vec4& a_color = glm::vec4(1.0f), float a_intensity = 10.0f, const glm::vec2& a_cutoff = glm::vec2(1.0f, 1.5f), float a_radius = 10.0f) :
        TransformAddr(a_transformAddr),
        RenderLayer(a_renderLayer),
        Color(a_color),
        Intensity(a_intensity),
        CutoffAngle(a_cutoff),
        Radius(a_radius)
    {

    }
};