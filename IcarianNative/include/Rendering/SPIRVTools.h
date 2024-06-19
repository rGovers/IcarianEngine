#pragma once

#include <string_view>
#include <SPIRV/GlslangToSpv.h>
#include <glslang/Include/ResourceLimits.h>
#include <glslang/Public/ShaderLang.h>

void spirv_init();
void spirv_destroy();

std::vector<unsigned int> spirv_fromGLSL(EShLanguage a_lang, const std::string_view& a_str, bool a_optimize = false);