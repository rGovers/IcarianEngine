// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#include <cstdint>
#include <vector>

struct MeshRenderBuffer;
struct SkinnedMeshRenderBuffer;

struct ModelBuffer
{
    uint32_t ModelAddr;
    uint32_t* TransformAddr;
    uint32_t TransformCount;
};
struct SkinnedModelBuffer
{
    uint32_t ModelAddr;
    uint32_t ObjectCount;
    uint32_t* TransformAddr;
    uint32_t* SkeletonAddr;
};

class MaterialRenderStack
{
private:
    uint32_t            m_materialAddr;

    uint32_t            m_size;

    ModelBuffer*        m_modelBuffers;
    uint32_t            m_modelBufferCount;

    SkinnedModelBuffer* m_skinnedModelBuffers;
    uint32_t            m_skinnedModelBufferCount;

    void InsertTransform(uint32_t a_addr, uint32_t a_transformAddr);
    void InsertSkinned(uint32_t a_addr, uint32_t a_transformAddr, uint32_t a_skeletonAddr);

    void RemoveModelBuffer(uint32_t a_addr);

protected:

public:
    MaterialRenderStack(const MeshRenderBuffer& a_renderBuffer);
    MaterialRenderStack(const SkinnedMeshRenderBuffer& a_renderBuffer);
    ~MaterialRenderStack();

    inline bool Empty()
    {
        return m_size == 0;
    }

    inline uint32_t GetMaterialAddr() const
    {
        return m_materialAddr;
    }

    inline const ModelBuffer* GetModelBuffers() const
	{
		return m_modelBuffers;
	}
    inline uint32_t GetModelBufferCount() const
    {
        return m_modelBufferCount;
    }

    inline const SkinnedModelBuffer* GetSkinnedModelBuffers() const
    {
        return m_skinnedModelBuffers;
    }
    inline uint32_t GetSkinnedModelBufferCount() const
    {
        return m_skinnedModelBufferCount;
    }

    bool Add(const MeshRenderBuffer& a_renderBuffer);
    bool Remove(const MeshRenderBuffer& a_renderBuffer);

    bool Add(const SkinnedMeshRenderBuffer& a_renderBuffer);
    bool Remove(const SkinnedMeshRenderBuffer& a_renderBuffer);
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