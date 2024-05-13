#include "Core/FlareShader.h"

#include "Core/IcarianAssert.h"
#include "Core/ShaderBuffers.h"

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
    if (args[0] == #str) \
    { \
        FSHADER_PLATFORM_UBOSTR(rStr, a_platform, args[1], args[2], structure); \
        const ShaderBufferInput input = \
        { \
            .Slot = (uint16_t)std::stoi(args[1]), \
            .BufferType = ShaderBufferType_##str, \
        }; \
        a_inputs->emplace_back(input); \
    } \
    
#define FSHADER_SSBO_DEFINITION(str, structure) \
    if (args[0] == "SS" #str) \
    { \
        FSHADER_PLATFORM_SSBOSTR(rStr, a_platform, args[1], args[2], structure, #str); \
        const ShaderBufferInput input = \
        { \
            .Slot = (uint16_t)std::stoi(args[1]), \
            .BufferType = ShaderBufferType_SS##str, \
        }; \
        a_inputs->emplace_back(input); \
    } \
    
#define FSHADER_PUSHBUFFER_DEFINITION(str, structure) \
    if (args[0] == "P" #str) \
    { \
        FSHADER_PLATFORM_PUSHSTR(rStr, a_platform, structure, args[1]); \
        const ShaderBufferInput input = \
        { \
            .Slot = uint16_t(-1), \
            .BufferType = ShaderBufferType_P##str, \
        }; \
        a_inputs->emplace_back(input); \
    } \

#define FSHADER_UBO do { FSHADER_UBO_STRUCTURETABLE(FSHADER_UBO_DEFINITION) } while (0)
#define FSHADER_SSBO do { FSHADER_SSBO_STRUCTURETABLE(FSHADER_SSBO_DEFINITION) } while (0)
#define FSHADER_PUSHBUFFER do { FSHADER_PUSHBUFFER_STRUCTURETABLE(FSHADER_PUSHBUFFER_DEFINITION) } while (0)

namespace IcarianCore
{
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

    std::string GLSLFromFlareShader(const std::string_view& a_str, e_ShaderPlatform a_platform, std::vector<ShaderBufferInput>* a_inputs, std::string* a_error)
    {
        std::string shader = std::string(a_str);

        *a_error = std::string();

        std::size_t pos = 0;
        while (true) 
        {
            const std::size_t sPos = shader.find("#!", pos);
            if (sPos == std::string::npos)
            {
                break;
            }

            const std::size_t sAPos = shader.find('(', sPos + 1);
            const std::size_t eApos = shader.find(')', sPos + 1);

            if (sAPos == std::string::npos || eApos == std::string::npos)
            {
                *a_error = "Invalid Flare Shader definition: " + std::to_string(sPos);

                return std::string();
            }

            if (sAPos > eApos)
            {
                *a_error = "Invalid Flare Shader braces: " + std::to_string(sPos);

                return std::string();
            }

            const std::string defName = shader.substr(sPos + 2, sAPos - sPos - 2);
            // Could probably have a single array with a double null terminator to determine the end
            // Not an issue at the moment but could reduce allocations and jumping around in memory
            // Potential improvement if performance becomes an issue
            const std::vector<std::string> args = SplitArgs(shader.substr(sAPos + 1, eApos - sAPos - 1));

            std::string rStr;
            if (defName == "structure")
            {
                if (args.size() != 3)
                {
                    *a_error = "Flare Shader structure requires 3 arguments";

                    return std::string();
                }

                // I am lazy therefore let the pre processor write it
                FSHADER_UBO;
                FSHADER_SSBO;
            }
            else if (defName == "texture")
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
            }
            else if (defName == "pushtexture")
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
            }
            else if (defName == "shadowtexture")
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
            }
            else if (defName == "cubeshadowtexture")
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
            }
            else if (defName == "shadowtexturearray")
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
            }
            else if (defName == "userbuffer")
            {
                if (args.size() != 2)
                {
                    *a_error = "Flare Shader user buffer requires 2 arguments"; 

                    return std::string();
                }

                switch (a_platform) 
                {
                case ShaderPlatform_Vulkan:
                {
                    rStr = "layout(std140,binding=" + args[0] + ",set=" + args[0] + ") uniform " + args[1] + ";";

                    break;
                }
                case ShaderPlatform_OpenGL:
                {
                    rStr = "layout(std140,binding=" + args[0] + ") uniform " + args[1] + ";";

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
            }
            else if (defName == "instancestructure")
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
            }
            else if (defName == "pushbuffer")
            {
                if (args.size() != 2)
                {
                    *a_error = "Flare Shader push buffer requires 2 arguments";

                    return std::string();
                }

                FSHADER_PUSHBUFFER;
            }

            std::size_t next = 1;
            if (!rStr.empty())
            {
                next = rStr.size();
            }

            shader.replace(sPos, eApos - sPos + 1, rStr);

            pos = sPos + next;
        }

        return shader;
    }
}