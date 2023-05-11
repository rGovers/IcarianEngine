#include "Rendering/MaterialRenderStack.h"

#include "Rendering/MeshRenderBuffer.h"

MaterialRenderStack::MaterialRenderStack(const MeshRenderBuffer& a_renderBuffer)
{
    m_materialAddr = a_renderBuffer.MaterialAddr;

    ModelBuffer buffer;
    buffer.ModelAddr = a_renderBuffer.ModelAddr;
    buffer.TransformAddr.emplace_back(a_renderBuffer.TransformAddr);

    m_modelBuffers.emplace_back(buffer);
}
MaterialRenderStack::~MaterialRenderStack()
{

}

bool MaterialRenderStack::Add(const MeshRenderBuffer& a_renderBuffer)
{
    if (m_materialAddr != a_renderBuffer.MaterialAddr)
    {
        return false;
    }

    const uint32_t bufferSize = (uint32_t)m_modelBuffers.size();
    for (uint32_t i = 0; i < bufferSize; ++i)
    {
        if (m_modelBuffers[i].ModelAddr == a_renderBuffer.ModelAddr)
        {
            m_modelBuffers[i].TransformAddr.emplace_back(a_renderBuffer.TransformAddr);

            return true;
        }
    }

    ModelBuffer buffer;
    buffer.ModelAddr = a_renderBuffer.ModelAddr;
    buffer.TransformAddr.emplace_back(a_renderBuffer.TransformAddr);

    m_modelBuffers.emplace_back(buffer);

    return true;
}
bool MaterialRenderStack::Remove(const MeshRenderBuffer& a_renderBuffer)
{
    if (m_materialAddr != a_renderBuffer.MaterialAddr)
    {
        return false;
    }

    for (auto iter = m_modelBuffers.begin(); iter != m_modelBuffers.end(); ++iter)
    {
        if (iter->ModelAddr == a_renderBuffer.ModelAddr)
        {
            for (auto iIter = iter->TransformAddr.begin(); iIter != iter->TransformAddr.end(); ++iIter)
            {
                if (*iIter == a_renderBuffer.TransformAddr)
                {
                    iter->TransformAddr.erase(iIter);
                    if (iter->TransformAddr.empty())
                    {
                        m_modelBuffers.erase(iter);
                    }

                    return true;
                }
            }

            return false;
        }
    }

    return false;
}