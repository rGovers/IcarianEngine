#pragma once

#include <cstdint>
#include <string>
#include <SPIRV/GlslangToSpv.h>
#include <glslang/Include/ResourceLimits.h>

std::string GLSL_fromFShader(const std::string_view& a_str);

void spirv_init();
void spirv_destroy();

TBuiltInResource spirv_create_resources();

std::vector<uint32_t> spirv_fromGLSL(EShLanguage a_lang, const std::string_view& a_str, bool a_optimize = false);