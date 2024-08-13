// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#include "InteropTypes.h"

#ifdef CUBE_LANGUAGE_CSHARP
namespace IcarianEngine {
#endif

/// @file EngineTransformInteropStructures.h

/// @cond INTERNAL

IOP_PACKED IOP_CSINTERNAL struct TransformBuffer
{
    IOP_CSPUBLIC IOP_UINT32 ParentAddr;
    IOP_CSPUBLIC IOP_VEC3 Translation;
    IOP_CSPUBLIC IOP_QUAT Rotation;
    IOP_CSPUBLIC IOP_VEC3 Scale;

#ifdef CUBE_LANGUAGE_CPP
    constexpr TransformBuffer(uint32_t a_parent = -1, const glm::vec3& a_translation = glm::vec3(0.0f), const glm::quat& a_quat = glm::identity<glm::quat>(), const glm::vec3& a_scale = glm::vec3(1.0f)) :
        ParentAddr(a_parent),
        Translation(a_translation),
        Rotation(a_quat),
        Scale(a_scale)
    {

    }

    glm::mat4 ToMat4() const
    {
        constexpr glm::mat4 Iden = glm::identity<glm::mat4>();

        return glm::scale(glm::translate(Iden, Translation) * glm::toMat4(Rotation), Scale);
    }
#endif
};

/// @endcond

#ifdef CUBE_LANGUAGE_CSHARP
}
#endif

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