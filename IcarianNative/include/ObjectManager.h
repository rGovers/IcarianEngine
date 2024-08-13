// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <queue>
#include <vector>

#include "DataTypes/TArray.h"

#include "EngineTransformInteropStructures.h"

class ObjectManager
{
private:
    std::queue<uint32_t>    m_freeTransforms;
    TArray<TransformBuffer> m_transformBuffer;

    ObjectManager();
protected:

public:
    ~ObjectManager();

    static void Init();
    static void Destroy();

    static uint32_t* BatchCreateTransformBuffer(uint32_t a_count);
    static uint32_t CreateTransformBuffer();
    static TransformBuffer GetTransformBuffer(uint32_t a_addr);
    static void SetTransformBuffer(uint32_t a_addr, const TransformBuffer& a_buffer);
    static void DestroyTransformBuffer(uint32_t a_addr);

    static glm::mat4 GetMatrix(uint32_t a_addr);
    static glm::mat4 GetGlobalMatrix(uint32_t a_addr);
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