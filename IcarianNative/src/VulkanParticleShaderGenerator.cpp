#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/VulkanParticleShaderGenerator.h"

#include <cstdint>

// Shamelessly used Godot for reference
// Credit:
// https://github.com/godotengine/godot/blob/master/scene/resources/particle_process_material.cpp

static std::string GenerateVariables(const ComputeParticleBuffer& a_parameters, std::vector<ShaderBufferInput>* a_inputs)
{
    a_inputs->clear();

    ShaderBufferInput input;
    input.ShaderSlot = ShaderSlot_Compute;
    input.Count = 1;

    std::string code;

    uint16_t set = 0;
    uint16_t slot = 0;

    // TODO: Dynamic values probably using the UserUBO or a ParticleComputeBuffer TBD
    if (a_parameters.Flags & 0b1 << ComputeParticleBuffer::DynamicBit)
    {
        
    }
    else
    {
        code += "const float emitterRatio = " + std::to_string(a_parameters.EmitterRatio) + "; \n";

        switch (a_parameters.EmitterType)
        {
        case ParticleEmitterType_Point:
        {
            break;
        }
        case ParticleEmitterType_Sphere:
        {
            code += "const float emitterRadius = " + std::to_string(glm::max(0.0f, a_parameters.EmitterRadius)) + "; \n";

            break;
        }
        }
    }

    input.Slot = slot;
    input.Set = set;
    input.BufferType = ShaderBufferType_SSParticleBuffer;
    a_inputs->emplace_back(input);
    code += "#!structure(SSParticleBuffer," + std::to_string(set++) + "," + std::to_string(slot++) + ",inParticleBuffer) \n";

    input.Slot = slot;
    input.Set = set;
    input.BufferType = ShaderBufferType_SSParticleBuffer;
    a_inputs->emplace_back(input);
    code += "layout(std140,set=" + std::to_string(set++) + ",binding=" + std::to_string(slot++) + ") buffer ParticleShaderBufferOut \n";
    code += "{ \n";
    code += "   int Count; \n";
    code += "   ParticleShaderBufferData objects[]; \n";
    code += "} outParticleBuffer; \n";

    return code;
}

static std::string GenerateBasicFunctions()
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

std::string VulkanParticleShaderGenerator::GenerateComputeShader(const ComputeParticleBuffer& a_parameters, std::vector<ShaderBufferInput>* a_inputs)
{
    std::string code;

    code += "#version 450 \n";

    code += "layout(local_size_x=256, local_size_y=1, local_size_z=1) in; \n";

    code += GenerateVariables(a_parameters, a_inputs);

    code += GenerateBasicFunctions();

    code += "void main() \n";
    code += "{ \n";
    code += "   uint index = gl_GlobalInvocationID.x; \n";
    code += "   uint seed = index; \n";
    code += "   if (index >= inParticleBuffer.Count) \n";
    code += "   { \n";
    code += "       return; \n";
    code += "   } \n";
    
    code += "   ParticleShaderBufferData particle = inParticleBuffer.objects[index]; \n";
    code += "   if (particle.Position.w <= 0.0) \n";
    code += "   { \n";
    code += "       if (rand(seed) < emitterRatio) \n";
    code += "       { \n";
    switch (a_parameters.EmitterType)
    {
    case ParticleEmitterType_Point:
    {
        code += "       outParticleBuffer.objects[index].Position = vec4(0.0, 0.0, 0.0, 5.0); \n";
        code += "       outParticleBuffer.objects[index].Velocity = vec3(rand(seed) * 2 - 1, -rand(seed), rand(seed) * 2 - 1); \n";
        code += "       outParticleBuffer.objects[index].Color = vec4(1.0); \n";

        break;
    }
    case ParticleEmitterType_Sphere:
    {

        break;
    }
    }
    code += "       } \n";
    code += "   } \n";
    code += "   else \n";
    code += "   { \n";
    code += "       outParticleBuffer.objects[index].Position = vec4(particle.Position.xyz + particle.Velocity * 0.01, particle.Position.w - 0.01); \n";
    code += "       outParticleBuffer.objects[index].Velocity = particle.Velocity; \n";
    code += "       outParticleBuffer.objects[index].Color = vec4(1.0); \n";
    code += "   } \n";
    code += "} \n";

    return code;
}

#endif