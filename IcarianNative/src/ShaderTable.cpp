// Icarian Engine - C# Game Engine
// 
// License at end of file.

#include "Rendering/ShaderTable.h"

#include "Core/StringUtils.h"
#include "Shaders.h"

const char* GetVertexShaderString(const std::string_view& a_str)
{
    switch (StringHash(a_str.data())) 
    {
    case StringHash("Quad"):
    {
        return QuadVertexShader;
    }
    case StringHash("UI"):
    {
        return UIVertexShader;
    }
    }

    return nullptr;
}
const char* GetPixelShaderString(const std::string_view& a_str)
{
    // Never optimised string comparisions as building jump tables is a pain in the ass then realised already have one just use switch statements
    // We have constexpr in this day and age
    switch (StringHash(a_str.data()))
    {
    case StringHash("AmbientOcclusion"):
    {
        return AmbientOcclusionPixelShader;
    }
    case StringHash("AmbientOcclusionFilter"):
    {
        return AmbientOcclusionFilterPixelShader;
    }
    case StringHash("AmbientLight"):
    {
        return AmbientLightPixelShader;
    }
    case StringHash("Blend"):
    {
        return BlendPixelShader;
    }
    case StringHash("DirectionalLight"):
    {
        return DirectionalLightPixelShader;
    }
    case StringHash("PointLight"):
    {
        return PointLightPixelShader;
    }
    case StringHash("SpotLight"):
    {
        return SpotLightPixelShader;
    }
    case StringHash("DirectionalLightShadow"):
    {
        return ShadowDirectionalLightPixelShader;
    }
    case StringHash("PointLightShadow"):
    {
        return ShadowPointLightPixelShader;
    }
    case StringHash("SpotLightShadow"):
    {
        return ShadowSpotLightPixelShader;
    }
    case StringHash("PostAtmosphere"):
    {
        return PostAtmospherePixelShader;
    }
    case StringHash("PostEmission"):
    {
        return PostEmissionPixelShader;
    }
    case StringHash("PostEmissionBlur"):
    {
        return PostEmissionBlurPixelShader;
    }
    case StringHash("PostToneMap"):
    {
        return PostToneMapPixelShader;
    }
    case StringHash("UIImage"):
    {
        return UIImagePixelShader;
    }
    case StringHash("UIText"):
    {
        return UITextPixelShader;
    }
    }

    return nullptr;
}

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