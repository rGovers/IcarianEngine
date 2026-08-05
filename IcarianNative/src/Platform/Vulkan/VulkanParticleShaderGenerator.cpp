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

static constexpr uint32_t WorkgroupSize = 256;

static IcarianCore::COWU8String GenerateComputeVariables
(
    const ComputeParticleBuffer& a_parameters,
    IcarianCore::Array<ShaderBufferInput>* a_inputs,
    IcarianCore::Allocator* a_allocator
)
{
    // TOOD: Consider implementing a string builder
    // There seems to be a lot of reallocating string when when can just build a list and the build the entire string at once
    a_inputs->Clear();

    IcarianCore::COWU8String code = IcarianCore::COWU8String(a_allocator);

    uint16_t slot = 0;

    code += "const vec3 gravity = vec3(" +
        IcarianCore::COWU8String::FromValue(a_parameters.Gravity.x, a_allocator) + ", " +
        IcarianCore::COWU8String::FromValue(a_parameters.Gravity.y, a_allocator) + ", " +
        IcarianCore::COWU8String::FromValue(a_parameters.Gravity.z, a_allocator) + "); \n";

    const bool isBurst = IISBITSET(a_parameters.Flags, ComputeParticleBuffer::BurstBit);
    if (!isBurst)
    {
        code += "const float emitterRatio = " + IcarianCore::COWU8String::FromValue(a_parameters.EmitterRatio, a_allocator) + "; \n";

        code += "const vec3 initialVelocity = vec3(" +
            IcarianCore::COWU8String::FromValue(a_parameters.InitialVelocity.x, a_allocator) + ", " +
            IcarianCore::COWU8String::FromValue(a_parameters.InitialVelocity.y, a_allocator) + ", " +
            IcarianCore::COWU8String::FromValue(a_parameters.InitialVelocity.z, a_allocator) + "); \n";
        code += "const float lifetime = " + IcarianCore::COWU8String::FromValue(a_parameters.Lifetime, a_allocator) + "; \n";
        code += "const float emitterVelocityScale = " + IcarianCore::COWU8String::FromValue(a_parameters.EmitterVelocityScale, a_allocator) + "; \n";

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

    const ShaderBufferInput timeInput =
    {
        .UserSlot = slot,
        .RealSlot = slot,
        .BufferType = ShaderBufferType_TimeBuffer,
        .Count = 1,
    };
    a_inputs->Push(timeInput);
    code += "#!structure(TimeBuffer," + IcarianCore::COWU8String::FromValue(slot++, 10, a_allocator) + ",timeBuffer) \n";

    const ShaderBufferInput particleAInput =
    {
        .UserSlot = slot,
        .RealSlot = slot,
        .BufferType = ShaderBufferType_SSParticleBuffer,
        .Count = 1,
    };
    a_inputs->Push(particleAInput);
    code += "#!structure(SSParticleBuffer," + IcarianCore::COWU8String::FromValue(slot++, 10, a_allocator) + ",inParticleBuffer) \n";

    const ShaderBufferInput particleBInput =
    {
        .UserSlot = slot,
        .RealSlot = slot,
        .BufferType = ShaderBufferType_SSParticleBuffer,
        .Count = 1,
    };
    const IcarianCore::COWU8String partSlotStr = IcarianCore::COWU8String::FromValue(slot++, 10, a_allocator);
    a_inputs->Push(particleBInput);
    code += "layout(std140,binding=" + partSlotStr + ",set=" + partSlotStr + ") buffer ParticleShaderBufferOut \n"
    "{ \n"
    "   int Count; \n"
    "   ParticleBufferData objects[]; \n"
    "} outParticleBuffer; \n";

    return code;
}

static IcarianCore::COWU8String GenerateComputeBasicFunctions(IcarianCore::Allocator* a_allocator)
{
    return IcarianCore::COWU8String("float rand(inout uint a_seed) \n"
    "{ \n"
    "   int s = int(a_seed); \n"
    "   if (s == 0) \n"
    "   { \n"
    "       s = 305420679; \n"
    "   } \n"
    "   int k = s / 127773; \n"
    "   s = 16807 * (s - k * 127773) - 2836 * k; \n"
    "   if (s < 0) \n"
    "   { \n"
    "       s += 2147483647; \n"
    "   } \n"
    "   a_seed = uint(s); \n"
    "   return float(a_seed % 65536) / 65535.0; \n"
    "} \n", a_allocator);
}

IcarianCore::COWU8String VulkanParticleShaderGenerator::GenerateComputeShader
(
    const ComputeParticleBuffer& a_parameters,
    IcarianCore::Array<ShaderBufferInput>* a_inputs,
    IcarianCore::Allocator* a_allocator
)
{
    IcarianCore::COWU8String code = IcarianCore::COWU8String(a_allocator);

    code += "#version 450 \n";

    code += "layout(local_size_x=" + IcarianCore::COWU8String::FromValue(WorkgroupSize, 10, a_allocator) + ", local_size_y=1, local_size_z=1) in; \n";

    code += GenerateComputeVariables(a_parameters, a_inputs, a_allocator);

    code += GenerateComputeBasicFunctions(a_allocator);

    code += "void main() \n";
    code += "{ \n";
    code += "   uint index = gl_GlobalInvocationID.x; \n";
    if (a_parameters.MaxParticles % WorkgroupSize != 0)
    {
        code += "   if (index >= inParticleBuffer.Count) \n"
        "   { \n"
        "       return; \n"
        "   } \n";
    }

    code += "   float delta = timeBuffer.Time.x; \n"
    "   float timePassed = timeBuffer.Time.y; \n"
    // Using a prime for a better seed
    "   uint seedDelta = uint(timePassed * 5003); \n"
    "   uint seed = index * seedDelta + seedDelta; \n"

    "   ParticleBufferData particle = inParticleBuffer.objects[index]; \n"
    "   if (particle.Position.w <= 0.0) \n"
    "   { \n"
    "       outParticleBuffer.objects[index].Position = vec4(0.0); \n";

    const bool isBurst = IISBITSET(a_parameters.Flags, ComputeParticleBuffer::BurstBit);
    if (!isBurst)
    {
        code += "       if (rand(seed) < emitterRatio) \n"
        "       { \n";

        switch (a_parameters.EmitterType)
        {
        case ParticleEmitterType_Point:
        {
            code += "       outParticleBuffer.objects[index].Position = vec4(0.0, 0.0, 0.0, lifetime); \n"
            "       outParticleBuffer.objects[index].Velocity = ";

            if (a_parameters.EmitterVelocityScale > 0.001f)
            {
                code += "vec3(rand(seed) * 2 - 1, rand(seed) * 2 - 1, rand(seed) * 2 - 1)";

                if (glm::epsilonEqual(a_parameters.EmitterVelocityScale, 1.0f, 0.001f))
                {
                    code += " * emitterVelocityScale";
                }
            }
            else
            {
                code += "vec3(0.0)";
            }

            if (a_parameters.InitialVelocity != glm::vec3(0.0))
            {
                code += " + initialVelocity";
            }

            code += "; \n";

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

    code += "   } \n"
    "   else \n"
    "   { \n"
    "       outParticleBuffer.objects[index].Position = vec4(particle.Position.xyz + particle.Velocity * delta, particle.Position.w - delta); \n"
    "       outParticleBuffer.objects[index].Velocity = particle.Velocity";

    if (a_parameters.Gravity != glm::vec3(0.0f))
    {
        code += " + (gravity * delta)";
    }

    code += "; \n"

    "   } \n"
    "} \n";

    return code;
}

static IcarianCore::COWU8String GenerateMeshVariables
(
    const ComputeParticleBuffer& a_parameters,
    uint16_t* a_slot,
    IcarianCore::Array<ShaderBufferInput>* a_inputs,
    IcarianCore::Allocator* a_allocator
)
{
    a_inputs->Clear();

    IcarianCore::COWU8String code = IcarianCore::COWU8String(a_allocator);

    code += "const float startSize = " + IcarianCore::COWU8String::FromValue(a_parameters.StartSize, a_allocator) + "; \n";
    code += "const float endSize = " + IcarianCore::COWU8String::FromValue(a_parameters.EndSize, a_allocator) + "; \n";

    code += "const vec4 startColour = vec4(" +
        IcarianCore::COWU8String::FromValue(a_parameters.StartColour.x, a_allocator) + ", " +
        IcarianCore::COWU8String::FromValue(a_parameters.StartColour.y, a_allocator) + ", " +
        IcarianCore::COWU8String::FromValue(a_parameters.StartColour.z, a_allocator) + ", " +
        IcarianCore::COWU8String::FromValue(a_parameters.StartColour.w, a_allocator) + "); \n";
    code += "const vec4 endColour = vec4(" +
        IcarianCore::COWU8String::FromValue(a_parameters.EndColour.x, a_allocator) + ", " +
        IcarianCore::COWU8String::FromValue(a_parameters.EndColour.y, a_allocator) + ", " +
        IcarianCore::COWU8String::FromValue(a_parameters.EndColour.z, a_allocator) + ", " +
        IcarianCore::COWU8String::FromValue(a_parameters.EndColour.w, a_allocator) + "); \n";

    code += "const float lifetime = " + IcarianCore::COWU8String::FromValue(a_parameters.Lifetime, a_allocator) + "; \n";
    code += "const float invLifetime = " + IcarianCore::COWU8String::FromValue(1.0f / a_parameters.Lifetime, a_allocator) + "; \n";

    const ShaderBufferInput cameraInput =
    {
        .UserSlot = *a_slot,
        .RealSlot = *a_slot,
        .BufferType = ShaderBufferType_CameraBuffer,
        .Count = 1,
    };
    a_inputs->Push(cameraInput);
    code += "#!structure(CameraBuffer, " + IcarianCore::COWU8String::FromValue((*a_slot)++, 10, a_allocator) + ", camBuffer) \n";

    const ShaderBufferInput modelInput =
    {
        .BufferType = ShaderBufferType_PModelBuffer,
        .Count = 1,
    };
    a_inputs->Push(modelInput);
    code += "#!pushbuffer(PModelBuffer, modelBuffer)";

    const ShaderBufferInput particleBuffer =
    {
        .UserSlot = *a_slot,
        .RealSlot = *a_slot,
        .BufferType = ShaderBufferType_SSParticleBuffer,
        .Count = 1,
    };
    a_inputs->Push(particleBuffer);
    code += "#!structure(SSParticleBuffer, " + IcarianCore::COWU8String::FromValue((*a_slot)++, 10, a_allocator) + ", particleBuffer) \n";

    switch (a_parameters.DisplayMode)
    {
    case ParticleDisplayMode_Quad:
    {
        break;
    }
    default:
    {
        IERROR("Invalid particle display mode");

        break;
    }
    }

    return code;
}

IcarianCore::COWU8String VulkanParticleShaderGenerator::GenerateMeshShader
(
    const ComputeParticleBuffer& a_parameters,
    uint16_t* a_slot,
    IcarianCore::Array<ShaderBufferInput>* a_inputs,
    IcarianCore::Allocator* a_allocator
)
{
    IcarianCore::COWU8String code = IcarianCore::COWU8String(a_allocator);

    code += "#version 450 \n"

    "#extension GL_EXT_mesh_shader : require\n"

    "layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in; \n"
    "layout(triangles, max_vertices = 4, max_primitives = 2) out; \n";

    code += GenerateMeshVariables(a_parameters, a_slot, a_inputs, a_allocator);

    code += "layout(location=0) out PerVertexData \n"
    "{ \n"
    "   vec2 UV; \n"
    "   vec4 Color; \n"
    "   uint Index; \n"
    "} vertOut[];\n"

    "struct TaskPayload \n"
    "{ \n"
    "   uint TaskID; \n"
    "}; \n"

    "taskPayloadSharedEXT TaskPayload taskIn; \n"

    "void main() \n"
    "{ \n"
    "   uint index = taskIn.TaskID * " + IcarianCore::COWU8String::FromValue(WorkgroupSize, 10, a_allocator) + " + gl_GlobalInvocationID.x; \n";

    if (a_parameters.MaxParticles % WorkgroupSize != 0)
    {
        code += "   if (index >= " + IcarianCore::COWU8String::FromValue(a_parameters.MaxParticles, 10, a_allocator) + ") \n"
        "   { \n"
        "       SetMeshOutputsEXT(0, 0); \n"
        "       return; \n"
        "   } \n";
    }

    switch (a_parameters.DisplayMode)
    {
    case ParticleDisplayMode_Quad:
    {
        code += "   ParticleBufferData particle = particleBuffer.objects[index]; \n"
        "   float particleLife = particle.Position.w; \n"
        // Apparently need to use an epsilon otherwise things get fucky
        "   if (particleLife > 0.001) \n"
        "   { \n"
        "       SetMeshOutputsEXT(4, 2); \n"
        "       float lifeScale = particleLife * invLifetime; \n";

        if (a_parameters.StartSize != a_parameters.EndSize)
        {
            code += "       float size = mix(endSize, startSize, lifeScale); \n";
        }
        else
        {
            code += "       float size = startSize; \n";
        }

        if (a_parameters.StartColour != a_parameters.EndColour)
        {
            code += "       vec4 colour = mix(endColour, startColour, lifeScale); \n";
        }
        else
        {
            code += "       vec4 colour = startColour; \n";
        }

        code += "       mat3 camBill = mat3(camBuffer.InvView); \n";

        for (uint32_t i = 0; i < 4; ++i)
        {
            // Sigh... If the GLSL compiler is shit just use C++ and "handwrite" the unrolled loop
            // This is a dumb performance gain but GLSL compiler apparently still needs work with const folding
            // Yes I should just manually hand unroll the loop but I am lazy so I eat the performance hit of string operations on the C++ side
            // Yes it would be a big optimization to manually unroll it however doing a lot of string ops so a couple extra from value is the least of our concern
            const IcarianCore::COWU8String indexStr = IcarianCore::COWU8String::FromValue(i, a_allocator);

            const glm::vec2 uv = glm::vec2(i & 1, i >> 1);
            const glm::vec2 pos = uv * 2.0f + glm::vec2(-1.0f);

            code += "       vec3 billPos = camBill * vec3(" +
                IcarianCore::COWU8String::FromValue(pos.x, a_allocator) + ", " +
                IcarianCore::COWU8String::FromValue(pos.y, a_allocator) + ", "
                "0.0); \n"
                "       gl_MeshVerticesEXT[" + indexStr + "].gl_Position = camBuffer.ViewProj * modelBuffer.Model * vec4(particle.Position.xyz + billPos * size, 1.0); \n"
                "       vertOut[" + indexStr + "].UV = uv; \n"
                "       vertOut[" + indexStr + "].Color = colour; \n"
                "       vertOut[" + indexStr + "].Index = index; \n";
        }

        code += "       gl_PrimitiveTriangleIndicesEXT[gl_LocalInvocationIndex + 0] =  uvec3(0, 1, 2); \n"
        "       gl_PrimitiveTriangleIndicesEXT[gl_LocalInvocationIndex + 1] =  uvec3(1, 3, 2); \n"
        "   } \n"
        "   else \n"
        "   { \n"
        "      SetMeshOutputsEXT(0, 0); \n"
        "   }\n";

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
IcarianCore::COWU8String VulkanParticleShaderGenerator::GeneratePixelShader
(
    const ComputeParticleBuffer& a_parameters,
    uint16_t* a_slot,
    IcarianCore::Array<ShaderBufferInput>* a_inputs,
    IcarianCore::Allocator* a_allocator
)
{
    return IcarianCore::COWU8String("#version 450 \n"

    "layout(location = 0) in PerVertexData \n"
    "{ \n"
    "   vec2 UV; \n"
    "   vec4 Color; \n"
    "   uint Index; \n"
    "} fragIn; \n"

    "layout(location = 0) out vec4 outColor; \n"

    "void main() \n"
    "{ \n"
    "   outColor = fragIn.Color; \n"
    "} \n", a_allocator);
}

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
