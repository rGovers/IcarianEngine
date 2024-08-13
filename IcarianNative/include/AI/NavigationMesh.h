// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include <filesystem>

#include "DataTypes/Array.h"

struct NavigationFace
{
    uint32_t Indicies[3];
    uint32_t Connections[3];
    glm::vec3 Center;
    // Do not care about the normal only the gradient
    float Dot;
};

class NavigationMesh
{
private:
    uint32_t        m_vertexCount;
    glm::vec3*      m_vertices;

    uint32_t        m_faceCount;
    NavigationFace* m_faces;

protected:

public:
    NavigationMesh(const std::filesystem::path& a_path);
    ~NavigationMesh();

    uint32_t GetIndex(const glm::vec3& a_point) const;

    Array<glm::vec3> GeneratePath(const glm::vec3& a_startPoint, const glm::vec3& a_endPoint, float a_agentRadius) const;
    Array<glm::vec3> GeneratePath(const glm::vec3& a_startPoint, const glm::vec3& a_endPoint, uint32_t a_startIndex, uint32_t a_endIndex, float a_agentRadius) const;
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