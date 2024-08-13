// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#define GLM_FORCE_SWIZZLE
#include <glm/glm.hpp>

struct Frustum
{
    glm::vec4 Planes[6];

    bool CompareSphere(const glm::vec3& a_pos, float a_radius) const
    {
        for (int i = 0; i < 6; ++i)
        {
            const float d = glm::dot(Planes[i].xyz(), a_pos) + Planes[i].w;

            if (d + a_radius + 0.01f < 0)
            {
                return false;
            }
        }

        return true;
    }

    static Frustum FromMat4(const glm::mat4& a_matrix)
    {
        Frustum result;

        for (int i = 0; i < 3; ++i)
        {
            result.Planes[i * 2] = glm::vec4
            (
                a_matrix[0][3] - a_matrix[0][i],
                a_matrix[1][3] - a_matrix[1][i],
                a_matrix[2][3] - a_matrix[2][i],
                a_matrix[3][3] - a_matrix[3][i]
            );
            result.Planes[i * 2] /= glm::length(result.Planes[i * 2].xyz());

            result.Planes[i * 2 + 1] = glm::vec4
            (
                a_matrix[0][3] + a_matrix[0][i],
                a_matrix[1][3] + a_matrix[1][i],
                a_matrix[2][3] + a_matrix[2][i],
                a_matrix[3][3] + a_matrix[3][i]
            );
            result.Planes[i * 2 + 1] /= glm::length(result.Planes[i * 2 + 1].xyz());
        }

        return result;
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