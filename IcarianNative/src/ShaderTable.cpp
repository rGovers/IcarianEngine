#include "Rendering/ShaderTable.h"

#include "Shaders.h"

const char* GetVertexShaderString(const std::string_view& a_str)
{
    const std::string iStr = std::string(InternalShaderPathString);

    if (a_str == iStr + "Quad")
    {
        return QuadVertexShader;
    }
    else if (a_str == iStr + "UI")
    {
        return UIVertexShader;
    }

    return nullptr;
}
const char* GetPixelShaderString(const std::string_view& a_str)
{
    const std::string iStr = std::string(InternalShaderPathString);

    if (a_str == iStr + "DirectionalLight")
    {
        return DirectionalLightPixelShader;
    }
    else if (a_str == iStr + "PointLight")
    {
        return PointLightPixelShader;
    }
    else if (a_str == iStr + "SpotLight")
    {
        return SpotLightPixelShader;
    }
    else if (a_str == iStr + "Post")
    {
        return PostPixelShader;
    }
    else if (a_str == iStr + "DirectionalLightShadow")
    {
        return ShadowDirectionalLightPixelShader;
    }
    else if (a_str == iStr + "UIImage")
    {
        return UIImagePixelShader;
    }
    else if (a_str == iStr + "UIText")
    {
        return UITextPixelShader;
    }

    return nullptr;
}