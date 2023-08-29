#pragma once

#include <cstdint>

struct SkinnedMeshRenderBuffer
{
    uint32_t SkeletonAddr;
    uint32_t MaterialAddr;
    uint32_t ModelAddr;
    uint32_t TransformAddr;

    constexpr SkinnedMeshRenderBuffer(uint32_t a_skeletonAddr = -1, uint32_t a_materialAddr = -1, uint32_t a_modelAddr = -1, uint32_t a_transformAddr = -1) :
        SkeletonAddr(a_skeletonAddr),
        MaterialAddr(a_materialAddr),
        ModelAddr(a_modelAddr),
        TransformAddr(a_transformAddr)
    {
        
    }
};
