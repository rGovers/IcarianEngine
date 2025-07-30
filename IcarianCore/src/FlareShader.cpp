// Icarian Engine - C# Game Engine
//
// License at end of file.

#include "Core/FlareShader.h"

#include <set>

#include "Core/IcarianAssert.h"
#include "Core/IcarianLambda.h"
#include "Core/ShaderBuffers.h"
#include "Core/StringUtils.h"

#define FSHADER_PLATFORM_UBOSTR(str, platform, argA, argB, structure, name) \
    switch (platform) \
    { \
    case ShaderPlatform_Vulkan: \
    case ShaderPlatform_VulkanCompute: \
    { \
        str = GLSL_VULKAN_UNIFORM_STRING(argA, argB, structure, name); \
        break; \
    } \
    case ShaderPlatform_OpenGL: \
    { \
        str = GLSL_OPENGL_UNIFORM_STRING(argA, argB, structure, name); \
        break; \
    } \
    default: \
    { \
        ICARIAN_ASSERT_MSG(0, "Flare Shader invalid shader platform"); \
        break; \
    } \
    }

#define FSHADER_PLATFORM_SSBOSTR(str, platform, argA, argB, structure, name) \
    switch (platform) \
    { \
    case ShaderPlatform_Vulkan: \
    case ShaderPlatform_VulkanCompute: \
    { \
        str = GLSL_VULKAN_SSBO_STRING(argA, argB, structure, name); \
        break; \
    } \
    case ShaderPlatform_OpenGL: \
    { \
        str = GLSL_OPENGL_SSBO_STRING(argA, argB, structure, name); \
        break; \
    } \
    default: \
    { \
        ICARIAN_ASSERT_MSG(0, "Flare Shader invalid shader platform"); \
        break; \
    } \
    }

#define FSHADER_PLATFORM_PUSHSTR(str, platform, structure, name) \
    switch (platform) \
    { \
    case ShaderPlatform_Vulkan: \
    case ShaderPlatform_VulkanCompute: \
    { \
        str = GLSL_VULKAN_PUSHBUFFER_STRING(name, structure); \
        break; \
    } \
    case ShaderPlatform_OpenGL: \
    { \
        str = GLSL_OPENGL_PUSHBUFFER_STRING(name, structure); \
        break; \
    } \
    default: \
    { \
        ICARIAN_ASSERT_MSG(0, "Flare Shader invalid shader platform"); \
        break; \
    } \
    }

#define FSHADER_PLATORM_TEXTURE(str, platform, type, slot, name) \
    switch (platform) \
    { \
    case ShaderPlatform_Vulkan: \
    case ShaderPlatform_VulkanCompute: \
    { \
        str = "layout(set=" + std::string(slot) + ",binding=" + std::string(slot) + ") uniform " #type " " + std::string(name) + ";"; \
        break; \
    } \
    case ShaderPlatform_OpenGL: \
    { \
        str = "layout(location=" + std::string(slot) + ") uniform " #type " " + std::string(name) + ";"; \
        break; \
    } \
    default: \
    { \
        ICARIAN_ASSERT_MSG(0, "Flare Shader invalid shader platform"); \
        break; \
    } \
    }

#define FSHADER_UBO_STRUCTURETABLE(F) \
    F(CameraBuffer, GLSL_CAMERA_SHADER_STRUCTURE) \
    F(AmbientLightBuffer, GLSL_AMBIENT_LIGHT_SHADER_STRUCTURE) \
    F(DirectionalLightBuffer, GLSL_DIRECTIONAL_LIGHT_SHADER_STRUCTURE) \
    F(PointLightBuffer, GLSL_POINT_LIGHT_SHADER_STRUCTURE) \
    F(SpotLightBuffer, GLSL_SPOT_LIGHT_SHADER_STRUCTURE) \
    F(ShadowLightBuffer, GLSL_SHADOW_LIGHT_SHADER_STRUCTURE) \
    F(TimeBuffer, GLSL_TIME_SHADER_STRUCTURE) \

#define FSHADER_SSBO_STRUCTURETABLE(F) \
    F(AmbientLightBuffer, GLSL_AMBIENT_LIGHT_SSBO_STRUCTURE) \
    F(DirectionalLightBuffer, GLSL_DIRECTIONAL_LIGHT_SSBO_STRUCTURE) \
    F(PointLightBuffer, GLSL_POINT_LIGHT_SSBO_STRUCTURE) \
    F(SpotLightBuffer, GLSL_SPOT_LIGHT_SSBO_STRUCTURE) \
    F(ModelBuffer, GLSL_MODEL_SSBO_STRUCTURE) \
    F(BoneBuffer, GLSL_BONE_SSBO_STRUCTURE) \
    F(ShadowLightBuffer, GLSL_SHADOW_LIGHT_SSBO_STRUCTURE) \
    F(ParticleBuffer, GLSL_PARTICLE_SSBO_STRUCTURE) \

