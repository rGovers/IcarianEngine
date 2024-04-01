#include "Rendering/MaterialRenderStack.h"

#include "Flare/IcarianAssert.h"
#include "Flare/IcarianDefer.h"
#include "Rendering/MeshRenderBuffer.h"
#include "Rendering/SkinnedMeshRenderBuffer.h"

MaterialRenderStack::MaterialRenderStack(const MeshRenderBuffer& a_renderBuffer)
{
    m_materialAddr = a_renderBuffer.MaterialAddr;

    ModelBuffer buffer;
    buffer.ModelAddr = a_renderBuffer.ModelAddr;
    buffer.TransformAddr = new uint32_t[1];
    buffer.TransformAddr[0] = a_renderBuffer.TransformAddr;
    buffer.TransformCount = 1;

    m_modelBufferCount = 1;
    m_modelBuffers = new ModelBuffer[1];
    m_modelBuffers[0] = buffer;

    m_skinnedModelBufferCount = 0;
    m_skinnedModelBuffers = nullptr;

    m_size = 1;
}
MaterialRenderStack::MaterialRenderStack(const SkinnedMeshRenderBuffer& a_renderBuffer)
{
    m_materialAddr = a_renderBuffer.MaterialAddr;

    SkinnedModelBuffer buffer;
    buffer.ModelAddr = a_renderBuffer.ModelAddr;
    buffer.TransformAddr = new uint32_t[1];
    buffer.TransformAddr[0] = a_renderBuffer.TransformAddr;
    buffer.SkeletonAddr = new uint32_t[1];
    buffer.SkeletonAddr[0] = a_renderBuffer.SkeletonAddr;
    buffer.ObjectCount = 1;

    m_skinnedModelBufferCount = 1;
    m_skinnedModelBuffers = new SkinnedModelBuffer[1];
    m_skinnedModelBuffers[0] = buffer;

    m_modelBufferCount = 0;
    m_modelBuffers = nullptr;

    m_size = 1;
}
MaterialRenderStack::~MaterialRenderStack()
{
    for (uint32_t i = 0; i < m_modelBufferCount; ++i)
    {
        if (m_modelBuffers[i].TransformAddr != nullptr)
		{
			delete[] m_modelBuffers[i].TransformAddr;
		}
    }

    delete[] m_modelBuffers;

    for (uint32_t i = 0; i < m_skinnedModelBufferCount; ++i)
    {
        if (m_skinnedModelBuffers[i].TransformAddr != nullptr)
        {
            delete[] m_skinnedModelBuffers[i].TransformAddr;
        }
        
        if (m_skinnedModelBuffers[i].SkeletonAddr != nullptr)
        {
            delete[] m_skinnedModelBuffers[i].SkeletonAddr;
        }
    }

    delete[] m_skinnedModelBuffers;
}

void MaterialRenderStack::InsertTransform(uint32_t a_addr, uint32_t a_transformAddr)
{
    ModelBuffer& buffer = m_modelBuffers[a_addr];

    const uint32_t transformCount = buffer.TransformCount;
    for (uint32_t i = 0; i < transformCount; ++i)
	{
		if (buffer.TransformAddr[i] == -1)
		{
            buffer.TransformAddr[i] = a_transformAddr;

			return;
		}
	}

    const uint32_t* oldTransformAddr = buffer.TransformAddr;
    IDEFER(delete[] oldTransformAddr);
    buffer.TransformAddr = new uint32_t[transformCount + 1];

    for (uint32_t i = 0; i < transformCount; ++i)
    {
        buffer.TransformAddr[i] = oldTransformAddr[i];
    }

    buffer.TransformAddr[transformCount] = a_transformAddr;
    buffer.TransformCount++;
}
void MaterialRenderStack::RemoveModelBuffer(uint32_t a_addr)
{
    delete[] m_modelBuffers[a_addr].TransformAddr;

    --m_modelBufferCount;

    for (uint32_t i = a_addr; i < m_modelBufferCount; ++i)
    {
        m_modelBuffers[i] = m_modelBuffers[i + 1];
    }
}

bool MaterialRenderStack::Add(const MeshRenderBuffer& a_renderBuffer)
{
    if (m_materialAddr != a_renderBuffer.MaterialAddr)
    {
        return false;
    }

    ++m_size;

    for (uint32_t i = 0; i < m_modelBufferCount; ++i)
    {
        if (m_modelBuffers[i].ModelAddr == a_renderBuffer.ModelAddr)
        {
            InsertTransform(i, a_renderBuffer.TransformAddr);

            return true;
        }
    }

    ModelBuffer buffer;
    buffer.ModelAddr = a_renderBuffer.ModelAddr;
    buffer.TransformAddr = new uint32_t[1];
    buffer.TransformAddr[0] = a_renderBuffer.TransformAddr;
    buffer.TransformCount = 1;

    const ModelBuffer* oldModelBuffers = m_modelBuffers;
    IDEFER(delete[] oldModelBuffers);

    const uint32_t bufferCount = m_modelBufferCount++;
    m_modelBuffers = new ModelBuffer[m_modelBufferCount];
    for (uint32_t i = 0; i < bufferCount; ++i)
    {
        m_modelBuffers[i] = oldModelBuffers[i];
    }
    m_modelBuffers[bufferCount] = buffer;

    return true;
}
bool MaterialRenderStack::Remove(const MeshRenderBuffer& a_renderBuffer)
{
    if (m_materialAddr != a_renderBuffer.MaterialAddr || m_modelBufferCount == 0)
    {
        return false;
    }

    for (uint32_t i = 0; i < m_modelBufferCount; ++i)
    {
        if (m_modelBuffers[i].ModelAddr == a_renderBuffer.ModelAddr)
        {
            const uint32_t transformCount = m_modelBuffers[i].TransformCount;
            for (uint32_t j = 0; j < transformCount; ++j)
            {
                if (m_modelBuffers[i].TransformAddr[j] == a_renderBuffer.TransformAddr)
                {
                    m_modelBuffers[i].TransformAddr[j] = -1;

                    m_size--;

                    if (m_size == 0)
                    {
                        return true;
                    }

                    for (uint32_t k = 0; k < transformCount; ++k)
                    {
                        if (m_modelBuffers[i].TransformAddr[k] != -1)
                        {
                            return true;
                        }
                    }

                    RemoveModelBuffer(i);

                    return true;
                }
            }

            return false;
        }
    }

    return false;
}

