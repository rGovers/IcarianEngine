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
};

#endif