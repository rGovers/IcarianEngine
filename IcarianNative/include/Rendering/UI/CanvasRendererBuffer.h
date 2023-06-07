#pragma once

#include <cstdint>

struct CanvasRendererBuffer
{
    static constexpr uint32_t DestroyedBit = 0;

    uint32_t CanvasAddr;
    uint32_t RenderTextureAddr;
    unsigned char Flags;

    inline bool IsDestroyed() const 
    { 
        return Flags & 0b1 << DestroyedBit;
    }
};