#pragma once

#include <string_view>

static constexpr const char InternalShaderPathString[] = "[INTERNAL]";

const char* GetVertexShaderString(const std::string_view& a_str);
const char* GetPixelShaderString(const std::string_view& a_str);