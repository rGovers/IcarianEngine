// Icarian Engine - C# Game Engine
// 
// License at end of file.

#include "Rendering/MaterialRenderStack.h"

#include "Core/IcarianDefer.h"
#include "Core/IcarianLambda.h"
#include "DataTypes/Allocator.h"
#include "IcarianError.h"
#include "Rendering/RenderBuffers.h"

MaterialRenderStack::MaterialRenderStack(Allocator* a_allocator, const ModelRenderBuffer& a_renderBuffer)
{
    m_allocator = a_allocator;

    m_materialAddr = a_renderBuffer.MaterialAddr;

    const ModelBuffer buffer =
    {
        .ModelAddr = a_renderBuffer.ModelAddr,
        .TransformCount = 1,
        .TransformAddr = ILAMBDA(
        {
            uint32_t* vals = m_allocator->TAllocate<uint32_t>(1);

            vals[0] = a_renderBuffer.TransformAddr;

            ILRETURN vals;
        }),
    };

    m_modelBufferCount = 1;
    m_modelBuffers = m_allocator->TAllocate<ModelBuffer>(1);
    m_modelBuffers[0] = buffer;

    m_size = 1;

    m_renderStackMode = RenderStackMode_Model;
}
MaterialRenderStack::MaterialRenderStack(Allocator* a_allocator, const SkinnedModelRenderBuffer& a_renderBuffer)
{
    m_allocator = a_allocator;

    m_materialAddr = a_renderBuffer.MaterialAddr;

    const ModelBuffer buffer =
    {
        .ModelAddr = a_renderBuffer.ModelAddr,
        .TransformCount = 1,
        .TransformAddr = ILAMBDA(
        {
            uint32_t* vals = m_allocator->TAllocate<uint32_t>(1);

            vals[0] = a_renderBuffer.TransformAddr;

            ILRETURN vals;
        }),
        .SkeletonAddr = ILAMBDA(
        {
            uint32_t* vals = m_allocator->TAllocate<uint32_t>(1);

            vals[0] = a_renderBuffer.SkeletonAddr;

            ILRETURN vals;
        })
    };

    m_modelBufferCount = 1;
    m_modelBuffers = m_allocator->TAllocate<ModelBuffer>(1);
    m_modelBuffers[0] = buffer;

    m_size = 1;

    m_renderStackMode = RenderStackMode_Skinned;
}
MaterialRenderStack::MaterialRenderStack(Allocator* a_allocator, const MeshRenderBuffer& a_renderBuffer)
{
    m_allocator = a_allocator;

    m_materialAddr = a_renderBuffer.MaterialAddr;

    const ModelBuffer buffer =
    {
        .IndexCount = a_renderBuffer.IndexCount,
        .ModelAddr = a_renderBuffer.MeshAddr,
        .TransformCount = 1,
        .TransformAddr = ILAMBDA(
        {
            uint32_t* vals = m_allocator->TAllocate<uint32_t>(1);

            vals[0] = a_renderBuffer.TransformAddr;

            ILRETURN vals;
        })
    };

    m_modelBufferCount = 1;
    m_modelBuffers = m_allocator->TAllocate<ModelBuffer>(1);
    m_modelBuffers[0] = buffer;

    m_size = 1;

    m_renderStackMode = RenderStackMode_Mesh;
}
MaterialRenderStack::~MaterialRenderStack()
{
    for (uint32_t i = 0; i < m_modelBufferCount; ++i)
    {
        if (m_modelBuffers[i].TransformAddr != nullptr)
		{
			m_allocator->Free(m_modelBuffers[i].TransformAddr);
		}
    }

    m_allocator->Free(m_modelBuffers);
}

