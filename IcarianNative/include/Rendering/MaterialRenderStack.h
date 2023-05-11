#pragma once

#include <cstdint>
#include <vector>

struct MeshRenderBuffer;

struct ModelBuffer
{
    uint32_t ModelAddr;
    std::vector<uint32_t> TransformAddr;
};

class MaterialRenderStack
{
private:
    uint32_t                 m_materialAddr;

    std::vector<ModelBuffer> m_modelBuffers;
    
protected:

public:
    MaterialRenderStack(const MeshRenderBuffer& a_renderBuffer);
    ~MaterialRenderStack();

    inline bool Empty()
    {
        return m_modelBuffers.empty();
    }

    inline uint32_t GetMaterialAddr() const
    {
        return m_materialAddr;
    }

    inline const std::vector<ModelBuffer>& GetModelBuffers() const
    {
        return m_modelBuffers;
    }

    bool Add(const MeshRenderBuffer& a_renderBuffer);
    bool Remove(const MeshRenderBuffer& a_renderBuffer);
};