#pragma once

#include <cstdint>

namespace FlareBase
{
    enum e_TextureMode
    {
        TextureMode_Null = -1,
        TextureMode_Texture,
        TextureMode_RenderTexture,
        TextureMode_RenderTextureDepth
    };
    
    enum e_TextureFilter
    {
        TextureFilter_Nearest = 0,
        TextureFilter_Linear = 1
    };
    
    enum e_TextureAddress
    {
        TextureAddress_Repeat = 0,
        TextureAddress_MirroredRepeat = 1,
        TextureAddress_ClampToEdge = 2
    };
    
    struct TextureSampler
    {
        uint32_t Addr;
        uint32_t TSlot;
        e_TextureMode TextureMode;
        e_TextureFilter FilterMode;
        e_TextureAddress AddressMode;
        void* Data;
    };
}
