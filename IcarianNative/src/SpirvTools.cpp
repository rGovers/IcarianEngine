#include "Rendering/SpirvTools.h"

#include "Flare/IcarianAssert.h"
#include "Logger.h"
#include "Rendering/ShaderBuffers.h"
#include "Trace.h"

static std::vector<std::string> SplitArgs(const std::string_view& a_string)
{
	std::vector<std::string> args;

	std::size_t pos = 0;

	while (true)
	{
		while (a_string[pos] == ' ')
		{
			++pos;
		}

		const std::size_t sPos = a_string.find(',', pos);
		if (sPos == std::string_view::npos)
		{
			args.emplace_back(a_string.substr(pos));

			break;
		}

		args.emplace_back(a_string.substr(pos, sPos - pos));
		pos = sPos + 1;
	}
	
	return args;
}

// Dont know why I never though of using the C++ preprocessor to write my shader uniforms for me 
// My laziness knows no bounds
std::string GLSL_fromFShader(const std::string_view& a_str)
{
	std::string shader = std::string(a_str);

	std::size_t pos = 0;
	while (true)
	{
		const std::size_t sPos = shader.find("#!", pos);
		if (sPos == std::string::npos)
		{
			break;
		}

		const std::size_t sAPos = shader.find("(", sPos + 1);
		const std::size_t eAPos = shader.find(')', sPos + 1);

		ICARIAN_ASSERT_MSG_R(sAPos != std::string::npos && eAPos != std::string::npos, "Invalid Flare Shader definition at " + std::to_string(sPos));
		ICARIAN_ASSERT_MSG_R(sAPos < eAPos, "Invalid Flare Shader braces at " + std::to_string(sPos));

		const std::string defName = shader.substr(sPos + 2, sAPos - sPos - 2);
		std::vector<std::string> args = SplitArgs(shader.substr(sAPos + 1, eAPos - sAPos - 1));

		std::string rStr;
		if (defName == "structure")
		{
			ICARIAN_ASSERT_MSG_R(args.size() == 4, "Flare Shader structure requires 3 arguments");

			if (args[0] == "CameraBuffer")
			{
				rStr = GLSL_UNIFORM_STRING(args[1], args[2], args[3], GLSL_CAMERA_SHADER_STRUCTURE);
			}
			else if (args[0] == "DirectionalLightBuffer")
			{
				rStr = GLSL_UNIFORM_STRING(args[1], args[2], args[3], GLSL_DIRECTIONAL_LIGHT_SHADER_STRUCTURE);
			}
			else if (args[0] == "PointLightBuffer")
			{
				rStr = GLSL_UNIFORM_STRING(args[1], args[2], args[3], GLSL_POINT_LIGHT_SHADER_STRUCTURE);
			}
			else if (args[0] == "SpotLightBuffer")
			{
				rStr = GLSL_UNIFORM_STRING(args[1], args[2], args[3], GLSL_SPOT_LIGHT_SHADER_STRUCTURE);
			}
			else if (args[0] == "TimeBuffer")
			{
				rStr = GLSL_UNIFORM_STRING(args[1], args[2], args[3], GLSL_TIME_SHADER_STRUCTURE);
			}
		}
		else if (defName == "pushbuffer")
		{
			ICARIAN_ASSERT_MSG_R(args.size() == 2, "Flare Shader push buffer requires 2 arguments");

			if (args[0] == "ModelBuffer")
			{
				rStr = GLSL_PUSHBUFFER_STRING(args[1], GLSL_MODEL_SHADER_STRUCTURE);
			}
			else if (args[0] == "UIBuffer")
			{
				rStr = GLSL_PUSHBUFFER_STRING(args[1], GLSL_UI_SHADER_STRUCTURE);
			}
		}

		std::size_t next = 1;
		if (!rStr.empty())
		{
			next = rStr.size();
		}

		shader.replace(sPos, eAPos - sPos + 1, rStr);

		pos = sPos + next;
	}

	return shader;
}

void spirv_init()
{
    glslang::InitializeProcess();
}
void spirv_destroy()
{
    glslang::FinalizeProcess();   
}
TBuiltInResource spirv_create_resources()
{
    TBuiltInResource resource;
    memset(&resource, 0, sizeof(resource));

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

    resource.limits.generalUniformIndexing = true;
    resource.limits.generalVariableIndexing = true;
    resource.limits.generalVaryingIndexing = true;
    resource.limits.generalSamplerIndexing = true;
    resource.limits.generalAttributeMatrixVectorIndexing = true;
    resource.limits.generalConstantMatrixVectorIndexing = true;

    return resource;
}
std::vector<unsigned int> spirv_fromGLSL(EShLanguage a_lang, const std::string_view& a_str)
{
	std::vector<unsigned int> spirv;

	constexpr EShMessages Messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);

    TRACE("Generating SPIRV");

    glslang::TShader shader = glslang::TShader(a_lang);

    const char* strs[1];
    strs[0] = a_str.data();
    shader.setStrings(strs, 1);

    const TBuiltInResource resource = spirv_create_resources();

    if (!shader.parse(&resource, 100, false, Messages))
    {
		Logger::Error(std::string(shader.getInfoLog()) + "\n" + shader.getInfoDebugLog());

        return spirv;
    }

    glslang::TProgram program;
    program.addShader(&shader);

    if (!program.link(Messages))
    {
		Logger::Error(std::string(shader.getInfoLog()) + "\n" + shader.getInfoDebugLog());

        return spirv;
    }

    
    glslang::GlslangToSpv(*program.getIntermediate(a_lang), spirv);
    
    TRACE("Generated SPIRV");

	return spirv;
}