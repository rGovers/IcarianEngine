#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "EngineMaterialInteropStructures.h"

namespace IcarianCore
{
    enum e_ShaderPlatform
    {
        ShaderPlatform_Null = -1,
        ShaderPlatform_Vulkan,
        ShaderPlatform_OpenGL
    };

    std::string GLSLFromFlareShader(const std::string_view& a_str, e_ShaderPlatform a_platform, std::vector<ShaderBufferInput>* a_inputs, std::string* a_error);
}