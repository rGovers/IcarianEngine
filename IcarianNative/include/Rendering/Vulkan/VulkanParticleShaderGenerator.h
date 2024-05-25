#pragma once

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include <string>

#include "DataTypes/Array.h"

#include "EngineMaterialInteropStructures.h"
#include "EngineParticleSystemInteropStructures.h"

class VulkanParticleShaderGenerator
{
private:

protected:

public:
    static std::string GenerateComputeShader(const ComputeParticleBuffer& a_parameters, Array<ShaderBufferInput>* a_inputs);

    static std::string GenerateVertexShader(const ComputeParticleBuffer& a_parameters, uint16_t* a_slot, Array<ShaderBufferInput>* a_inputs, Array<VertexInputAttribute>* a_vertexInputs);
    static std::string GeneratePixelShader(const ComputeParticleBuffer& a_parameters, uint16_t* a_slot, Array<ShaderBufferInput>* a_inputs);
};

#endif