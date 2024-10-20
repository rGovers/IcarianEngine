// Icarian Engine - C# Game Engine
// 
// License at end of file.

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/VulkanParticleShaderGenerator.h"

#include <cstdint>

#include "Core/Bitfield.h"
#include "IcarianError.h"

// Shamelessly used Godot for reference
// Credit:
// https://github.com/godotengine/godot/blob/master/scene/resources/particle_process_material.cpp

static std::string GenerateComputeVariables(const ComputeParticleBuffer& a_parameters, Array<ShaderBufferInput>* a_inputs)
{
    a_inputs->Clear();

    ShaderBufferInput input;
    input.Count = 1;

    std::string code;

    uint16_t slot = 0;

    // TODO: Dynamic values probably using the UserUBO or a ParticleComputeBuffer TBD
    if (IISBITSET(a_parameters.Flags, ComputeParticleBuffer::DynamicBit))
    {
        
    }
    else
    {
        code += "const vec3 gravity = vec3(" + std::to_string(a_parameters.Gravity.x) + ", " + std::to_string(a_parameters.Gravity.y) + ", " + std::to_string(a_parameters.Gravity.z) + "); \n";
        code += "const vec4 colour = vec4(" + std::to_string(a_parameters.Colour.x) + ", " + std::to_string(a_parameters.Colour.y) + ", " + std::to_string(a_parameters.Colour.z) + ", " + std::to_string(a_parameters.Colour.w) + "); \n";

        const bool isBurst = IISBITSET(a_parameters.Flags, ComputeParticleBuffer::BurstBit);
        if (!isBurst)
        {
            code += "const float emitterRatio = " + std::to_string(a_parameters.EmitterRatio) + "; \n";

            switch (a_parameters.EmitterType)
            {
            case ParticleEmitterType_Point:
            {
                break;
            }
            default:
            {
                IERROR("Invalid particle emitter type");

                break;
            }
            }
        }
    }

    input.Slot = slot;
    input.BufferType = ShaderBufferType_TimeBuffer;
    a_inputs->Push(input);
    code += "#!structure(TimeBuffer," + std::to_string(slot++) + ",timeBuffer) \n";

    input.Slot = slot;
    input.BufferType = ShaderBufferType_SSParticleBuffer;
    a_inputs->Push(input);
    code += "#!structure(SSParticleBuffer," + std::to_string(slot++) + ",inParticleBuffer) \n";

    input.Slot = slot;
    input.BufferType = ShaderBufferType_SSParticleBuffer;
    a_inputs->Push(input);
    code += "layout(std140,binding=" + std::to_string(slot) + ",set=" + std::to_string(slot) + ") buffer ParticleShaderBufferOut \n";
    code += "{ \n";
    code += "   int Count; \n";
    code += "   ParticleBufferData objects[]; \n";
    code += "} outParticleBuffer; \n";
    ++slot;

    return code;
}

static std::string GenerateComputeBasicFunctions()
{
    std::string code;

    // Knicked but cannot remember original source something C related is all I can say.
    // Godot also uses it but cannot remember where I originally saw it
    code += "float rand(inout uint a_seed) \n";
    code += "{ \n";
    code += "   int s = int(a_seed); \n";
    code += "   if (s == 0) \n";
    code += "   { \n";
    code += "       s = 305420679; \n";
    code += "   } \n";
    code += "   int k = s / 127773; \n";
    code += "   s = 16807 * (s - k * 127773) - 2836 * k; \n";
    code += "   if (s < 0) \n";
    code += "   { \n";
    code += "       s += 2147483647; \n";
    code += "   } \n";
    code += "   a_seed = uint(s); \n";
    code += "   return float(a_seed % 65536) / 65535.0; \n";
    code += "} \n";

    return code;
}

std::string VulkanParticleShaderGenerator::GenerateComputeShader(const ComputeParticleBuffer& a_parameters, Array<ShaderBufferInput>* a_inputs)
{
    std::string code;

    constexpr uint32_t WorkgroupSize = 256;

    code += "#version 450 \n";

    code += "layout(local_size_x=" + std::to_string(WorkgroupSize) +", local_size_y=1, local_size_z=1) in; \n";

    code += GenerateComputeVariables(a_parameters, a_inputs);

    code += GenerateComputeBasicFunctions();

    code += "void main() \n";
    code += "{ \n";
    code += "   uint index = gl_GlobalInvocationID.x; \n";
    if (a_parameters.MaxParticles % WorkgroupSize != 0)
    {
        code += "   if (index >= inParticleBuffer.Count) \n";
        code += "   { \n";
        code += "       return; \n";
        code += "   } \n";
    }
    
    code += "   float delta = timeBuffer.Time.x; \n";
    code += "   float timePassed = timeBuffer.Time.y; \n";
    code += "   uint seedDelta = uint(timePassed * 1000); \n";
    code += "   uint seed = index * seedDelta + seedDelta; \n";
    
    code += "   ParticleBufferData particle = inParticleBuffer.objects[index]; \n";
    code += "   if (particle.Position.w <= 0.0) \n";
    code += "   { \n";
    code += "       outParticleBuffer.objects[index].Position = vec4(0.0); \n";

    const bool isBurst = IISBITSET(a_parameters.Flags, ComputeParticleBuffer::BurstBit);
    if (!isBurst)
    {
        code += "       if (rand(seed) < emitterRatio) \n";
        code += "       { \n";

        switch (a_parameters.EmitterType)
        {
        case ParticleEmitterType_Point:
        {
            code += "       outParticleBuffer.objects[index].Position = vec4(0.0, 0.0, 0.0, 5.0); \n";
            code += "       outParticleBuffer.objects[index].Velocity = vec3(rand(seed) * 2 - 1, rand(seed) * 2 - 1, rand(seed) * 2 - 1); \n";
            code += "       outParticleBuffer.objects[index].Color = colour; \n";

            break;
        }
        default:
        {
            IERROR("Invalid particle emitter type");

            break;
        }
        }

        code += "       } \n";
    }

    code += "   } \n";
    code += "   else \n";
    code += "   { \n";
    code += "       outParticleBuffer.objects[index].Position = vec4(particle.Position.xyz + particle.Velocity * delta, particle.Position.w - delta); \n";
    code += "       outParticleBuffer.objects[index].Velocity = particle.Velocity + gravity * delta; \n";
    code += "       outParticleBuffer.objects[index].Color = particle.Color; \n";
    code += "   } \n";
    code += "} \n";

    return code;
}

