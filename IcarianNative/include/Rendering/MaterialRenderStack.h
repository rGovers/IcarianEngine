#pragma once

#include <cstdint>
#include <vector>

struct MeshRenderBuffer;

struct ModelBuffer
{
    uint32_t ModelAddr;
    uint32_t* TransformAddr;
    uint32_t TransformCount;
};

class MaterialRenderStack
{
private:
    uint32_t     m_materialAddr;

    ModelBuffer* m_modelBuffers;
    uint32_t     m_modelBufferCount;

    void InsertTransform(uint32_t a_addr, uint32_t a_transformAddr);
protected:

public:
    MaterialRenderStack(const MeshRenderBuffer& a_renderBuffer);
    ~MaterialRenderStack();

    inline bool Empty()
    {
        return m_modelBufferCount == 0;
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

    bool Add(const MeshRenderBuffer& a_renderBuffer);
    bool Remove(const MeshRenderBuffer& a_renderBuffer);
};