#pragma once

#include <cstdint>

#include "Frustum.h"
#include "ObjectManager.h"
#include "Rendering/Viewport.h"

#include <glm/ext/matrix_clip_space.hpp>

struct CameraBuffer
{
    uint32_t TransformAddr;
    uint32_t RenderTextureAddr;
    Viewport View;
    uint32_t RenderLayer;
    float FOV;
    float Near;
    float Far;

    constexpr CameraBuffer(uint32_t a_transformAddr = -1, uint32_t a_renderTextureAddr = -1, const Viewport& a_viewport = Viewport(), uint32_t a_renderLayer = 0b1, float a_fov = glm::pi<float>() * 0.4f, float a_near = 0.1f, float a_far = 100.0f) :
        TransformAddr(a_transformAddr),
        RenderTextureAddr(a_renderTextureAddr), 
        View(a_viewport), 
        RenderLayer(a_renderLayer), 
        FOV(a_fov), 
        Near(a_near), 
        Far(a_far) 
    {

    }

    inline glm::mat4 ToProjection(const glm::vec2& a_screenSize) const
    {
        return glm::perspective(FOV, a_screenSize.x / a_screenSize.y, Near, Far);
    }
    inline glm::mat4 ToProjection(const glm::vec2& a_screenSize, float a_near, float a_far) const
    {
        return glm::perspective(FOV, a_screenSize.x / a_screenSize.y, a_near, a_far);
    }

    Frustum ToFrustum(const glm::vec2& a_screenSize, ObjectManager* a_objectManager) const
    {
        const glm::mat4 proj = ToProjection(a_screenSize);
        const glm::mat4 trans = glm::inverse(a_objectManager->GetGlobalMatrix(TransformAddr));

        return Frustum::FromMat4(proj * trans);
    }
};