bool MaterialRenderStack::InsertTransform(uint32_t a_addr, uint32_t a_transformAddr)
{
    ModelBuffer& buffer = m_modelBuffers[a_addr];

    const uint32_t transformCount = buffer.TransformCount;
    for (uint32_t i = 0; i < transformCount; ++i)
	{
		if (buffer.TransformAddr[i] != uint32_t(-1))
		{
            continue;
        }

        buffer.TransformAddr[i] = a_transformAddr;

        return true;
	}

    if (buffer.TransformCount >= MaxTransformStackSize)
    {
        return false;
    }

    uint32_t* oldTransformAddr = buffer.TransformAddr;
    IDEFER(m_allocator->Free(oldTransformAddr));

    const uint32_t newSize = transformCount << 1;
    IDEFER(buffer.TransformCount = newSize);

    buffer.TransformAddr = m_allocator->TAllocate<uint32_t>(newSize);

    for (uint32_t i = 0; i < transformCount; ++i)
    {
        buffer.TransformAddr[i] = oldTransformAddr[i];
    }

    buffer.TransformAddr[transformCount] = a_transformAddr;

    for (uint32_t i = transformCount + 1; i < newSize; ++i)
    {
        buffer.TransformAddr[i] = uint32_t(-1);
    }

    return true;
}
void MaterialRenderStack::RemoveModelBuffer(uint32_t a_addr)
{
    m_allocator->Destroy(m_modelBuffers[a_addr].TransformAddr);

    --m_modelBufferCount;

    for (uint32_t i = a_addr; i < m_modelBufferCount; ++i)
    {
        m_modelBuffers[i] = m_modelBuffers[i + 1];
    }
}

bool MaterialRenderStack::Add(const ModelRenderBuffer& a_renderBuffer)
{
    if (m_renderStackMode != RenderStackMode_Model)
    {
        return false;
    }

    if (m_materialAddr != a_renderBuffer.MaterialAddr)
    {
        return false;
    }

    for (uint32_t i = 0; i < m_modelBufferCount; ++i)
    {
        if (m_modelBuffers[i].ModelAddr != a_renderBuffer.ModelAddr)
        {
            continue;
        }

        const bool validInsertion = InsertTransform(i, a_renderBuffer.TransformAddr);
        if (!validInsertion)
        {
            continue;
        }

        ++m_size;

        return true;
    }

    if (m_modelBufferCount >= MaxRenderStackSize)
    {
        return false;
    }

    const ModelBuffer buffer =
    {
        .ModelAddr = a_renderBuffer.ModelAddr,
        .TransformCount = 1,
        .TransformAddr = ILAMBDA(
        {
            uint32_t* vals = m_allocator->TAllocate<uint32_t>(1);

            vals[0] = a_renderBuffer.TransformAddr;

            ILRETURN vals;
        })
    };

    ModelBuffer* oldModelBuffers = m_modelBuffers;
    IDEFER(m_allocator->Destroy(oldModelBuffers));

    m_modelBuffers = m_allocator->TAllocate<ModelBuffer>(m_modelBufferCount + 1);
    for (uint32_t i = 0; i < m_modelBufferCount; ++i)
    {
        m_modelBuffers[i] = oldModelBuffers[i];
    }
    m_modelBuffers[m_modelBufferCount++] = buffer;

    ++m_size;

    return true;
}
bool MaterialRenderStack::Remove(const ModelRenderBuffer& a_renderBuffer)
{
    if (m_renderStackMode != RenderStackMode_Model)
    {
        return false;
    }

    if (m_materialAddr != a_renderBuffer.MaterialAddr)
    {
        return false;
    }

    for (uint32_t i = 0; i < m_modelBufferCount; ++i)
    {
        ModelBuffer& buffer = m_modelBuffers[i];

        if (buffer.ModelAddr != a_renderBuffer.ModelAddr)
        {
            continue;
        }

        const uint32_t transformCount = buffer.TransformCount;
        for (uint32_t j = 0; j < transformCount; ++j)
        {
            if (buffer.TransformAddr[j] != a_renderBuffer.TransformAddr)
            {
                continue;
            }

            buffer.TransformAddr[j] = -1;

            if (--m_size == 0)
            {
                return true;
            }

            for (uint32_t k = 0; k < transformCount; ++k)
            {
                if (m_modelBuffers[i].TransformAddr[k] != uint32_t(-1))
                {
                    return true;
                }
            }

            RemoveModelBuffer(i);

            return true;
        }

        return false;
    }

    return false;
}

