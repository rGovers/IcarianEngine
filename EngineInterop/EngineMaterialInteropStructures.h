#pragma once

#include "InteropTypes.h"

#ifdef CUBE_LANGUAGE_CPP 
namespace FlareBase
{
#ifndef VertexInputAttrib
    struct VertexInputAttrib;
#endif
}
#endif

#ifdef CUBE_LANGUAGE_CSHARP
namespace IcarianEngine.Rendering {
#endif

IOP_CSPUBLIC enum IOP_ENUM_NAME(ShaderBufferType) : IOP_UINT16
{
    IOP_ENUM_VALUE(ShaderBufferType, Null) = IOP_UINT16_MAX,
    IOP_ENUM_VALUE(ShaderBufferType, CameraBuffer) = 0,
    IOP_ENUM_VALUE(ShaderBufferType, ModelBuffer) = 1,
    IOP_ENUM_VALUE(ShaderBufferType, UIBuffer) = 2,
    IOP_ENUM_VALUE(ShaderBufferType, DirectionalLightBuffer) = 3,
    IOP_ENUM_VALUE(ShaderBufferType, PointLightBuffer) = 4,
    IOP_ENUM_VALUE(ShaderBufferType, SpotLightBuffer) = 5,
    IOP_ENUM_VALUE(ShaderBufferType, Texture) = 6,
    IOP_ENUM_VALUE(ShaderBufferType, PushTexture) = 7,
    IOP_ENUM_VALUE(ShaderBufferType, SSModelBuffer) = 8,
    IOP_ENUM_VALUE(ShaderBufferType, SSBoneBuffer) = 9,
    IOP_ENUM_VALUE(ShaderBufferType, UserUBO) = 10
};

IOP_CSPUBLIC enum IOP_ENUM_NAME(ShaderSlot) : IOP_UINT16
{
    IOP_ENUM_VALUE(ShaderSlot, Null) = IOP_UINT16_MAX,
    IOP_ENUM_VALUE(ShaderSlot, Vertex) = 0,
    IOP_ENUM_VALUE(ShaderSlot, Pixel) = 1,
    IOP_ENUM_VALUE(ShaderSlot, All) = 2
};

IOP_CSPUBLIC enum IOP_ENUM_NAME(CullMode) : IOP_UINT16
{
    IOP_ENUM_VALUE(CullMode, None) = 0,
    IOP_ENUM_VALUE(CullMode, Front) = 1,
    IOP_ENUM_VALUE(CullMode, Back) = 2,
    IOP_ENUM_VALUE(CullMode, Both) = 3
};

IOP_CSPUBLIC enum IOP_ENUM_NAME(PrimitiveMode) : IOP_UINT16
{
    IOP_ENUM_VALUE(PrimitiveMode, Triangles) = 0,
    IOP_ENUM_VALUE(PrimitiveMode, TriangleStrip) = 1
};

/// @cond INTERNAL
IOP_CSINTERNAL enum IOP_ENUM_NAME(InternalRenderProgram) : IOP_UINT16
{
    IOP_ENUM_VALUE(InternalRenderProgram, DirectionalLight) = 0,
    IOP_ENUM_VALUE(InternalRenderProgram, PointLight) = 1,
    IOP_ENUM_VALUE(InternalRenderProgram, SpotLight) = 2,
    IOP_ENUM_VALUE(InternalRenderProgram, Post) = 3
};
/// @endcond

IOP_PACKED IOP_CSPUBLIC struct ShaderBufferInput
{
    IOP_CSPUBLIC IOP_UINT16 Slot;
    IOP_CSPUBLIC IOP_ENUM_NAME(ShaderBufferType) BufferType;
    IOP_CSPUBLIC IOP_ENUM_NAME(ShaderSlot) ShaderSlot;
    IOP_CSPUBLIC IOP_UINT16 Set;

#ifdef CUBE_LANGUAGE_CPP
    constexpr bool operator ==(const ShaderBufferInput& a_other) const
    {
        return Slot == a_other.Slot && BufferType == a_other.BufferType && ShaderSlot == a_other.ShaderSlot;
    }
    constexpr bool operator !=(const ShaderBufferInput& a_other) const
    {
        return !(*this == a_other);
    }
#endif
};

/// @cond INTERNAL
IOP_PACKED IOP_CSINTERNAL struct RenderProgram
{
    IOP_CSPUBLIC IOP_UINT32 VertexShader;
    IOP_CSPUBLIC IOP_UINT32 PixelShader;
    IOP_CSPUBLIC IOP_UINT32 ShadowVertexShader;
    IOP_CSPUBLIC IOP_UINT32 RenderLayer;
    IOP_POINTER(FlareBase::VertexInputAttrib*) VertexAttribs;
    IOP_POINTER(ShaderBufferInput*) ShaderBufferInputs;
    IOP_POINTER(ShaderBufferInput*) ShadowShaderBufferInputs;
    IOP_UINT16 VertexInputCount;
    IOP_UINT16 ShaderBufferInputCount;
    IOP_UINT16 ShadowShaderBufferInputCount;
    IOP_CSPUBLIC IOP_ENUM_NAME(CullMode) CullingMode;
    IOP_CSPUBLIC IOP_ENUM_NAME(PrimitiveMode) PrimitiveMode;
    IOP_CSPUBLIC IOP_UINT16 VertexStride;
    IOP_CSPUBLIC IOP_UINT8 EnableColorBlending;
    IOP_UINT32 UBODataSize;
    IOP_POINTER(void*) UBOData;
    IOP_POINTER(void*) Data;
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

        if (EnableColorBlending != a_other.EnableColorBlending)
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

        // for (uint32_t i = 0; i < VertexInputCount; ++i)
        // {
        //     if (VertexAttribs[i] != a_other.VertexAttribs[i])
        //     {
        //         return false;
        //     }
        // }

        for (uint32_t i = 0; i < ShaderBufferInputCount; ++i)
        {
            if (ShaderBufferInputs[i] != a_other.ShaderBufferInputs[i])
            {
                return false;
            }
        }

        for (uint32_t i = 0; i < ShadowShaderBufferInputCount; ++i)
        {
            if (ShadowShaderBufferInputs[i] != a_other.ShadowShaderBufferInputs[i])
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
