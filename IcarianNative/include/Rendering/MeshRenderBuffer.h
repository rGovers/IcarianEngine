#pragma once

#include <cstdint>

struct MeshRenderBuffer
{
    uint32_t MaterialAddr;
    uint32_t ModelAddr;
    uint32_t TransformAddr;

    constexpr MeshRenderBuffer(uint32_t a_materialAddr = -1, uint32_t a_modelAddr = -1, uint32_t a_transformAddr = -1) :
        MaterialAddr(a_materialAddr),
        ModelAddr(a_modelAddr),
        TransformAddr(a_transformAddr)
    {
        
    }
};
