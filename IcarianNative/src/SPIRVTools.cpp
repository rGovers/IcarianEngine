// Icarian Engine - C# Game Engine
// 
// License at end of file.

#include "Rendering/SPIRVTools.h"

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN
#include "Rendering/Vulkan/IcarianVulkanHeader.h"
#endif

#include <SPIRV/SpvTools.h>

#include "IcarianError.h"
#include "Trace.h"

void spirv_init()
{
    glslang::InitializeProcess();
}
void spirv_destroy()
{
    glslang::FinalizeProcess();   
}
constexpr TBuiltInResource spirv_create_resources()
{
    TBuiltInResource resource = { 0 };

    resource.maxLights = 32;
	resource.maxClipPlanes = 6;
	resource.maxTextureUnits = 32;
	resource.maxTextureCoords = 32;
	resource.maxVertexAttribs = 64;
	resource.maxVertexUniformComponents = 4096;
	resource.maxVaryingFloats = 64;
	resource.maxVertexTextureImageUnits = 32;
	resource.maxCombinedTextureImageUnits = 80;
	resource.maxTextureImageUnits = 32;
	resource.maxFragmentUniformComponents = 4096;
	resource.maxDrawBuffers = 32;
	resource.maxVertexUniformVectors = 128;
	resource.maxVaryingVectors = 8;
	resource.maxFragmentUniformVectors = 16;
	resource.maxVertexOutputVectors = 16;
	resource.maxFragmentInputVectors = 15;
	resource.minProgramTexelOffset = -8;
	resource.maxProgramTexelOffset = 7;
	resource.maxClipDistances = 8;
	resource.maxComputeWorkGroupCountX = 65535;
	resource.maxComputeWorkGroupCountY = 65535;
	resource.maxComputeWorkGroupCountZ = 65535;
	resource.maxComputeWorkGroupSizeX = 1024;
	resource.maxComputeWorkGroupSizeY = 1024;
	resource.maxComputeWorkGroupSizeZ = 64;
	resource.maxComputeUniformComponents = 1024;
	resource.maxComputeTextureImageUnits = 16;
	resource.maxComputeImageUniforms = 8;
	resource.maxComputeAtomicCounters = 8;
	resource.maxComputeAtomicCounterBuffers = 1;
	resource.maxVaryingComponents = 60;
	resource.maxVertexOutputComponents = 64;
	resource.maxGeometryInputComponents = 64;
	resource.maxGeometryOutputComponents = 128;
	resource.maxFragmentInputComponents = 128;
	resource.maxImageUnits = 8;
	resource.maxCombinedImageUnitsAndFragmentOutputs = 8;
	resource.maxCombinedShaderOutputResources = 8;
	resource.maxImageSamples = 0;
	resource.maxVertexImageUniforms = 0;
	resource.maxTessControlImageUniforms = 0;
	resource.maxTessEvaluationImageUniforms = 0;
	resource.maxGeometryImageUniforms = 0;
	resource.maxFragmentImageUniforms = 8;
	resource.maxCombinedImageUniforms = 8;
	resource.maxGeometryTextureImageUnits = 16;
	resource.maxGeometryOutputVertices = 256;
	resource.maxGeometryTotalOutputComponents = 1024;
	resource.maxGeometryUniformComponents = 1024;
	resource.maxGeometryVaryingComponents = 64;
	resource.maxTessControlInputComponents = 128;
	resource.maxTessControlOutputComponents = 128;
	resource.maxTessControlTextureImageUnits = 16;
	resource.maxTessControlUniformComponents = 1024;
	resource.maxTessControlTotalOutputComponents = 4096;
	resource.maxTessEvaluationInputComponents = 128;
	resource.maxTessEvaluationOutputComponents = 128;
	resource.maxTessEvaluationTextureImageUnits = 16;
	resource.maxTessEvaluationUniformComponents = 1024;
	resource.maxTessPatchComponents = 120;
	resource.maxPatchVertices = 32;
	resource.maxTessGenLevel = 64;
	resource.maxViewports = 16;
	resource.maxVertexAtomicCounters = 0;
	resource.maxTessControlAtomicCounters = 0;
	resource.maxTessEvaluationAtomicCounters = 0;
	resource.maxGeometryAtomicCounters = 0;
	resource.maxFragmentAtomicCounters = 8;
	resource.maxCombinedAtomicCounters = 8;
	resource.maxAtomicCounterBindings = 1;
	resource.maxVertexAtomicCounterBuffers = 0;
	resource.maxTessControlAtomicCounterBuffers = 0;
	resource.maxTessEvaluationAtomicCounterBuffers = 0;
	resource.maxGeometryAtomicCounterBuffers = 0;
	resource.maxFragmentAtomicCounterBuffers = 1;
	resource.maxCombinedAtomicCounterBuffers = 1;
	resource.maxAtomicCounterBufferSize = 16384;
	resource.maxTransformFeedbackBuffers = 4;
	resource.maxTransformFeedbackInterleavedComponents = 64;
	resource.maxCullDistances = 8;
	resource.maxCombinedClipAndCullDistances = 8;
	resource.maxSamples = 4;
	resource.maxMeshOutputVerticesNV = 256;
	resource.maxMeshOutputPrimitivesNV = 512;
	resource.maxMeshWorkGroupSizeX_NV = 32;
	resource.maxMeshWorkGroupSizeY_NV = 1;
	resource.maxMeshWorkGroupSizeZ_NV = 1;
	resource.maxTaskWorkGroupSizeX_NV = 32;
	resource.maxTaskWorkGroupSizeY_NV = 1;
	resource.maxTaskWorkGroupSizeZ_NV = 1;
	resource.maxMeshViewCountNV = 4;

	resource.limits.nonInductiveForLoops = true;
    resource.limits.generalUniformIndexing = true;
    resource.limits.generalVariableIndexing = true;
    resource.limits.generalVaryingIndexing = true;
    resource.limits.generalSamplerIndexing = true;
    resource.limits.generalAttributeMatrixVectorIndexing = true;
    resource.limits.generalConstantMatrixVectorIndexing = true;

    return resource;
}
std::vector<unsigned int> spirv_fromGLSL(EShLanguage a_lang, const std::string_view& a_str, bool a_optimize)
{
	constexpr EShMessages Messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);

    TRACE("Generating SPIRV");

    glslang::TShader shader = glslang::TShader(a_lang);
	shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_1);

    const char* strs[] =
	{
		a_str.data()
	};
    shader.setStrings(strs, 1);

    constexpr TBuiltInResource Resource = spirv_create_resources();
    if (!shader.parse(&Resource, 100, true, Messages))
    {
		IERROR(std::string(shader.getInfoLog()) + "\n" + shader.getInfoDebugLog() + "\n" + std::string(a_str));

		return std::vector<uint32_t>();
    }

    glslang::TProgram program;
    program.addShader(&shader);

    if (!program.link(Messages))
    {
		IERROR(std::string(shader.getInfoLog()) + "\n" + shader.getInfoDebugLog() + "\n" + std::string(a_str));

		return std::vector<uint32_t>();
    }

	std::vector<unsigned int> spirv;
	spirv.reserve(1024);

	glslang::SpvOptions options = 
	{
		.disableOptimizer = !a_optimize,
		.optimizeSize = true,
#ifdef DEBUG
		.validate = true
#endif
	};
	glslang::TIntermediate* intermediate = program.getIntermediate(a_lang);

    glslang::GlslangToSpv(*intermediate, spirv, &options);
	
    TRACE("Generated SPIRV");

	return spirv;
}

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