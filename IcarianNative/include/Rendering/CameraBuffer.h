// Icarian Engine - C# Game Engine
// 
// License at end of file.

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

    Frustum ToFrustum(const glm::vec2& a_screenSize) const
    {
        const glm::mat4 proj = ToProjection(a_screenSize);
        const glm::mat4 trans = glm::inverse(ObjectManager::GetGlobalMatrix(TransformAddr));

        return Frustum::FromMat4(proj * trans);
    }
};

// MIT License
// 
// Copyright (c) 2024 River Govers
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.