bool MaterialRenderStack::InsertSkinned(uint32_t a_addr, uint32_t a_transformAddr, uint32_t a_skeletonAddr)
{
    ModelBuffer& buffer = m_modelBuffers[a_addr];

    const uint32_t objectCount = buffer.TransformCount;

    for (uint32_t i = 0; i < objectCount; ++i)
    {
        if (buffer.TransformAddr[i] != uint32_t(-1))
        {
            continue;
        }

        IVERIFY(buffer.SkeletonAddr[i] == uint32_t(-1));

        buffer.TransformAddr[i] = a_transformAddr;
        buffer.SkeletonAddr[i] = a_skeletonAddr;

        return true;
    }

    if (buffer.TransformCount >= MaxTransformStackSize)
    {
        return false;
    }

    uint32_t* oldTransformAddr = buffer.TransformAddr;
    IDEFER(m_allocator->Free(oldTransformAddr));
    uint32_t* oldSkeletonAddr = buffer.SkeletonAddr;
    IDEFER(m_allocator->Free(oldSkeletonAddr));

    const uint32_t newSize = objectCount << 1;
    IDEFER(buffer.TransformCount = newSize);

    buffer.TransformAddr = m_allocator->TAllocate<uint32_t>(newSize);
    buffer.SkeletonAddr = m_allocator->TAllocate<uint32_t>(newSize);

    for (uint32_t i = 0; i < objectCount; ++i)
    {
        buffer.TransformAddr[i] = oldTransformAddr[i];
        buffer.SkeletonAddr[i] = oldSkeletonAddr[i];
    }

    buffer.TransformAddr[objectCount] = a_transformAddr;
    buffer.SkeletonAddr[objectCount] = a_skeletonAddr;

    for (uint32_t i = objectCount + 1; i < newSize; ++i)
    {
        buffer.TransformAddr[i] = uint32_t(-1);
        buffer.SkeletonAddr[i] = uint32_t(-1);
    }

    return true;
}

bool MaterialRenderStack::Add(const SkinnedModelRenderBuffer& a_renderBuffer)
{
    if (m_renderStackMode != RenderStackMode_Skinned)
    {
        return false;
    }

    if (m_materialAddr != a_renderBuffer.MaterialAddr)
    {
        return false;
    }

    for (uint32_t i = 0; i < m_modelBufferCount; ++i)
    {
        if (m_modelBuffers[i].ModelAddr != a_renderBuffer.ModelAddr)
        {
            continue;
        }

        const bool validInsertion = InsertSkinned(i, a_renderBuffer.TransformAddr, a_renderBuffer.SkeletonAddr);
        if (!validInsertion)
        {
            continue;
        }

        ++m_size;

        return true;
    }

    if (m_modelBufferCount >= MaxRenderStackSize)
    {
        return false;
    }

    const ModelBuffer buffer = 
    {
        .ModelAddr = a_renderBuffer.ModelAddr,
        .TransformCount = 1,
        .TransformAddr = ILAMBDA(
        {
            uint32_t* vals = m_allocator->TAllocate<uint32_t>(1);

            vals[0] = a_renderBuffer.TransformAddr;

            ILRETURN vals;
        }),
        .SkeletonAddr = ILAMBDA(
        {
            uint32_t* vals = m_allocator->TAllocate<uint32_t>(1);

            vals[0] = a_renderBuffer.TransformAddr;

            ILRETURN vals;
        })
    };

    ModelBuffer* oldModelBuffers = m_modelBuffers;
    IDEFER(m_allocator->Free(oldModelBuffers));

    m_modelBuffers = m_allocator->TAllocate<ModelBuffer>(m_modelBufferCount + 1);
    for (uint32_t i = 0; i < m_modelBufferCount; ++i)
    {
        m_modelBuffers[i] = oldModelBuffers[i];
    }
    m_modelBuffers[m_modelBufferCount++] = buffer;

    ++m_size;

    return true;
}
bool MaterialRenderStack::Remove(const SkinnedModelRenderBuffer& a_renderBuffer)
{
    if (m_renderStackMode != RenderStackMode_Skinned)
    {
        return false;
    }

    if (m_materialAddr != a_renderBuffer.MaterialAddr)
    {
        return false;
    }

    for (uint32_t i = 0; i < m_modelBufferCount; ++i)
    {
        ModelBuffer& buffer = m_modelBuffers[i];

        if (buffer.ModelAddr != a_renderBuffer.ModelAddr)
        {
            continue;
        }

        const uint32_t objectCount = buffer.TransformCount;

        for (uint32_t j = 0; j < objectCount; ++j)
        {
            if (buffer.TransformAddr[j] != a_renderBuffer.TransformAddr)
            {
                continue;
            }

            buffer.SkeletonAddr[j] = -1;
            buffer.TransformAddr[j] = -1;

            if (--m_size == 0)
            {
                return true;
            }

            for (uint32_t k = 0; k < objectCount; ++k)
            {
                if (m_modelBuffers[i].TransformAddr[k] != uint32_t(-1))
                {
                    return true;
                }
            }

            RemoveModelBuffer(i);

            return true;
        }

        return false;
    }

    return false;
}

