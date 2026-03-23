// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#include <cstdint>
#include <unordered_map>

#include "DataTypes/Array.h"
#include "DataTypes/COWString.h"
#include "DataTypes/Dictionary.h"

#include "EngineMaterialInteropStructures.h"

#define FL_MESHOUT_TABLE(F) \
    F(int, 4) \
    F(uint, 4) \
    F(float, 4) \
    F(vec2, 8) \
    F(vec3, 12) \
    F(vec4, 16) \

#define FL_MESHOUT_ENUMVAL(value) MeshOutType_##value
#define FL_MESHOUT_ENUM(value, size) FL_MESHOUT_ENUMVAL(value),

// TODO: At somepoint I probably need to write a language spec and an actual language rather then a preprocessor
// When this happen should probably review IcarianCore and turn it into a proper library rather then random crap it is
// My list of shit to do is too damm long but
class FlareShader
{
private:

protected:

public:
    enum e_MeshOutType
    {
        FL_MESHOUT_TABLE(FL_MESHOUT_ENUM)

        MeshOutType_Last
    };

    enum e_ShaderPlatform
    {
        ShaderPlatform_Null = -1,
        ShaderPlatform_Vulkan,
        ShaderPlatform_VulkanCompute
    };

    enum e_MeshShaderPrimitive
    {
        MeshShaderPrimitive_Null,
        MeshShaderPrimitive_Point,
        MeshShaderPrimitive_Line,
        MeshShaderPrimitive_Triangle,
    };

    struct ShaderWorkgroups
    {
        uint32_t GroupX;
        uint32_t GroupY;
        uint32_t GroupZ;
    };

    struct MeshShaderOut
    {
        COWU8String Identifier;
        uint32_t Slot;
        e_MeshOutType Type;
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
        // Array<MeshShaderOut> MeshOutputs;
    };

    struct ShaderBuilder
    {
        COWU8String String;
        e_ShaderPlatform Platform;
        Dictionary<COWU8String, COWU8String> Imports;
        Array<ShaderBufferInput>* Inputs;
        Array<ShaderBufferInput> OtherInputs;
        ShaderOutput* Out;
    };

    static COWU8String GLSLFromFlareShader
    (
        COWU8String* Shader,
        const ShaderBuilder& a_builder,
        Allocator* a_allocator,
        Allocator* a_tempAllocator
    );

    static COWU8String GenerateMeshVertexStub(const COWU8String& a_shader, Array<MeshShaderOut>* a_outputs, Allocator* a_allocator, Allocator* a_tempAllocator);
};

// MIT License
// 
// Copyright (c) 2026 River Govers
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