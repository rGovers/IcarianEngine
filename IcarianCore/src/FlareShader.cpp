#include "Core/FlareShader.h"

#include "Core/IcarianAssert.h"
#include "Core/ShaderBuffers.h"
#include "Core/StringUtils.h"

#define FSHADER_PLATFORM_UBOSTR(str, platform, argA, argB, structure) \
    switch (platform) \
    { \
    case ShaderPlatform_Vulkan: \
    { \
        str = GLSL_VULKAN_UNIFORM_STRING(argA, argB, structure); \
        break; \
    } \
    case ShaderPlatform_OpenGL: \
    { \
        str = GLSL_OPENGL_UNIFORM_STRING(argA, argB, structure); \
        break; \
    } \
    default: \
    { \
        ICARIAN_ASSERT_MSG_R(0, "Flare Shader invalid shader platform"); \
        break; \
    } \
    }

#define FSHADER_PLATFORM_SSBOSTR(str, platform, argA, argB, structure, name) \
    switch (platform) \
    { \
    case ShaderPlatform_Vulkan: \
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
        ICARIAN_ASSERT_MSG_R(0, "Flare Shader invalid shader platform"); \
        break; \
    } \
    }

#define FSHADER_PLATFORM_PUSHSTR(str, platform, structure, name) \
    switch (platform) \
    { \
    case ShaderPlatform_Vulkan: \
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
        ICARIAN_ASSERT_MSG_R(0, "Flare Shader invalid shader platform"); \
        break; \
    } \
    }

#define FSHADER_PLATORM_TEXTURE(str, platform, type, slot, name) \
    switch (platform) \
    { \
    case ShaderPlatform_Vulkan: \
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
        ICARIAN_ASSERT_MSG_R(0, "Flare Shader invalid shader platform"); \
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
    F(ModelBuffer, GLSL_MODEL_SHADER_STRUCTURE) \
    F(UIBuffer, GLSL_UI_SHADER_STRUCTURE) \
    F(ShadowLightBuffer, GLSL_SHADOW_LIGHT_SHADER_STRUCTURE) \

#define FSHADER_UBO_DEFINITION(str, structure) \
    case StringHash(#str): \
    { \
        FSHADER_PLATFORM_UBOSTR(rStr, a_platform, args[1], args[2], structure); \
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

    std::string GLSLFromFlareShader(const std::string_view& a_str, e_ShaderPlatform a_platform, std::vector<ShaderBufferInput>* a_inputs, std::string* a_error)
    {
        std::string shader = std::string(a_str);

        *a_error = std::string();

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
                {
                    rStr = "layout(set=" + std::string(args[0]) + ",binding=" + std::string(args[0]) + ") uniform sampler2D " + std::string(args[2]) + "[" + std::string(args[1]) + "];";
                    
                    break;
                }
                case ShaderPlatform_OpenGL: 
                {
                    rStr = "layout(location=" + std::string(args[0]) + ") uniform sampler2D " + std::string(args[2]) + "[" + std::string(args[1]) + "];";

                    break;
                }
                default: 
                {
                    ICARIAN_ASSERT_MSG_R(0, "Flare Shader invalid shader platform");

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
                    *a_error = "Flare Shader user buffer requires 2 arguments"; 

                    return std::string();
                }

                switch (a_platform) 
                {
                case ShaderPlatform_Vulkan:
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
                    ICARIAN_ASSERT_MSG_R(0, "Flare Shader invalid shader platform");

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
            case StringHash("instancestructure"):
            {
                if (args.size() != 1)
                {
                    *a_error = "Flare Shader instanced structure requires 1 argument";

                    return std::string();
                }

                switch (a_platform)
                {
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
                    ICARIAN_ASSERT_MSG_R(0, "Flare Shader invalid shader platform");

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
            case StringHash("preloop"):
            {
                if (args.size() != 4)
                {
                    *a_error = "Flare Shader pre loop requires 4 arguements";

                    return std::string();
                }

                const std::string val = args[0];
                const int startIndex = std::stoi(args[1]);
                const int endIndex = std::stoi(args[2]);
                const uint32_t size = val.size();

                for (int i = startIndex; i < endIndex; ++i)
                {
                    const std::string valStr = std::to_string(i);
                    std::string snippet = args[3];
                    
                    std::size_t index = snippet.find(val);
                    while (index != std::string::npos)
                    {
                        snippet.replace(index, size, valStr);

                        index = snippet.find(val);
                    }

                    rStr += snippet;
                }

                break;
            }
            }

            std::size_t next = 1;
            if (!rStr.empty())
            {
                next = rStr.size();
            }

            shader.replace(sPos, eAPos - sPos + 1, rStr);
        }

        return shader;
    }
}