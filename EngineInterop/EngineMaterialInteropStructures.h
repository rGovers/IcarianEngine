// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#include "InteropTypes.h"

#ifdef CUBE_LANGUAGE_CPP 
#include "EngineModelInteropStructures.h"
#endif

#ifdef CUBE_LANGUAGE_CSHARP
namespace IcarianEngine.Rendering {
#endif

/// @file EngineMaterialInteropStructures.h

/// <summary>
/// Material blend mode enumeration
/// </summary>
IOP_CSPUBLIC enum IOP_ENUM_NAME(MaterialBlendMode) : IOP_UINT8
{
    IOP_ENUM_VALUE(MaterialBlendMode, None) = 0,
    IOP_ENUM_VALUE(MaterialBlendMode, One) = 1,
    IOP_ENUM_VALUE(MaterialBlendMode, Alpha) = 2,
    IOP_ENUM_VALUE(MaterialBlendMode, AlphaBlend) = 3
};

/// <summary>
/// Material mode enumeration
/// </summary>
IOP_CSPUBLIC enum IOP_ENUM_NAME(MaterialMode) : IOP_UINT8
{
    IOP_ENUM_VALUE(MaterialMode, BaseVertex) = 0,
    IOP_ENUM_VALUE(MaterialMode, BaseMesh) = 1,
    IOP_ENUM_VALUE(MaterialMode, Compute) = 2,
};

/// <summary>
/// Shader buffer type enumeration.
/// </summary>
IOP_CSPUBLIC enum IOP_ENUM_NAME(ShaderBufferType) : IOP_UINT16
{
    IOP_ENUM_VALUE(ShaderBufferType, Null) = IOP_UINT16_MAX,
    IOP_ENUM_VALUE(ShaderBufferType, PModelBuffer) = 0,
    IOP_ENUM_VALUE(ShaderBufferType, PUIBuffer) = 1,
    IOP_ENUM_VALUE(ShaderBufferType, PShadowLightBuffer) = 2,
    IOP_ENUM_VALUE(ShaderBufferType, CameraBuffer) = 3,
    IOP_ENUM_VALUE(ShaderBufferType, TimeBuffer) = 4,
    IOP_ENUM_VALUE(ShaderBufferType, DirectionalLightBuffer) = 5,
    IOP_ENUM_VALUE(ShaderBufferType, PointLightBuffer) = 6,
    IOP_ENUM_VALUE(ShaderBufferType, SpotLightBuffer) = 7,
    IOP_ENUM_VALUE(ShaderBufferType, AmbientLightBuffer) = 8,
    IOP_ENUM_VALUE(ShaderBufferType, BufferTexture) = 9,
    IOP_ENUM_VALUE(ShaderBufferType, Texture) = 10,
    IOP_ENUM_VALUE(ShaderBufferType, PushTexture) = 11,
    IOP_ENUM_VALUE(ShaderBufferType, ShadowLightBuffer) = 12,
    IOP_ENUM_VALUE(ShaderBufferType, ShadowTexture2D) = 13, 
    IOP_ENUM_VALUE(ShaderBufferType, ShadowTextureCube) = 14,
    IOP_ENUM_VALUE(ShaderBufferType, UserUBO) = 15,
    IOP_ENUM_VALUE(ShaderBufferType, UserArray) = 16,
    IOP_ENUM_VALUE(ShaderBufferType, SSModelBuffer) = 17,
    IOP_ENUM_VALUE(ShaderBufferType, SSBoneBuffer) = 18,
    IOP_ENUM_VALUE(ShaderBufferType, SSDirectionalLightBuffer) = 19,
    IOP_ENUM_VALUE(ShaderBufferType, SSPointLightBuffer) = 20,
    IOP_ENUM_VALUE(ShaderBufferType, SSSpotLightBuffer) = 21,
    IOP_ENUM_VALUE(ShaderBufferType, SSAmbientLightBuffer) = 22,
    IOP_ENUM_VALUE(ShaderBufferType, SSShadowLightBuffer) = 23,
    IOP_ENUM_VALUE(ShaderBufferType, SSParticleBuffer) = 24,
    IOP_ENUM_VALUE(ShaderBufferType, AShadowTexture2D) = 25,
    IOP_ENUM_VALUE(ShaderBufferType, MeshVertex) = 26,
    IOP_ENUM_VALUE(ShaderBufferType, MeshletVertices) = 27,
    IOP_ENUM_VALUE(ShaderBufferType, MeshletTriangles) = 28,
    IOP_ENUM_VALUE(ShaderBufferType, Meshlet) = 29,

#ifdef CUBE_LANGUAGE_CPP
    IOP_ENUM_VALUE(ShaderBufferType, OutMeshEmulationVertex) = 30,
    IOP_ENUM_VALUE(ShaderBufferType, OutMeshEmulationIndex) = 31,
    IOP_ENUM_VALUE(ShaderBufferType, OutMeshEmulationDraw) = 32,
#endif
};

/// <summary>
/// Culling mode enumeration.
/// </summary>
IOP_CSPUBLIC enum IOP_ENUM_NAME(CullMode) : IOP_UINT8
{
    IOP_ENUM_VALUE(CullMode, None) = 0,
    IOP_ENUM_VALUE(CullMode, Front) = 0b1 << 0,
    IOP_ENUM_VALUE(CullMode, Back) = 0b1 << 1,
    IOP_ENUM_VALUE(CullMode, Both) = IOP_ENUM_VALUE(CullMode, Front) | IOP_ENUM_VALUE(CullMode, Back)
};

/// <summary>
/// Primitive mode enumeration.
/// </summary>
IOP_CSPUBLIC enum IOP_ENUM_NAME(PrimitiveMode) : IOP_UINT8
{
    IOP_ENUM_VALUE(PrimitiveMode, Triangles) = 0,
    IOP_ENUM_VALUE(PrimitiveMode, TriangleStrip) = 1
};

/// @cond INTERNAL
IOP_PACKED IOP_CSINTERNAL struct ShaderBufferInput
{
    // We now disconnect the slot from the user assigned slot
    // The user assigned slot is just for the engine to know what binding the C# side is talking about
    // The RealSlot is what the engine uses for actual shader bindings
    // Need both so we have a mapping between the 2
    // We detacted them because it was causing headaches when building a shader emulation layer for unsupported shader types for older GPUs
    // Vulkan was having a hissy fit about buffer layouts and triggering a driver segfault otherwise
    IOP_CSPUBLIC IOP_UINT16 UserSlot;
    IOP_CSPUBLIC IOP_UINT16 RealSlot;
    IOP_CSPUBLIC IOP_ENUM_NAME(ShaderBufferType) BufferType;
    IOP_CSPUBLIC IOP_UINT16 Count;
};

IOP_PACKED IOP_CSINTERNAL struct RenderProgram
{
    // May have to start storing stuff out of band if I need to make this any larger
    IOP_POINTER(VertexInputAttribute*) VertexAttributes;
    IOP_POINTER(void*) UBOData;
    IOP_POINTER(void*) UserArrayData;
    IOP_POINTER(void*) Data;
    IOP_CSPUBLIC IOP_UINT32 VertexShader;
    IOP_CSPUBLIC IOP_UINT32 PixelShader;
    IOP_CSPUBLIC IOP_UINT32 ExtraShader;
    IOP_CSPUBLIC IOP_UINT32 ShadowVertexShader;
    IOP_CSPUBLIC IOP_UINT32 RenderLayer;
    IOP_UINT32 UBODataSize;
    IOP_UINT32 UserArrayStride;
    IOP_UINT32 UserArrayCount;
    IOP_UINT16 VertexInputCount;
    IOP_CSPUBLIC IOP_UINT16 VertexStride;
    IOP_CSPUBLIC IOP_ENUM_NAME(MaterialBlendMode) ColorBlendMode;
    IOP_CSPUBLIC IOP_ENUM_NAME(CullMode) CullingMode;
    IOP_CSPUBLIC IOP_ENUM_NAME(PrimitiveMode) PrimitiveMode;
    IOP_CSPUBLIC IOP_ENUM_NAME(MaterialMode) MaterialMode;
    IOP_UINT8 Flags;

#ifdef CUBE_LANGUAGE_CPP
    static constexpr uint32_t DestroyFlag = 0;
    static constexpr uint32_t FreeFlag = 7;
#endif
};
/// @endcond

#ifdef CUBE_LANGUAGE_CSHARP
}
#endif

// MIT License
// 
// Copyright (c) 2026 River Govers
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
