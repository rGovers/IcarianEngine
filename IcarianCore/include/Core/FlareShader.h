// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "EngineMaterialInteropStructures.h"

namespace IcarianCore
{
    enum e_ShaderPlatform
    {
        ShaderPlatform_Null = -1,
        ShaderPlatform_Vulkan,
        ShaderPlatform_VulkanCompute,
        ShaderPlatform_OpenGL
    };

#define IC_MESHOUT_TABLE(F) \
    F(int, 4) \
    F(uint, 4) \
    F(float, 4) \
    F(vec2, 8) \
    F(vec3, 12) \
    F(vec4, 16) \

#define IC_MESHOUT_ENUMVAL(value) MeshOutType_##value

#define IC_MESHOUT_ENUM(value, size) IC_MESHOUT_ENUMVAL(value),

    constexpr static uint32_t MeshShaderEmulationVertexBufferOutput = 65;
    constexpr static uint32_t MeshShaderEmulationIndexBufferOutput = 66;

    enum e_MeshOutType
    {
        IC_MESHOUT_TABLE(IC_MESHOUT_ENUM)

        MeshOutType_Last
    };

    struct MeshShaderOut
    {
        std::string Identifier;
        uint32_t Slot;
        e_MeshOutType Type;
    };

    struct ShaderWorkgroups
    {
        uint32_t GroupX;
        uint32_t GroupY;
        uint32_t GroupZ;
    };

    enum e_MeshShaderPrimitive
    {
        MeshShaderPrimitive_Null,
        MeshShaderPrimitive_Point,
        MeshShaderPrimitive_Line,
        MeshShaderPrimitive_Triangle,
    };

    struct MeshShaderData
    {
        e_MeshShaderPrimitive PrimitiveType;
        uint32_t MaxPrimitives;
        uint32_t MaxVertices;
    };

    struct ShaderOutput
    {
        ShaderWorkgroups Workgroups;
        MeshShaderData MeshData;
        std::vector<MeshShaderOut> MeshOutputs;
    };

    std::string GLSLFromFlareShader
    (
        const std::string_view& a_str,
        e_ShaderPlatform a_platform,
        const std::unordered_map<std::string, std::string>& a_imports,
        std::vector<ShaderBufferInput>* a_inputs,
        std::string* a_error,
        ShaderOutput* a_out = nullptr
    );
    std::string GenerateMeshVertexStub(const std::vector<MeshShaderOut>& a_outputs);
}

// MIT License
// 
// Copyright (c) 2025 River Govers
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
