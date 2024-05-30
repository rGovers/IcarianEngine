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
    switch (StringHash(a_str.data()) )
    {
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