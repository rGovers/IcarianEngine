#pragma once

#include <cstdint>

#include "ShaderBufferInput.h"
#include "Vertices.h"

namespace FlareBase
{
    enum e_InternalRenderProgram : uint16_t
    {
        InternalRenderProgram_DirectionalLight = 0,
        InternalRenderProgram_PointLight = 1,
        InternalRenderProgram_SpotLight = 2,
        InternalRenderProgram_Post = 3
    };

    enum e_CullMode : uint16_t
    {
        CullMode_None = 0,
        CullMode_Front = 1,
        CullMode_Back = 2,
        CullMode_Both = 3
    };

    enum e_PrimitiveMode : uint16_t
    {
        PrimitiveMode_Triangles = 0,
        PrimitiveMode_TriangleStrip = 1
    };

    struct RenderProgram
    {
        static constexpr unsigned int DestroyFlag = 0;
        static constexpr unsigned int FreeFlag = 7;

        uint32_t VertexShader = -1;
        uint32_t PixelShader = -1;
        uint32_t RenderLayer;
        uint16_t VertexStride;
        uint16_t VertexInputCount;
        VertexInputAttrib* VertexAttribs;
        uint16_t ShaderBufferInputCount;
        ShaderBufferInput* ShaderBufferInputs;
        e_CullMode CullingMode;
        e_PrimitiveMode PrimitiveMode;
        uint8_t EnableColorBlending;
        void* Data;
        uint8_t Flags = 0;

        bool operator ==(const RenderProgram& a_other) const
        {
            if (VertexShader != a_other.VertexShader || PixelShader != a_other.PixelShader || RenderLayer != a_other.RenderLayer)
            {
                return false;
            }

            if (VertexInputCount != a_other.VertexInputCount || ShaderBufferInputCount != a_other.ShaderBufferInputCount)
            {
                return false;
            }

            for (uint32_t i = 0; i < VertexInputCount; ++i)
            {
                if (VertexAttribs[i] != a_other.VertexAttribs[i])
                {
                    return false;
                }
            }

            for (uint32_t i = 0; i < ShaderBufferInputCount; ++i)
            {
                if (ShaderBufferInputs[i] != a_other.ShaderBufferInputs[i])
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
    };
}