#define FSHADER_PUSHBUFFER_STRUCTURETABLE(F) \
    F(ModelBuffer, GLSL_MODEL_PUSH_STRUCTURE) \
    F(UIBuffer, GLSL_UI_PUSH_STRUCTURE) \
    F(ShadowLightBuffer, GLSL_SHADOW_LIGHT_PUSH_STRUCTURE) \

#define FSHADER_UBO_DEFINITION(str, structure) \
    case StringHash(#str): \
    { \
        FSHADER_PLATFORM_UBOSTR(rStr, a_platform, args[1], args[2], structure, #str); \
        const ShaderBufferInput input = \
        { \
            .Slot = (uint16_t)std::stoi(args[1]), \
            .BufferType = ShaderBufferType_##str, \
        }; \
        a_inputs->emplace_back(input); \
        break; \
    } \

#define FSHADER_SSBO_DEFINITION(str, structure) \
    case StringHash("SS" #str): \
    { \
        FSHADER_PLATFORM_SSBOSTR(rStr, a_platform, args[1], args[2], structure, #str); \
        const ShaderBufferInput input = \
        { \
            .Slot = (uint16_t)std::stoi(args[1]), \
            .BufferType = ShaderBufferType_SS##str, \
        }; \
        a_inputs->emplace_back(input); \
        break; \
    } \

#define FSHADER_PUSHBUFFER_DEFINITION(str, structure) \
    case StringHash("P" #str): \
    { \
        FSHADER_PLATFORM_PUSHSTR(rStr, a_platform, structure, args[1]); \
        const ShaderBufferInput input = \
        { \
            .Slot = uint16_t(-1), \
            .BufferType = ShaderBufferType_P##str, \
        }; \
        a_inputs->emplace_back(input); \
        break; \
    } \

#define FSHADER_UBO FSHADER_UBO_STRUCTURETABLE(FSHADER_UBO_DEFINITION)
#define FSHADER_SSBO FSHADER_SSBO_STRUCTURETABLE(FSHADER_SSBO_DEFINITION)
#define FSHADER_PUSHBUFFER FSHADER_PUSHBUFFER_STRUCTURETABLE(FSHADER_PUSHBUFFER_DEFINITION)

namespace IcarianCore
{
    static uint32_t SplitArgs(const std::string_view& a_string, std::vector<std::string>* a_args)
    {
        const char* start = a_string.data();
        const char* iter = start;
        const char* prevIter = start;

        const uint32_t len = (uint32_t)a_string.length();

        int32_t block = 0;
        int32_t scope = 1;
        while (true)
        {
            if (iter - start >= len)
            {
                if (*iter == '}')
                {
                    --block;
                }

                a_args->emplace_back(a_string.substr(prevIter - start, iter - prevIter));

                ICARIAN_ASSERT(block == 0);

                return iter - start + 1;
            }

            switch (*iter)
            {
            case '(':
            {
                ++scope;

                break;
            }
            case ')':
            {
                if (--scope == 0)
                {
                    a_args->emplace_back(a_string.substr(prevIter - start, iter - prevIter));

                    ICARIAN_ASSERT(block == 0);

                    return iter - start + 1;
                }

                break;
            }
            case '{':
            {
                ++block;

                break;
            }
            case '}':
            {
                --block;

                break;
            }
            case ',':
            {
                if (block == 0)
                {
                    a_args->emplace_back(a_string.substr(prevIter - start, iter - prevIter));

                    prevIter = iter + 1;
                }

                break;
            }
            }

            ++iter;
        }

    	return -1;
    }

    static std::string TrimWhitespace(const std::string_view& a_string)
    {
        const std::size_t start = a_string.find_first_not_of(' ');
        const std::size_t end = a_string.find_last_not_of(' ');

        return std::string(a_string.substr(start, end + 1));
    }

    typedef uint32_t MeshOutCaseType;

#define MESHOUT_CASE(value, size) case StringHash<MeshOutCaseType>(#value): { ILRETURN IC_MESHOUT_ENUMVAL(value); }

#define MESHOUT_STR(value, size) #value,

    constexpr const char* MeshOutString[] =
    {
        IC_MESHOUT_TABLE(MESHOUT_STR)
    };

    // TODO: At some point build an actual shader language as this is starting to get out of hand with the pre-preprocessor
    std::string GLSLFromFlareShader(const std::string_view& a_str, e_ShaderPlatform a_platform, const std::unordered_map<std::string, std::string>& a_imports, std::vector<ShaderBufferInput>* a_inputs, std::string* a_error, ShaderOutput* a_out)
    {
        std::string shader = std::string(a_str);

        std::set<std::string> imported;
        std::vector<MeshShaderOut> meshOutputs;

        *a_error = std::string();

        if (a_out != nullptr)
        {
            *a_out = ShaderOutput();
        }

        bool isMesh = false;
        MeshShaderData meshData = MeshShaderData();

        const size_t versionIndex = shader.find("#version");
        if (versionIndex == std::string::npos)
        {
            *a_error = "No Flare Shader GLSL version";

            return std::string();
        }

        const size_t versionNext = shader.find('\n', versionIndex);
        if (versionNext == std::string::npos)
        {
            *a_error = "Flare Shader no new line after version";

            return std::string();
        }

        while (true)
        {
            const std::size_t sPos = shader.find("#!");
            if (sPos == std::string::npos)
            {
                break;
            }

            const std::size_t sAPos = shader.find('(', sPos + 1);
            if (sAPos == std::string::npos)
            {
                *a_error = "Invalid Flare Shader definition: " + std::to_string(sPos);

                return std::string();
            }

            const std::string defName = shader.substr(sPos + 2, sAPos - sPos - 2);
            // Could probably have a single array with a double null terminator to determine the end
            // Not an issue at the moment but could reduce allocations and jumping around in memory
            // Potential improvement if performance becomes an issue
            std::vector<std::string> args;
            const std::size_t eAPos = sAPos + SplitArgs(shader.data() + sAPos + 1, &args);

            std::string rStr;
            switch (StringHash(defName.c_str()))
            {
            case StringHash("workgroup"):
            {
                if (args.size() != 3)
                {
                    *a_error = "Flare Shader workgroup requires 3 arguments";

                    return std::string();
                }

                rStr = "layout(local_size_x=" + args[0] + ",local_size_y=" + args[1] + ",local_size_z=" + args[2] + ") in;";

                if (a_out != nullptr)
                {
                    a_out->Workgroups.GroupX = (uint32_t)std::stoi(args[0]);
                    a_out->Workgroups.GroupY = (uint32_t)std::stoi(args[1]);
                    a_out->Workgroups.GroupZ = (uint32_t)std::stoi(args[2]);
                }

                break;
            }
            case StringHash("meshprimitives"):
            {
                if (args.size() != 3)
                {
                    *a_error = "Flare Shader meshprimitives requires 3 arguments";

                    return std::string();
                }

                isMesh = true;

                const std::string argStr = TrimWhitespace(args[0]);

                const e_MeshShaderPrimitive primitiveType = ILAMBDA(
                {
                    switch (StringHash<uint32_t>(argStr.c_str()))
                    {
                    case StringHash<uint32_t>("triangles"):
                    {
                        ILRETURN MeshShaderPrimitive_Triangle;
                    }
                    case StringHash<uint32_t>("lines"):
                    {
                        ILRETURN MeshShaderPrimitive_Line;
                    }
                    case StringHash<uint32_t>("points"):
                    {
                        ILRETURN MeshShaderPrimitive_Point;
                    }
                    }

                    ILRETURN MeshShaderPrimitive_Null;
                });

                if (primitiveType == MeshShaderPrimitive_Null)
                {
                    *a_error = "Flare Shader invalid Mesh Primitive type";

                    return std::string();
                }

                meshData.MaxVertices = (uint32_t)std::stoi(args[1]);
                meshData.MaxPrimitives = (uint32_t)std::stoi(args[2]);
                meshData.PrimitiveType = primitiveType;

                switch (a_platform)
                {
                case ShaderPlatform_Vulkan:
                {
                    rStr = "layout(" + argStr + ", max_vertices=" + args[1] + ", max_primitives=" + args[2] + ") out;";

                    break;
                }
                default:
                {
                    ICARIAN_ASSERT_MSG(0, "Flare Shader invalid shader platform");

                    break;
                }
                }

                break;
            }
            case StringHash("meshout"):
            {
                if (args.size() != 3)
                {
                    *a_error = "Flare Shader meshout requires 3 arguments";

                    return std::string();
                }

                isMesh = true;

                const std::string argStr = TrimWhitespace(args[1]);

                const e_MeshOutType outType = ILAMBDA(
                {
                    switch (StringHash<MeshOutCaseType>(argStr.c_str()))
                    {
                    IC_MESHOUT_TABLE(MESHOUT_CASE)
                    }

                    ILRETURN MeshOutType_Last;
                });

                if (outType == MeshOutType_Last)
                {
                    *a_error = "Flare Shader invalid mesh output type " + args[1];

                    return std::string();
                }

                const std::string iden = TrimWhitespace(args[2]);

                const MeshShaderOut meshOut =
                {
                    .Identifier = iden,
                    .Slot = (uint32_t)std::stoi(args[0]),
                    .Type = outType,
                };

                meshOutputs.emplace_back(meshOut);

                break;
            }
            case StringHash("setmeshoutput"):
            {
                if (args.size() != 2)
                {
                    *a_error = "Flare Shader setmeshoutput requires 2 arguments";

                    return std::string();
                }

                isMesh = true;

                switch (a_platform)
                {
                case ShaderPlatform_Vulkan:
                {
                    rStr = "SetMeshOutputsEXT(" + args[0] + "," + args[1] + ")";

                    break;
                }
                case ShaderPlatform_VulkanCompute:
                {
                    if (meshData.MaxPrimitives <= 0)
                    {
                        *a_error = "Flare Shader setmeshout with no max primitives";

                        return std::string();
                    }

                    // Emulating mesh shaders so need to prefill the non populated values
                    // I need to validate that all hardware culls data that shares points
                    // If not I may need to populate the buffer with a vertex that is offscreen and use that as null
                    rStr += "for (int i = " + args[1] + "; i < " + std::to_string(meshData.MaxPrimitives) + "; i++)"
                    "{";

                    switch (meshData.PrimitiveType)
                    {
                    case MeshShaderPrimitive_Triangle:
                    {
                        rStr += "   fl_indices[i] = uvec3(0, 0, 0);";

                        break;
                    }
                    case MeshShaderPrimitive_Line:
                    {
                        rStr += "   fl_indices[i] = uvec2(0, 0);";

                        break;
                    }
                    case MeshShaderPrimitive_Point:
                    {
                        // TODO: This will not work change this
                        rStr += "   fl_indices[i] = uint(0);";

                        break;
                    }
                    default:
                    {
                        *a_error = "Flare Shader setmeshout with invalid Mesh Primitive Type";

                        return std::string();
                    }
                    }

                    rStr += "}";

                    break;
                }
                default:
                {
                    ICARIAN_ASSERT_MSG(0, "Flare Shader invalid shader platform");

                    break;
                }
                }

                break;
            }
            case StringHash("meshpayload"):
            {
                if (args.size() != 1)
                {
                    *a_error = "Flare Shader meshpayload requires 1 argument";

                    return std::string();
                }

                isMesh = true;

                switch (a_platform)
                {
                case ShaderPlatform_Vulkan:
                {
                    rStr = "fl_vertOut[" + args[0] + "]";

                    break;
                }
                case ShaderPlatform_VulkanCompute:
                {
                    rStr = "fl_vertOut.vertices[gl_GlobalInvocationID.x * " + std::to_string(meshData.MaxVertices) + " + (" + args[0] + ")]";

                    break;
                }
                default:
                {
                    ICARIAN_ASSERT_MSG(0, "Flare Shader invalid shader platform");

                    break;
                }
                }

                break;
            }
            case StringHash("meshposition"):
            {
                if (args.size() != 1)
                {
                    *a_error = "Flare Shader meshposition requires 1 argument";

                    return std::string();
                }

                isMesh = true;

                switch (a_platform)
                {
                case ShaderPlatform_Vulkan:
                {
                    rStr = "gl_MeshVerticesEXT[" + args[0] + "].gl_Position";

                    break;
                }
                case ShaderPlatform_VulkanCompute:
                {
                    rStr = "fl_vertOut.vertices[gl_GlobalInvocationID.x * " + std::to_string(meshData.MaxVertices) + " + (" + args[0] + ")].position";

                    break;
                }
                default:
                {
                    ICARIAN_ASSERT_MSG(0, "Flare Shader invalid shader platform");

                    break;
                }
                }

                break;
            }
            case StringHash("meshtri"):
            {
                if (args.size() != 1)
                {
                    *a_error = "Flare Shader meshtri requires 1 argument";

                    return std::string();
                }

                isMesh = true;

                switch (a_platform)
                {
                case ShaderPlatform_Vulkan:
                {
                    rStr = "gl_PrimitiveTriangleIndicesEXT[" + args[0] + "]";

                    break;
                }
                case ShaderPlatform_VulkanCompute:
                {
                    rStr = "fl_indices[" + args[0] + "]";

                    break;
                }
                default:
                {
                    ICARIAN_ASSERT_MSG(0, "Flare Shader invalid shader platform");

                    break;
                }
                }

                break;
            }
            case StringHash("structure"):
            {
                if (args.size() != 3)
                {
                    *a_error = "Flare Shader structure requires 3 arguments";

                    return std::string();
                }

                // I am lazy therefore let the pre processor write it
                switch (StringHash(args[0].c_str()))
                {
                FSHADER_UBO
                FSHADER_SSBO
                }

                break;
            }
            case StringHash("meshvertex"):
            {
                if (args.size() != 3)
                {
                    *a_error = "Flare Shader meshvertex reqires 3 arguments";
                }

                if (a_platform == ShaderPlatform_OpenGL)
                {
                    *a_error = "Flare Shader mesh unsupported on OpenGL";

                    return std::string();
                }

                rStr = "struct Vertex " + args[2] + "; layout(std430,set=" + args[0] + ",binding=" + args[0] + ") readonly buffer MeshVertices { Vertex Vertices[]; } " + args[1] + ";";

                const ShaderBufferInput input =
                {
                    .Slot = (uint16_t)std::stoi(args[0]),
                    .BufferType = ShaderBufferType_MeshVertex,
                    .Count = 1
                };

                a_inputs->emplace_back(input);

                break;
            }
            case StringHash("meshletvertices"):
            {
                if (args.size() != 2)
                {
                    *a_error = "Flare Shader meshletvertices requires 2 arguments";

                    return std::string();
                }

                if (a_platform == ShaderPlatform_OpenGL)
                {
                    *a_error = "Flare Shader mesh unsupported on OpenGL";

                    return std::string();
                }

                rStr = "layout(std430,set=" + args[0] + ",binding=" + args[0] + ") readonly buffer MeshletVertices { uint Vertices[]; } " + args[1] + ";";

                const ShaderBufferInput input =
                {
                    .Slot = (uint16_t)std::stoi(args[0]),
                    .BufferType = ShaderBufferType_MeshletVertices,
                    .Count = 1,
                };

                a_inputs->emplace_back(input);

                break;
            }
            case StringHash("meshlettriangles"):
            {
                if (args.size() != 2)
                {
                    *a_error = "Flare Shader meshlettriangles requires 2 arguments";

                    return std::string();
                }

                if (a_platform == ShaderPlatform_OpenGL)
                {
                    *a_error = "Flare Shader mesh unsupported on OpenGL";

                    return std::string();
                }

                // It is actually a uint8_t so users will need to use meshlettriangledata to extract the actual value
                rStr = "layout(std430,set=" + args[0] + ",binding=" + args[0] + ") readonly buffer MeshletIndices { uint Indices[]; } " + args[1] + ";";

                const ShaderBufferInput input =
                {
                    .Slot = (uint16_t)std::stoi(args[0]),
                    .BufferType = ShaderBufferType_MeshletTriangles,
                    .Count = 1
                };

                a_inputs->emplace_back(input);

                break;
            }
            case StringHash("meshlet"):
            {
                if (args.size() != 2)
                {
                    *a_error = "Flare Shader meshlet requires 2 arguments";

                    return std::string();
                }

                if (a_platform == ShaderPlatform_OpenGL)
                {
                    *a_error = "Flare Shader mesh unsupported on OpenGL";

                    return std::string();
                }

                rStr = "struct MeshletData { uvec4 Data; vec4 Bounds; }; layout(std140,set=" + args[0] + ",binding=" + args[0] + ") readonly buffer Meshlets { MeshletData Meshlets[]; } " + args[1] + ";";

                const ShaderBufferInput input =
                {
                    .Slot = (uint16_t)std::stoi(args[0]),
                    .BufferType = ShaderBufferType_Meshlet,
                    .Count = 1
                };

                a_inputs->emplace_back(input);

                break;
            }
            case StringHash("meshlettriangledata"):
            {
                if (args.size() != 2)
                {
                    *a_error = "Flare Shader meshlettriangledata requires 2 arguments";

                    return std::string();
                }

                if (a_platform == ShaderPlatform_OpenGL)
                {
                    *a_error = "Flare Shader mesh unsupported on OpenGL";

                    return std::string();
                }

                // GLSL does not support uint8 so need to do some bit fiddling to extract the true value
                // Yes I have noticed the extension but that requires querying device features or we can just do some bit fiddling
                rStr = "((" + args[0] + ".Indices[(" + args[1] + ") / 4] >> ((" + args[1] + ") % 4 * 8)) & 0xFF)";

                break;
            }
            case StringHash("buffertexture"):
            {
                if (a_platform != ShaderPlatform_VulkanCompute)
                {
                    *a_error = "Flare Shader buffer texture not available on non compute platform";

                    return std::string();
                }

                rStr = "layout(" + args[0] + ",set=" + args[1] + ",binding=" + args[1] + ") uniform image2D" + args[3] + ";";

                const ShaderBufferInput input =
                {
                    .Slot = (uint16_t)std::stoi(args[1]),
                    .BufferType = ShaderBufferType_BufferTexture,
                    .Count = (uint16_t)std::stoi(args[2])
                };

                a_inputs->emplace_back(input);

                break;
            }
            case StringHash("texture"):
            {
                if (args.size() != 2)
                {
                    *a_error = "Flare Shader texture requires 2 arguments";

                    return std::string();
                }

                FSHADER_PLATORM_TEXTURE(rStr, a_platform, sampler2D, args[0], args[1]);

                const ShaderBufferInput input =
                {
                    .Slot = (uint16_t)std::stoi(args[0]),
                    .BufferType = ShaderBufferType_Texture
                };

                a_inputs->emplace_back(input);

                break;
            }
            case StringHash("pushtexture"):
            {
                if (args.size() != 2)
                {
                    *a_error = "Flare Shader pushtexture requires 2 arguments";

                    return std::string();
                }

                FSHADER_PLATORM_TEXTURE(rStr, a_platform, sampler2D, args[0], args[1]);

                const ShaderBufferInput input =
                {
                    .Slot = (uint16_t)std::stoi(args[0]),
                    .BufferType = ShaderBufferType_PushTexture
                };

                a_inputs->emplace_back(input);

                break;
            }
            case StringHash("shadowtexture"):
            {
                if (args.size() != 2)
                {
                    *a_error = "Flare Shader shadowtexture requires 2 arguments";

                    return std::string();
                }

                FSHADER_PLATORM_TEXTURE(rStr, a_platform, sampler2D, args[0], args[1]);

                const ShaderBufferInput input =
                {
                    .Slot = (uint16_t)std::stoi(args[0]),
                    .BufferType = ShaderBufferType_ShadowTexture2D
                };

                a_inputs->emplace_back(input);

                break;
            }
            case StringHash("cubeshadowtexture"):
            {
                if (args.size() != 2)
                {
                    *a_error = "Flare Shader cubeshadowtexture requires 2 arguments";

                    return std::string();
                }

                FSHADER_PLATORM_TEXTURE(rStr, a_platform, samplerCube, args[0], args[1]);

                const ShaderBufferInput input =
                {
                    .Slot = (uint16_t)std::stoi(args[0]),
                    .BufferType = ShaderBufferType_ShadowTextureCube
                };

                a_inputs->emplace_back(input);

                break;
            }
            case StringHash("shadowtexturearray"):
            {
                if (args.size() != 3)
                {
                    *a_error = "Flare Shader shadowtexturearray requires 3 arguments";

                    return std::string();
                }

                switch (a_platform)
                {
                case ShaderPlatform_Vulkan:
                case ShaderPlatform_VulkanCompute:
                {
                    rStr = "layout(set=" + args[0] + ",binding=" + args[0] + ") uniform sampler2D " + args[2] + "[" + args[1] + "];";

                    break;
                }
                case ShaderPlatform_OpenGL:
                {
                    rStr = "layout(location=" + args[0] + ") uniform sampler2D " + args[2] + "[" + args[1] + "];";

                    break;
                }
                default:
                {
                    ICARIAN_ASSERT_MSG(0, "Flare Shader invalid shader platform");

                    break;
                }
                }

                const ShaderBufferInput input =
                {
                    .Slot = (uint16_t)std::stoi(args[0]),
                    .BufferType = ShaderBufferType_AShadowTexture2D,
                    .Count = (uint16_t)std::stoi(args[1])
                };

                a_inputs->emplace_back(input);

                break;
            }
            case StringHash("userbuffer"):
            {
                if (args.size() != 3)
                {
                    *a_error = "Flare Shader user buffer requires 3 arguments";

                    return std::string();
                }

                switch (a_platform)
                {
                case ShaderPlatform_Vulkan:
                case ShaderPlatform_VulkanCompute:
                {
                    rStr = "layout(std140,binding=" + args[0] + ",set=" + args[0] + ") uniform UserBuffer " + args[1] + " " + args[2] + ";";

                    break;
                }
                case ShaderPlatform_OpenGL:
                {
                    rStr = "layout(std140,binding=" + args[0] + ") uniform UserBuffer " + args[1] + " " + args[2] + ";";

                    break;
                }
                default:
                {
                    ICARIAN_ASSERT_MSG(0, "Flare Shader invalid shader platform");

                    break;
                }
                }

                const ShaderBufferInput input =
                {
                    .Slot = (uint16_t)std::stoi(args[0]),
                    .BufferType = ShaderBufferType_UserUBO
                };

                a_inputs->emplace_back(input);

                break;
            }
            case StringHash("userarray"):
            {
                if (args.size() != 3)
                {
                    *a_error = "Flare Shader user array requires 3 arguments";

                    return std::string();
                }

                switch (a_platform)
                {
                case ShaderPlatform_Vulkan:
                case ShaderPlatform_VulkanCompute:
                {
                    rStr = "struct UserArrayData " + args[1] + "; layout(std140,binding=" + args[0] + ",set=" + args[0] + ") readonly buffer UserArray { int Count; UserArrayData objects[]; } " + args[2] + ";";

                    break;
                }
                case ShaderPlatform_OpenGL:
                {
                    rStr = "struct UserArrayData " + args[1] + "; layout(std140,binding=" + args[0] + ") readonly buffer UserArray { int Count; UserArrayData objects[]; } " + args[2] + ";";

                    break;
                }
                default:
                {
                    ICARIAN_ASSERT_MSG(0, "Flare Shader invalid shader platform");
                }
                }

                const ShaderBufferInput input =
                {
                    .Slot = (uint16_t)std::stoi(args[0]),
                    .BufferType = ShaderBufferType_UserArray
                };

                a_inputs->emplace_back(input);

                break;
            }
            case StringHash("instancestructure"):
            {
                if (args.size() != 1)
                {
                    *a_error = "Flare Shader instanced structure requires 1 argument";

                    return std::string();
                }

                switch (a_platform)
                {
                case ShaderPlatform_VulkanCompute:
                {
                    *a_error = "Flare Shader instanced structure used in compute mode";

                    return std::string();
                }
                case ShaderPlatform_Vulkan:
                {
                    rStr = args[0] + ".objects[gl_InstanceIndex]";

                    break;
                }
                case ShaderPlatform_OpenGL:
                {
                    rStr = args[0] + ".objects[gl_InstanceID]";

                    break;
                }
                default:
                {
                    ICARIAN_ASSERT_MSG(0, "Flare Shader invalid shader platform");

                    break;
                }
                }

                break;
            }
            case StringHash("pushbuffer"):
            {
                if (args.size() != 2)
                {
                    *a_error = "Flare Shader push buffer requires 2 arguments";

                    return std::string();
                }

                switch (StringHash(args[0].c_str()))
                {
                FSHADER_PUSHBUFFER
                }

                break;
            }
            // Exists because optimization is terrible with unrolling loops so a pre processor loop is needed
            case StringHash("preloop"):
            {
                if (args.size() != 4)
                {
                    *a_error = "Flare Shader pre loop requires 4 arguments";

                    return std::string();
                }

                const std::string val = args[0];
                const int startIndex = std::stoi(args[1]);
                const int endIndex = std::stoi(args[2]);
                const uint32_t size = val.size();

                // Only gets optimized away if there is no break
                // Therefore there is no cost if you do not break
                // Do this to allow breaking in preprocessor loops
                // Potential for no cost with a break but depends on vendor SPIRV compiler so mileage my vary
                rStr += "switch(0) { \n"
                    "default: { \n";

                for (int i = startIndex; i < endIndex; ++i)
                {
                    const std::string valStr = std::to_string(i);
                    std::string snippet = args[3];

                    while (true)
                    {
                        const std::size_t index = snippet.find(val);
                        if (index == std::string::npos)
                        {
                            break;
                        }

                        snippet.replace(index, size, valStr);
                    }

                    rStr += snippet;
                }

                rStr += "}} \n";

                break;
            }
            case StringHash("import"):
            {
                if (args.size() != 1)
                {
                    *a_error = "Flare Shader import requires 1 argument";

                    return std::string();
                }

                const std::string val = args[0];

                if (imported.find(val) != imported.end())
                {
                    break;
                }

                const auto iter = a_imports.find(val);
                if (iter != a_imports.end())
                {
                    rStr = iter->second;

                    imported.emplace(val);

                    break;
                }

                *a_error = "Flare Shader no import found: " + val;

                return std::string();
            }
            }

            shader.replace(sPos, eAPos - sPos + 1, rStr);
        }

        if (isMesh)
        {
            switch (a_platform)
            {
            case ShaderPlatform_Vulkan:
            {
                std::string s = "layout(location=0) out PerVertexData"
                "{";

                for (const MeshShaderOut& out : meshOutputs)
                {
                    s += std::string(MeshOutString[out.Type]) + " " + out.Identifier + ";";
                }

                s += "}"
                " fl_vertOut[];\n";

                shader.insert(versionNext + 1, s);

                shader.insert(versionNext + 1, "\n#extension GL_EXT_mesh_shader : require\n");

                break;
            }
            case ShaderPlatform_VulkanCompute:
            {
                if (meshData.MaxPrimitives <= 0 || meshData.MaxVertices <= 0)
                {
                    *a_error = "Flare Shader Mesh Shader but Max Primitives or Max Vertices not set";

                    return std::string();
                }

                {
                    std::string s = "struct fl_MeshVertexOut"
                    "{"
                    "   vec4 position;";

                    for (const MeshShaderOut& out : meshOutputs)
                    {
                        s += std::string(MeshOutString[out.Type]) + " " + out.Identifier + ";";
                    }

                    s += "}"

                    " layout(std140,binding=" + std::to_string(MeshShaderEmulationVertexBufferOutput) + ",set=" + std::to_string(MeshShaderEmulationVertexBufferOutput) + ") buffer fl_MeshShaderVertexBuffer"
                    "{"
                    "   fl_MeshVertexOut vertices[];"
                    "} fl_vertOut;\n";

                    shader.insert(versionNext + 1, s);
                }

                {
                    const std::string s = "layout(std140,binding=" + std::to_string(MeshShaderEmulationIndexBufferOutput) + ",set=" + std::to_string(MeshShaderEmulationIndexBufferOutput) + ") buffer gl_MeshShaderIndexBuffer"
                    "{"
                    "   uint indices[]"
                    "} fl_indexOut;\n";

                    shader.insert(versionNext + 1, s);
                }

                {
                    std::string s = std::string();

                    switch (meshData.PrimitiveType)
                    {
                    case MeshShaderPrimitive_Triangle:
                    {
                        s += "uvec3";

                        break;
                    }
                    case MeshShaderPrimitive_Line:
                    {
                        s += "uvec2";

                        break;
                    }
                    case MeshShaderPrimitive_Point:
                    {
                        s += "uint";

                        break;
                    }
                    default:
                    {
                        *a_error = "Flare Shader Mesh Shader invalid Primitive Type";

                        return std::string();
                    }
                    }

                    s += " fl_indices[" + std::to_string(meshData.MaxPrimitives) + "];\n";

                    shader.insert(versionNext + 1, s);
                }

                break;
            }
            default:
            {
                ICARIAN_ASSERT_MSG(0, "Flare Shader invalid shader platform");

                break;
            }
            }
        }

        if (a_out != nullptr)
        {
            a_out->MeshData = meshData;
            a_out->MeshOutputs = meshOutputs;
        }

        return shader;
    }

    std::string GenerateMeshVertexStub(const std::vector<MeshShaderOut>& a_outputs)
    {
        // TODO: Can probably change this function to just generate the SPIR-V directly down the line
        // Is simple enough that should not need to do much and seem like a good place to start with SPIR-V
        std::string inputs;
        std::string outputs;
        std::string body;

        if (a_outputs.size() >= std::numeric_limits<uint32_t>::max())
        {
            return std::string();
        }
        const uint32_t count = (uint32_t)a_outputs.size();

        for (uint32_t i = 0; i < count; ++i)
        {
            const MeshShaderOut& out = a_outputs[i];
            if (out.Type >= MeshOutType_Last)
            {
                return std::string();
            }

            const char* typeStr = MeshOutString[out.Type];

            const std::string inStr = "fl_" + out.Identifier;

            const std::string inputSlot = std::to_string(i + 1);
            const std::string outputSlot = std::to_string(out.Slot);

            inputs += "layout(location=" + inputSlot + ") in " + typeStr + inStr + ";\n";
            outputs += "layout(location=" + outputSlot + ") out " + typeStr + out.Identifier + ";\n";

            body += "    " + out.Identifier + "=" + inStr + "\n;";
        }

        return "#version 450 \n"

        "layout(location=0) in vec4 fl_position;\n" +
        inputs +

        outputs +

        "void main()\n"
        "{\n"
        "   gl_Position = fl_position\n;" +

        body +
        "}\n";
    }
}

// MIT License
//
// Copyright (c) 2025 River Govers
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
