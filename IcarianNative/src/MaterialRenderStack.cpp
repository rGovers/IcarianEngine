#include "Rendering/MaterialRenderStack.h"

#include "Flare/IcarianDefer.h"
#include "Rendering/MeshRenderBuffer.h"

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
}

void MaterialRenderStack::InsertTransform(uint32_t a_addr, uint32_t a_transformAddr)
{
    const uint32_t transformCount = m_modelBuffers[a_addr].TransformCount;

    for (uint32_t i = 0; i < transformCount; ++i)
	{
		if (m_modelBuffers[a_addr].TransformAddr[i] == -1)
		{
            m_modelBuffers[a_addr].TransformAddr[i] = a_transformAddr;

			return;
		}
	}

    const uint32_t* oldTransformAddr = m_modelBuffers[a_addr].TransformAddr;
    IDEFER(delete[] oldTransformAddr);
    uint32_t* transformAddr = new uint32_t[transformCount + 1];

    for (uint32_t i = 0; i < transformCount; ++i)
    {
        transformAddr[i] = oldTransformAddr[i];
    }
    transformAddr[transformCount] = a_transformAddr;

    m_modelBuffers[a_addr].TransformAddr = transformAddr;
    m_modelBuffers[a_addr].TransformCount++;
}
bool MaterialRenderStack::Add(const MeshRenderBuffer& a_renderBuffer)
{
    if (m_materialAddr != a_renderBuffer.MaterialAddr)
    {
        return false;
    }

    const uint32_t bufferSize = (uint32_t)m_modelBufferCount;
    for (uint32_t i = 0; i < bufferSize; ++i)
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

                    return true;
                }
            }

            return false;
        }
    }

    return false;
}