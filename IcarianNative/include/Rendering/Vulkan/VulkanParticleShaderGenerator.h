// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "DataTypes/Array.h"
#include "DataTypes/COWString.h"

class Allocator;

#include "EngineMaterialInteropStructures.h"
#include "EngineParticleSystemInteropStructures.h"

class VulkanParticleShaderGenerator
{
private:

protected:

public:
    static COWU8String GenerateComputeShader(const ComputeParticleBuffer& a_parameters, Array<ShaderBufferInput>* a_inputs, Allocator* a_allocator);

    static COWU8String GenerateMeshShader(const ComputeParticleBuffer& a_parameters, uint16_t* a_slot, Array<ShaderBufferInput>* a_inputs, Allocator* a_allocator);
    static COWU8String GeneratePixelShader(const ComputeParticleBuffer& a_parameters, uint16_t* a_slot, Array<ShaderBufferInput>* a_inputs, Allocator* a_allocator);
};

#endif

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