void MaterialRenderStack::InsertSkinned(uint32_t a_addr, uint32_t a_transformAddr, uint32_t a_skeletonAddr)
{
    SkinnedModelBuffer& buffer = m_skinnedModelBuffers[a_addr];

    const uint32_t objectCount = buffer.ObjectCount;

    for (uint32_t i = 0; i < objectCount; ++i)
    {
        if (buffer.TransformAddr[i] == -1)
        {
            ICARIAN_ASSERT_MSG(buffer.SkeletonAddr[i] == -1, "Skeleton address is not -1");

            buffer.TransformAddr[i] = a_transformAddr;
            buffer.SkeletonAddr[i] = a_skeletonAddr;

            return;
        }
    }

    const uint32_t* oldTransformAddr = buffer.TransformAddr;
    IDEFER(delete[] oldTransformAddr);
    const uint32_t* oldSkeletonAddr = buffer.SkeletonAddr;
    IDEFER(delete[] oldSkeletonAddr);

    buffer.TransformAddr = new uint32_t[objectCount + 1];
    buffer.SkeletonAddr = new uint32_t[objectCount + 1];

    for (uint32_t i = 0; i < objectCount; ++i)
    {
        buffer.TransformAddr[i] = oldTransformAddr[i];
        buffer.SkeletonAddr[i] = oldSkeletonAddr[i];
    }

    buffer.TransformAddr[objectCount] = a_transformAddr;
    buffer.SkeletonAddr[objectCount] = a_skeletonAddr;
    buffer.ObjectCount++;
}

bool MaterialRenderStack::Add(const SkinnedMeshRenderBuffer& a_renderBuffer)
{
    if (m_materialAddr != a_renderBuffer.MaterialAddr)
    {
        return false;
    }

    m_size++;

    for (uint32_t i = 0; i < m_skinnedModelBufferCount; ++i)
    {
        if (m_skinnedModelBuffers[i].ModelAddr == a_renderBuffer.ModelAddr)
        {
            InsertSkinned(i, a_renderBuffer.TransformAddr, a_renderBuffer.SkeletonAddr);

            return true;
        }
    }

    SkinnedModelBuffer buffer;
    buffer.ModelAddr = a_renderBuffer.ModelAddr;
    buffer.TransformAddr = new uint32_t[1];
    buffer.TransformAddr[0] = a_renderBuffer.TransformAddr;
    buffer.SkeletonAddr = new uint32_t[1];
    buffer.SkeletonAddr[0] = a_renderBuffer.SkeletonAddr;
    buffer.ObjectCount = 1;

    const SkinnedModelBuffer* oldSkinnedModelBuffers = m_skinnedModelBuffers;
    IDEFER(delete[] oldSkinnedModelBuffers);

    const uint32_t bufferCount = m_skinnedModelBufferCount++;
    m_skinnedModelBuffers = new SkinnedModelBuffer[m_skinnedModelBufferCount];
    for (uint32_t i = 0; i < bufferCount; ++i)
    {
        m_skinnedModelBuffers[i] = oldSkinnedModelBuffers[i];
    }
    m_skinnedModelBuffers[bufferCount] = buffer;

    return true;
}
bool MaterialRenderStack::Remove(const SkinnedMeshRenderBuffer& a_renderBuffer)
{
    if (m_materialAddr != a_renderBuffer.MaterialAddr || m_skinnedModelBufferCount == 0)
    {
        return false;
    }

    for (uint32_t i = 0; i < m_skinnedModelBufferCount; ++i)
    {
        if (m_skinnedModelBuffers[i].ModelAddr == a_renderBuffer.ModelAddr)
        {
            const uint32_t objectCount = m_skinnedModelBuffers[i].ObjectCount;
            for (uint32_t j = 0; j < objectCount; ++j)
            {
                if (m_skinnedModelBuffers[i].TransformAddr[j] == a_renderBuffer.TransformAddr)
                {
                    m_skinnedModelBuffers[i].SkeletonAddr[j] = -1;
                    m_skinnedModelBuffers[i].TransformAddr[j] = -1;

                    m_size--;

                    return true;
                }
            }

            return false;
        }
    }

    return false;
}