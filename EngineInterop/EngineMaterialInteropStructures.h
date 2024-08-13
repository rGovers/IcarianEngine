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
    IOP_ENUM_VALUE(ShaderBufferType, Texture) = 9,
    IOP_ENUM_VALUE(ShaderBufferType, PushTexture) = 10,
    IOP_ENUM_VALUE(ShaderBufferType, ShadowLightBuffer) = 11,
    IOP_ENUM_VALUE(ShaderBufferType, ShadowTexture2D) = 12, 
    IOP_ENUM_VALUE(ShaderBufferType, ShadowTextureCube) = 13,
    IOP_ENUM_VALUE(ShaderBufferType, UserUBO) = 14,
    IOP_ENUM_VALUE(ShaderBufferType, SSModelBuffer) = 15,
    IOP_ENUM_VALUE(ShaderBufferType, SSBoneBuffer) = 16,
    IOP_ENUM_VALUE(ShaderBufferType, SSDirectionalLightBuffer) = 17,
    IOP_ENUM_VALUE(ShaderBufferType, SSPointLightBuffer) = 18,
    IOP_ENUM_VALUE(ShaderBufferType, SSSpotLightBuffer) = 19,
    IOP_ENUM_VALUE(ShaderBufferType, SSAmbientLightBuffer) = 20,
    IOP_ENUM_VALUE(ShaderBufferType, SSShadowLightBuffer) = 21,
    IOP_ENUM_VALUE(ShaderBufferType, SSParticleBuffer) = 22,
    IOP_ENUM_VALUE(ShaderBufferType, AShadowTexture2D) = 23
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
    IOP_CSPUBLIC IOP_UINT16 Slot;
    IOP_CSPUBLIC IOP_ENUM_NAME(ShaderBufferType) BufferType;
    IOP_CSPUBLIC IOP_UINT16 Count;

#ifdef CUBE_LANGUAGE_CPP
    constexpr bool operator ==(const ShaderBufferInput& a_other) const
    {
        return Slot == a_other.Slot && BufferType == a_other.BufferType && Count == a_other.Count;
    }
    constexpr bool operator !=(const ShaderBufferInput& a_other) const
    {
        return !(*this == a_other);
    }
#endif
};

IOP_PACKED IOP_CSINTERNAL struct RenderProgram
{
    IOP_CSPUBLIC IOP_UINT32 VertexShader;
    IOP_CSPUBLIC IOP_UINT32 PixelShader;
    IOP_CSPUBLIC IOP_UINT32 ShadowVertexShader;
    IOP_CSPUBLIC IOP_UINT32 RenderLayer;
    IOP_POINTER(VertexInputAttribute*) VertexAttributes;
    IOP_UINT16 VertexInputCount;
    IOP_CSPUBLIC IOP_UINT16 VertexStride;
    IOP_UINT32 UBODataSize;
    IOP_POINTER(void*) UBOData;
    IOP_POINTER(void*) Data;
    IOP_CSPUBLIC IOP_ENUM_NAME(MaterialBlendMode) ColorBlendMode;
    IOP_CSPUBLIC IOP_ENUM_NAME(CullMode) CullingMode;
    IOP_CSPUBLIC IOP_ENUM_NAME(PrimitiveMode) PrimitiveMode;
    IOP_UINT8 Flags;

#ifdef CUBE_LANGUAGE_CPP
    static constexpr unsigned int DestroyFlag = 0;
    static constexpr unsigned int FreeFlag = 7;

    bool operator ==(const RenderProgram& a_other) const
    {
        if (VertexShader != a_other.VertexShader || PixelShader != a_other.PixelShader)
        {
            return false;
        }

        if (CullingMode != a_other.CullingMode || PrimitiveMode != a_other.PrimitiveMode)
        {
            return false;
        }

        if (ColorBlendMode != a_other.ColorBlendMode)
        {
            return false;
        }

        if (ShadowVertexShader != a_other.ShadowVertexShader)
        {
            return false;
        }

        if (RenderLayer != a_other.RenderLayer)
        {
            return false;
        }

        if (VertexInputCount != a_other.VertexInputCount)
        {
            return false;
        }

        for (uint32_t i = 0; i < VertexInputCount; ++i)
        {
            if (VertexAttributes[i] != a_other.VertexAttributes[i])
            {
                return false;
            }
        }

        return true;
    }

    bool operator !=(const RenderProgram& a_other) const
    {
        return !(*this == a_other);
    }
#endif
};
/// @endcond

#ifdef CUBE_LANGUAGE_CSHARP
}
#endif

// MIT License
// 
// Copyright (c) 2024 River Govers
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