bool MaterialRenderStack::Add(const MeshRenderBuffer& a_renderBuffer)
{
    if (m_renderStackMode != RenderStackMode_Mesh)
    {
        return false;
    }

    if (m_materialAddr != a_renderBuffer.MaterialAddr)
    {
        return false;
    }

    for (uint32_t i = 0; i < m_modelBufferCount; ++i)
    {
        if (m_modelBuffers[i].ModelAddr != a_renderBuffer.MeshAddr)
        {
            continue;
        }

        if (m_modelBuffers[i].IndexCount != a_renderBuffer.IndexCount)
        {
            continue;
        }

        const bool validInsertion = InsertTransform(i, a_renderBuffer.TransformAddr);
        if (!validInsertion)
        {
            continue;
        }

        ++m_size;

        return true;
    }

    if (m_modelBufferCount >= MaxRenderStackSize)
    {
        return false;
    }

    const ModelBuffer buffer = 
    {
        .IndexCount = a_renderBuffer.IndexCount,
        .ModelAddr = a_renderBuffer.MeshAddr,
        .TransformCount = 1,
        .TransformAddr = ILAMBDA(
        {
            uint32_t* vals = m_allocator->TAllocate<uint32_t>(1);

            vals[0] = a_renderBuffer.TransformAddr;

            ILRETURN vals;
        }),
    };

    ModelBuffer* oldModelBuffers = m_modelBuffers;
    IDEFER(m_allocator->Free(oldModelBuffers));

    m_modelBuffers = m_allocator->TAllocate<ModelBuffer>(m_modelBufferCount + 1);
    for (uint32_t i = 0; i < m_modelBufferCount; ++i)
    {
        m_modelBuffers[i] = oldModelBuffers[i];
    }
    m_modelBuffers[m_modelBufferCount++] = buffer;

    ++m_size;

    return true;
}
bool MaterialRenderStack::Remove(const MeshRenderBuffer& a_renderBuffer)
{
    if (m_renderStackMode != RenderStackMode_Mesh)
    {
        return false;
    }

    if (m_materialAddr != a_renderBuffer.MaterialAddr)
    {
        return false;
    }

    for (uint32_t i = 0; i < m_modelBufferCount; ++i)
    {
        ModelBuffer& buffer = m_modelBuffers[i];

        if (buffer.ModelAddr != a_renderBuffer.MeshAddr)
        {
            continue;
        }

        if (buffer.IndexCount != a_renderBuffer.IndexCount)
        {
            continue;
        }

        const uint32_t objectCount = buffer.TransformCount;

        for (uint32_t j = 0; j < objectCount; ++j)
        {
            if (buffer.TransformAddr[j] != a_renderBuffer.TransformAddr)
            {
                continue;
            }

            buffer.TransformAddr[j] = -1;

            if (--m_size == 0)
            {
                return true;
            }

            for (uint32_t k = 0; k < objectCount; ++k)
            {
                if (m_modelBuffers[i].TransformAddr[k] != uint32_t(-1))
                {
                    return true;
                }
            }

            RemoveModelBuffer(i);

            return true;
        }

        return false;
    }

    return false;
}

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