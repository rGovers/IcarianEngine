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