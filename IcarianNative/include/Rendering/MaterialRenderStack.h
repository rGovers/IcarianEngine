// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#include <cstdint>

class Allocator;
struct MeshRenderBuffer;
struct ModelRenderBuffer;
struct SkinnedModelRenderBuffer;

struct ModelBuffer
{
    uint32_t IndexCount;
    uint32_t ModelAddr;
    uint32_t TransformCount;
    uint32_t* TransformAddr;
    uint32_t* SkeletonAddr;
};

enum e_RenderStackMode
{
    RenderStackMode_Null,
    RenderStackMode_Model,
    RenderStackMode_Skinned,
    RenderStackMode_Mesh
};

class MaterialRenderStack
{
private:
    Allocator*          m_allocator;

    ModelBuffer*        m_modelBuffers;
    
    uint32_t            m_materialAddr;
    
    uint32_t            m_size;
    uint32_t            m_modelBufferCount;
    
    e_RenderStackMode   m_renderStackMode;

    void InsertTransform(uint32_t a_addr, uint32_t a_transformAddr);
    void InsertSkinned(uint32_t a_addr, uint32_t a_transformAddr, uint32_t a_skeletonAddr);

    void RemoveModelBuffer(uint32_t a_addr);

protected:

public:
    MaterialRenderStack(Allocator* a_allocator, const ModelRenderBuffer& a_renderBuffer);
    MaterialRenderStack(Allocator* a_allocator, const SkinnedModelRenderBuffer& a_renderBuffer);
    MaterialRenderStack(Allocator* a_allocator, const MeshRenderBuffer& a_renderBuffer);
    ~MaterialRenderStack();

    inline bool Empty()
    {
        return m_size == 0;
    }

    inline e_RenderStackMode GetRenderStackMode() const
    {
        return m_renderStackMode;
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

    bool Add(const ModelRenderBuffer& a_renderBuffer);
    bool Remove(const ModelRenderBuffer& a_renderBuffer);

    bool Add(const SkinnedModelRenderBuffer& a_renderBuffer);
    bool Remove(const SkinnedModelRenderBuffer& a_renderBuffer);

    bool Add(const MeshRenderBuffer& a_renderBuffer);
    bool Remove(const MeshRenderBuffer& a_renderBuffer);
};

// MIT License
// 
// Copyright (c) 2025 River Govers
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