#pragma once

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include <string>
#include <vector>

#include "EngineMaterialInteropStructures.h"
#include "EngineParticleSystemInteropStructures.h"

class VulkanParticleShaderGenerator
{
private:

protected:

public:
    static std::string GenerateComputeShader(const ComputeParticleBuffer& a_parameters, std::vector<ShaderBufferInput>* a_inputs);

    static std::string GenerateVertexShader(const ComputeParticleBuffer& a_parameters, uint16_t* a_slot, std::vector<ShaderBufferInput>* a_inputs, std::vector<VertexInputAttribute>* a_vertexInputs);
    static std::string GeneratePixelShader(const ComputeParticleBuffer& a_parameters, uint16_t* a_slot, std::vector<ShaderBufferInput>* a_inputs);
};

#endif