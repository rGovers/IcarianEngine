#pragma once

#include <cstdint>
#include <string_view>

#define INTERNALSHADERPATHSTR "[INTERNAL]"
constexpr uint32_t InternalShaderStringSize = sizeof(INTERNALSHADERPATHSTR) - 1;

const char* GetVertexShaderString(const std::string_view& a_str);
const char* GetPixelShaderString(const std::string_view& a_str);