#pragma once

#include "InteropTypes.h"

#ifdef CUBE_LANGUAGE_CSHARP
namespace IcarianEngine.Rendering {
#endif

IOP_CSPUBLIC enum IOP_ENUM_NAME(TextureFilter) : IOP_UINT16
{
    IOP_ENUM_VALUE(TextureFilter, Nearest) = 0,
    IOP_ENUM_VALUE(TextureFilter, Linear) = 1
};

IOP_CSPUBLIC enum IOP_ENUM_NAME(TextureAddress) : IOP_UINT16
{
    IOP_ENUM_VALUE(TextureAddress, Repeat) = 0,
    IOP_ENUM_VALUE(TextureAddress, MirroredRepeat) = 1,
    IOP_ENUM_VALUE(TextureAddress, ClampToEdge) = 2
};

/// @cond INTERNAL

IOP_CSINTERNAL enum IOP_ENUM_NAME(TextureMode) : IOP_UINT16
{
    IOP_ENUM_VALUE(TextureMode, Null) = IOP_UINT16_MAX,
    IOP_ENUM_VALUE(TextureMode, Texture) = 0,
    IOP_ENUM_VALUE(TextureMode, RenderTexture) = 1,
    IOP_ENUM_VALUE(TextureMode, RenderTextureDepth) = 2,
    IOP_ENUM_VALUE(TextureMode, DepthRenderTexture) = 3,
    IOP_ENUM_VALUE(TextureMode, DepthCubeRenderTexture) = 4
};

IOP_PACKED IOP_CSINTERNAL struct TextureSamplerBuffer
{
    IOP_UINT32 Addr;
    IOP_UINT32 Slot;
    IOP_ENUM_NAME(TextureMode) TextureMode;
    IOP_ENUM_NAME(TextureFilter) FilterMode;
    IOP_ENUM_NAME(TextureAddress) AddressMode;
    IOP_POINTER(void*) Data;
};

/// @endcond

#ifdef CUBE_LANGUAGE_CSHARP
}
#endif