std::string VulkanParticleShaderGenerator::GenerateVertexShader(const ComputeParticleBuffer& a_parameters, uint16_t* a_slot, Array<ShaderBufferInput>* a_inputs, Array<VertexInputAttribute>* a_vertexInputs)
{
    std::string code;

    code += "#version 450 \n";

    ShaderBufferInput input;
    input.Count = 1;

    switch (a_parameters.DisplayMode)
    {
    case ParticleDisplayMode_Quad:
    {
        input.Slot = *a_slot;
        input.BufferType = ShaderBufferType_SSParticleBuffer;
        a_inputs->Push(input);
        code += "#!structure(SSParticleBuffer," + std::to_string((*a_slot)++) + ",particleBuffer) \n";

        code += "vec2 positions[6] = vec2[] \n";
        code += "( \n";
        code += "   vec2(-1.0, 1.0), \n";
        code += "   vec2(1.0, 1.0), \n";
        code += "   vec2(-1.0, -1.0), \n";
        code += "   vec2(1.0, 1.0), \n";
        code += "   vec2(1.0, -1.0), \n";
        code += "   vec2(-1.0, -1.0) \n";
        code += "); \n";

        code += "vec2 uvs[6] = vec2[] \n";
        code += "( \n";
        code += "   vec2(0.0, 1.0), \n";
        code += "   vec2(1.0, 1.0), \n";
        code += "   vec2(0.0, 0.0), \n";
        code += "   vec2(1.0, 1.0), \n";
        code += "   vec2(1.0, 0.0), \n";
        code += "   vec2(0.0, 0.0) \n";
        code += "); \n";

        code += "layout(location=1) out vec2 fragUV; \n";

        break;
    }
    default:
    {
        IERROR("Invalid particle display mode");

        break;
    }
    }

    input.Slot = *a_slot;
    input.BufferType = ShaderBufferType_CameraBuffer;
    a_inputs->Push(input);
    code += "#!structure(CameraBuffer," + std::to_string((*a_slot)++) + ",camBuffer) \n";

    input.BufferType = ShaderBufferType_PModelBuffer;
    a_inputs->Push(input);
    code += "#!pushbuffer(PModelBuffer, modelBuffer)";

    code += "layout(location=0) out vec4 fragColor; \n";

    code += "void main() \n";
    code += "{ \n";
    switch (a_parameters.DisplayMode)
    {
    case ParticleDisplayMode_Quad:
    {
        code += "   int particleIndex = gl_VertexIndex / 6; \n";
        code += "   int vertexIndex = gl_VertexIndex % 6; \n";
        
        code += "   ParticleBufferData particle = particleBuffer.objects[particleIndex]; \n";

        code += "   float alive = max(sign(particle.Position.w), 0.0); \n";
        code += "   fragColor = particle.Color; \n";
        code += "   mat3 camBill = mat3(camBuffer.InvView); \n";
        code += "   vec3 billPos = camBill * vec3(positions[vertexIndex], 0.0); \n";
        code += "   vec4 pos = camBuffer.ViewProj * modelBuffer.Model * vec4(particle.Position.xyz + billPos * 0.01, 1.0); \n";
        code += "   gl_Position = pos * alive; \n";

        break;
    }
    default:
    {
        IERROR("Invalid particle display mode");

        break;
    }
    }

    code += "} \n";
 
    return code;
}
std::string VulkanParticleShaderGenerator::GeneratePixelShader(const ComputeParticleBuffer& a_parameters, uint16_t* a_slot, Array<ShaderBufferInput>* a_inputs)
{
    std::string code;

    code += "#version 450 \n";

    code += "layout(location=0) in vec4 fragColor; \n";

    code += "layout(location=0) out vec4 outColor; \n";

    code += "void main() \n";
    code += "{ \n";
    code += "   outColor = fragColor; \n";
    code += "} \n";

    return code;
}

#endif

// MIT License
// 
// Copyright (c) 2024 River Govers
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