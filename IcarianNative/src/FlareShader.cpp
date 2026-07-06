// Icarian Engine - C# Game Engine
// 
// License at end of file.

#include "Rendering/FlareShader.h"

#include "Core/IcarianLambda.h"
#include "Core/ShaderBuffers.h"
#include "Core/StringUtils.h"
#include "DataTypes/Set.h"

#define GLSL_VULKAN_UNIFORM_STRING(slot, name, structure, structureName) COWU8String(SHADER_UNIFORM_STR(structure), a_tempAllocator) + "; layout(std140,binding=" + (slot) + ",set=" + (slot) + ") uniform " + (structureName) + "{ " + (structureName) + "Data " + (name) + "; };"
#define GLSL_VULKAN_SSBO_STRING(slot, name, structure, structureName) COWU8String(SHADER_UNIFORM_STR(structure), a_tempAllocator) + "; layout(std140,binding=" + (slot) + ",set=" + (slot) + ") readonly buffer " + (structureName) + " { int Count; " + (structureName) + "Data objects[]; } " + (name) + ";"
#define GLSL_VULKAN_PUSHBUFFER_STRING(name, structure) COWU8String("layout(push_constant) " SHADER_UNIFORM_STR(structure) " ", a_tempAllocator) + (name) + ";"

#define FSHADER_PLATFORM_UBOSTR(str, platform, argA, argB, structure, name) \
    switch (platform) \
    { \
    case ShaderPlatform_Vulkan: \
    case ShaderPlatform_VulkanCompute: \
    { \
        str = GLSL_VULKAN_UNIFORM_STRING(argA, argB, structure, name); \
        break; \
    } \
    default: \
    { \
        IERROR("Flare Shader invalid shader platform"); \
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
    default: \
    { \
        IERROR("Flare Shader invalid shader platform"); \
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
    default: \
    { \
        IERROR("Flare Shader invalid shader platform"); \
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

// TODO: Return an actual error rather than using IERROR
#define FSHADER_UBO_DEFINITION(str, structure) \
    case StringHash(#str): \
    { \
        uint16_t userSlot; \
        if (!args[1].ToUint16(&userSlot, 10)) \
        { \
            IERROR("Failed to parse user slot"); \
        } \
        const uint16_t index = ILAMBDA( \
        { \
            for (const ShaderBufferInput& input : a_builder.OtherInputs) \
            { \
                if (input.BufferType == ShaderBufferType_##str && input.UserSlot == userSlot) \
                { \
                    ILRETURN input.RealSlot; \
                } \
            }\
            ILRETURN nextSlot++; \
        }); \
        const COWU8String indexStr = COWU8String::FromValue(index, 10, a_tempAllocator); \
        FSHADER_PLATFORM_UBOSTR(rStr, a_builder.Platform, indexStr, args[2], structure, #str); \
        const ShaderBufferInput input = \
        { \
            .UserSlot = userSlot, \
            .RealSlot = index, \
            .BufferType = ShaderBufferType_##str, \
            .Count = 1, \
        }; \
        a_builder.Inputs->Push(input); \
        break; \
    } \

#define FSHADER_SSBO_DEFINITION(str, structure) \
    case StringHash("SS" #str): \
    { \
        uint16_t userSlot; \
        if (!args[1].ToUint16(&userSlot, 10)) \
        { \
            IERROR("Failed to parse user slot"); \
        } \
        const uint16_t index = ILAMBDA( \
        { \
            for (const ShaderBufferInput& input : a_builder.OtherInputs) \
            { \
                if (input.BufferType == ShaderBufferType_SS##str && input.UserSlot == userSlot) \
                { \
                    ILRETURN input.RealSlot; \
                } \
            }\
            ILRETURN nextSlot++; \
        }); \
        const COWU8String indexStr = COWU8String::FromValue(index, 10, a_tempAllocator); \
        FSHADER_PLATFORM_SSBOSTR(rStr, a_builder.Platform, indexStr, args[2], structure, #str); \
        const ShaderBufferInput input = \
        { \
            .UserSlot = userSlot, \
            .RealSlot = index, \
            .BufferType = ShaderBufferType_SS##str, \
            .Count = 1, \
        }; \
        a_builder.Inputs->Push(input); \
        break; \
    } \

#define FSHADER_PUSHBUFFER_DEFINITION(str, structure) \
    case StringHash("P" #str): \
    { \
        FSHADER_PLATFORM_PUSHSTR(rStr, a_builder.Platform, structure, args[1]); \
        const ShaderBufferInput input = \
        { \
            .UserSlot = uint16_t(-1), \
            .RealSlot = uint16_t(-1), \
            .BufferType = ShaderBufferType_P##str, \
            .Count = 1, \
        }; \
        a_builder.Inputs->Push(input); \
        break; \
    } \

#define FSHADER_UBO FSHADER_UBO_STRUCTURETABLE(FSHADER_UBO_DEFINITION)
#define FSHADER_SSBO FSHADER_SSBO_STRUCTURETABLE(FSHADER_SSBO_DEFINITION)
#define FSHADER_PUSHBUFFER FSHADER_PUSHBUFFER_STRUCTURETABLE(FSHADER_PUSHBUFFER_DEFINITION)

static COWU8String SpiltArgs(const char* a_str, Array<COWU8String>* a_args, uint32_t* a_offset, uint32_t* a_lines, Allocator* a_allocator)
{
    const char* iter = a_str;
    const char* prevIter = a_str;

    *a_offset = uint32_t(-1);
    if (a_lines != nullptr)
    {
        *a_lines = 0;
    }

    int32_t block = 0;
    int32_t scope = 0;

    while (true)
    {
        if (*iter == 0)
        {
            return COWU8String("Flare Shader unclosed argument", a_allocator);
        }

        switch (*iter) 
        {
        case '(':
        {
            if (scope++ == 0)
            {
                prevIter = iter + 1;
            }

            break;
        }
        case ')':
        {
            if (--scope == 0)
            {
                a_args->Push(COWU8String(prevIter, (uint32_t)(iter - prevIter), a_allocator));

                if (block != 0)
                {
                    return COWU8String("Flare Shader unclosed block in define argument", a_allocator);
                }

                *a_offset = iter - a_str + 1;

                return COWU8String(a_allocator);
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
            if (block == 0 && scope == 1)
            {
                a_args->Push(COWU8String(prevIter, (uint32_t)(iter - prevIter), a_allocator));

                prevIter = iter + 1;
            }

            break;
        }
        case '\n':
        {
            if (a_lines != nullptr)
            {
                (*a_lines)++;
            }

            break;
        }
        default:
        {
            break;
        }
        }

        ++iter;
    }

    IERROR("Unreachable path hit");

    return COWU8String("Unreachable path", a_allocator);
}

typedef uint32_t MeshOutCaseType;

#define MESHOUT_CASE(value, size) case StringHash<MeshOutCaseType>(#value): { ILRETURN FL_MESHOUT_ENUMVAL(value); }
#define MESHOUT_STR(value, size) #value,

constexpr const char* MeshOutString[] =
{
    FL_MESHOUT_TABLE(MESHOUT_STR)
};

struct MeshShaderOut
{
    COWU8String Identifier;
    uint32_t Slot;
    FlareShader::e_MeshOutType Type;
};

COWU8String FlareShader::GLSLFromFlareShader
(
    COWU8String* a_shader,
    const ShaderBuilder& a_builder,
    Allocator* a_allocator,
    Allocator* a_tempAllocator
)
{
    IVERIFY(a_shader != nullptr);

    COWU8String shader = COWU8String(a_builder.String, a_allocator);

    Set<COWU8String> imported = Set<COWU8String>(a_tempAllocator);
    Array<MeshShaderOut> meshOutputs = Array<MeshShaderOut>(a_tempAllocator);

    if (a_builder.Out != nullptr)
    {
        memset((void*)a_builder.Out, 0, sizeof(*a_builder.Out));
    }

    const uint16_t maxValue = ILAMBDA(
    {
        uint16_t val = -1;

        for (const ShaderBufferInput& input : a_builder.OtherInputs)
        {
            if (input.RealSlot == uint16_t(-1))
            {
                continue;
            }

            if (input.RealSlot > val || val == uint16_t(-1))
            {
                val = input.RealSlot;
            }
        }

        ILRETURN val;
    });

    uint16_t nextSlot;
    if (maxValue == uint16_t(-1))
    {
        nextSlot = 0;
    }
    else
    {
        nextSlot = maxValue + 1;
    }

    bool isMesh = false;
    MeshShaderData meshData;
    uint32_t prevLine = 0;
    uint32_t lineNumber = 1;

    const uint32_t versionIndex = shader.FindString("#version");
    if (versionIndex == uint32_t(-1))
    {
        return COWU8String("Flare Shader no GLSL #version", a_allocator);
    }

    const uint32_t versionNext = shader.FindCharacter('\n', versionIndex);
    if (versionNext == uint32_t(-1))
    {
        return COWU8String("Flare Shader no new line after #version", a_allocator);
    }

    uint32_t versionLine = uint32_t(-1);

    while (prevLine != uint32_t(-1))
    {
        uint32_t nextLine = shader.FindCharacter('\n', prevLine);
        if (nextLine == uint32_t(-1))
        {
            break;
        }

        if (nextLine == versionNext)
        {
            versionLine = lineNumber;

            ++lineNumber;
            prevLine = nextLine + 1;

            continue;
        }

        // Subtle bug with the current resolver as it works out in
        // Need to work from the inner defines out
        // Have not done as not super high priority as it only mangles line numbers
        // Only occurs for nested defines which are not common in the 1st place
        // TODO: Fix me
        while (true)
        {
            const uint32_t definePos = shader.FindString("#!", prevLine);
            if (definePos >= nextLine)
            {
                ++lineNumber;
                prevLine = nextLine + 1;

                break;
            }

            const uint32_t currentLine = lineNumber;

            const uint32_t openPos = shader.FindCharacter('(', definePos);

            const COWU8String defName = shader.Substring
            (
                definePos + 2,
                openPos,
                a_tempAllocator
            );

            if (versionLine == uint32_t(-1))
            {
                return "Flare Shader definition before GLSL version defined: " +
                    COWU8String(defName, a_allocator) + " line " +
                    COWU8String::FromValue(currentLine, 10, a_allocator);
            }

            uint32_t lines;
            uint32_t closePos;
            Array<COWU8String> args = Array<COWU8String>(a_tempAllocator);
            {
                uint32_t offset;

                const char* shaderCStr = shader.CStr();
                const COWU8String splitError = SpiltArgs
                (
                    shaderCStr + openPos,
                    &args,
                    &offset,
                    &lines,
                    a_tempAllocator
                );
                if (!splitError.Empty())
                {
                    return COWU8String(splitError, a_allocator) + ": line " +
                        COWU8String::FromValue(currentLine, 10, a_allocator);
                }

                closePos = openPos + offset;
                lineNumber += lines;
            }

            COWU8String rStr = COWU8String(a_tempAllocator);

            switch (defName.Hash())
            {
            case StringHash("workgroup"):
            {
                if (args.Size() != 3)
                {
                    return "Flare Shader #!workgroup requires 3 arguments: line " +
                        COWU8String::FromValue(currentLine, 10, a_allocator);
                }

                rStr = "layout(local_size_x=" + args[0] + ",local_size_y=" + args[1] + ",local_size_z=" + args[2] + ") in;\n"
                    "#line " + COWU8String::FromValue(lineNumber + lines, 10, a_tempAllocator) + "\n";

                if (a_builder.Out != nullptr)
                {
                    if (!args[0].ToUint32(&a_builder.Out->Workgroups.GroupX))
                    {
                        return "Flare Shader failed to parse workgroup X: line " +
                            COWU8String::FromValue(currentLine, 10, a_allocator) +
                            ", expected unsigned integer above 0";
                    }
                    if (!args[1].ToUint32(&a_builder.Out->Workgroups.GroupY))
                    {
                        return "Flare Shader failed to parse workgroup Y: line " +
                            COWU8String::FromValue(currentLine, 10, a_allocator) +
                            ", expected unsigned integer above 0";
                    }
                    if (!args[2].ToUint32(&a_builder.Out->Workgroups.GroupZ))
                    {
                        return "Flare Shader failed to parse workgroup Z: line " +
                            COWU8String::FromValue(currentLine, 10, a_allocator) +
                            ", expected unsigned integer above 0";
                    }
                }

                break;
            }
            case StringHash("meshprimitives"):
            {
                if (args.Size() != 3)
                {
                    return "Flare Shader #!meshprimitives requires 3 arguments: line " +
                        COWU8String::FromValue(currentLine, 10, a_allocator);
                }

                isMesh = true;

                const COWU8String argString = ILAMBDA(
                {
                    COWU8String val = args[0];
                    val.TrimWhitespace();
                    val.ToLower();

                    ILRETURN val;
                });

                const e_MeshShaderPrimitive primitiveType = ILAMBDA(
                {
                    const char* cStr = argString.CStr();
                    switch (StringHash<uint32_t>(cStr))
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
                    return "Flare Shader invalid mesh primitive type: " + args[0] + " line " +
                        COWU8String::FromValue(currentLine, 10, a_allocator) +
                        ", valid types are triangles, lines and points";
                }

                if (!args[1].ToUint32(&meshData.MaxVertices))
                {
                    return "Flare Shader invalid max vertices: " + args[1] + " line " +
                        COWU8String::FromValue(currentLine, 10, a_allocator) +
                        ", expected unsigned integer above 0";
                }
                if (!args[2].ToUint32(&meshData.MaxPrimitives))
                {
                    return "Flare Shader invalid max primitives: " + args[2] + " line " +
                        COWU8String::FromValue(currentLine, 10, a_allocator) +
                        ", expected unsigned integer above 0";
                }

                meshData.PrimitiveType = primitiveType;

                switch (a_builder.Platform)
                {
                case ShaderPlatform_Vulkan:
                {
                    rStr = "layout(" + argString + ", max_vertices=" + args[1] + ", max_primitives=" + args[2] + ") out; \n";

                    break;
                }
                case ShaderPlatform_VulkanCompute:
                {
                    break;
                }
                default:
                {
                    IERROR("Flare Shader invalid shader platform");

                    break;
                }
                }

                rStr += "#line " + COWU8String::FromValue(lineNumber, 10, a_tempAllocator) + "\n";

                break;
            }
            case StringHash("meshout"):
            {
                if (args.Size() != 3)
                {
                    return "Flare Shader #!meshout requires 3 arguments: line " +
                        COWU8String::FromValue(currentLine, 10, a_allocator);
                }

                isMesh = true;

                COWU8String argString = args[1];
                argString.TrimWhitespace();

                const e_MeshOutType outType = ILAMBDA(
                {
                    switch (StringHash<MeshOutCaseType>(argString.CStr()))
                    {
                    FL_MESHOUT_TABLE(MESHOUT_CASE);
                    }

                    ILRETURN MeshOutType_Last;
                });

                if (outType == MeshOutType_Last)
                {
                    COWU8String errStr = "Flare Shader invalid mesh output type: " + COWU8String(args[1], a_allocator) + " line " +
                        COWU8String::FromValue(currentLine, 10, a_allocator) +
                        ", valid types are ";

                    constexpr uint32_t TableLength = sizeof(MeshOutString) / sizeof(*MeshOutString);
                    for (uint32_t i = 0; i < TableLength; ++i)
                    {
                       errStr += MeshOutString[i];

                        if (i < TableLength - 1)
                        {
                            errStr += ", ";
                        }
                    }

                    return errStr;
                }

                uint16_t userSlot;
                if (!args[0].ToUint16(&userSlot))
                {
                    return "Flare Shader #!meshout failed to parse 1st argument, requires unsigned value greater than or equal to 0: line " +
                        COWU8String::FromValue(currentLine, 10, a_allocator);
                }

                COWU8String idenString = args[2];
                idenString.TrimWhitespace();

                const MeshShaderOut meshOut = 
                {
                    .Identifier = idenString,
                    .Slot = userSlot,
                    .Type = outType,
                };

                meshOutputs.Push(meshOut);

                break;
            }
            case StringHash("setmeshoutput"):
            {
                if (args.Size() != 2)
                {
                    return "Flare Shader #!setmeshoutput requires 2 arguments: line " +
                        COWU8String::FromValue(currentLine, 10, a_allocator);
                }

                if (meshData.PrimitiveType == MeshShaderPrimitive_Null)
                {
                    return "Flare Shader using #!setmeshoutput but PrimitiveType not set. "
                        "Use the #!meshprimitives define with the 1st argument as either triangles, lines or points: line " + 
                        COWU8String::FromValue(currentLine, 10, a_allocator);
                }

                if (meshData.MaxVertices <= 0)
                {
                    return "Flare Shader using #!setmeshoutput but Max Vertices not set. "
                        "Use the #!meshprimitives define with the 2nd argument as an unsigned value above 0: line " +
                        COWU8String::FromValue(currentLine, 10, a_allocator);
                }

                if (meshData.MaxPrimitives <= 0)
                {
                    return "Flare Shader using #!setmeshoutput but Max Primitives not set. "
                        "Use the #!meshprimitives define with the 3rd argument as an unsigned value above 0: line " +
                        COWU8String::FromValue(currentLine, 10, a_allocator);
                }

                isMesh = true;

                switch (a_builder.Platform)
                {
                case ShaderPlatform_Vulkan:
                {
                    rStr = "SetMeshOutputsEXT(" + args[0] + "," + args[1] + ")";

                    break;
                }
                case ShaderPlatform_VulkanCompute:
                {
                    const COWU8String multiString = ILAMBDA(
                    {
                        switch (meshData.PrimitiveType)
                        {
                        case MeshShaderPrimitive_Point:
                        {
                            ILRETURN COWU8String("* 1", a_tempAllocator);
                        }
                        case MeshShaderPrimitive_Line:
                        {
                            ILRETURN COWU8String("* 2", a_tempAllocator);
                        }
                        case MeshShaderPrimitive_Triangle:
                        {
                            ILRETURN COWU8String("* 3", a_tempAllocator);
                        }
                        default:
                        {
                            break;
                        }
                        }

                        ILRETURN COWU8String(a_tempAllocator);
                    });

                    if (multiString.Empty())
                    {
                        return "Flare Shader using #!setmeshoutput but PrimitiveType is an invalid value. "
                            "Use the #!meshprimitives define with the 1st argument as either triangles, lines or points: line " + 
                            COWU8String::FromValue(currentLine, 10, a_allocator);
                    }

                    const COWU8String indexCountStr = "(" + args[1] + ") " + multiString;

                    const COWU8String meshVertStr = COWU8String::FromValue(meshData.MaxVertices, 10, a_tempAllocator);
                    const COWU8String meshIndexStr = COWU8String::FromValue(meshData.MaxPrimitives, 10, a_tempAllocator) +
                        " " + multiString;

                    rStr = "fl_drawOut.drawCalls[gl_WorkGroupID.x].indexCount = uint(" + indexCountStr + ");"
                        "fl_drawOut.drawCalls[gl_WorkGroupID.x].instanceCount = 1;"
                        "fl_drawOut.drawCalls[gl_WorkGroupID.x].firstIndex = uint(gl_WorkGroupID.x * (" + meshIndexStr + "));"
                        "fl_drawOut.drawCalls[gl_WorkGroupID.x].vertexOffset = int(gl_WorkGroupID.x * (" + meshVertStr + "));"
                        "fl_drawOut.drawCalls[gl_WorkGroupID.x].firstInstance = 0";

                    break;
                }
                default:
                {
                    IERROR("Flare Shader invalid shader platform");

                    break;
                }
                }

                break;
            }
            case StringHash("meshpayload"):
            {
                if (args.Size() != 1)
                {
                    return "Flare Shader #!meshpayload requires 1 argument: line " +
                        COWU8String::FromValue(currentLine, 10, a_allocator);
                }

                if (meshData.MaxPrimitives <= 0)
                {
                    return "Flare Shader using #!meshpayload but Max Primitives not set. "
                        "Use the #!meshprimitives define with the 3rd argument as an unsigned value above 0: line " +
                        COWU8String::FromValue(currentLine, 10, a_allocator);
                }

                isMesh = true;

                switch (a_builder.Platform)
                {
                case ShaderPlatform_Vulkan:
                {
                    rStr = "fl_vertOut[" + args[0] + "]";

                    break;
                }
                case ShaderPlatform_VulkanCompute:
                {
                    const COWU8String meshVertStr = COWU8String::FromValue(meshData.MaxVertices, 10, a_tempAllocator);
                    rStr = "fl_vertOut.vertices[gl_WorkGroupID.x * " + meshVertStr + " + (" + args[0] + ")]";

                    break;
                }
                default:
                {
                    IERROR("Flare Shader invalid shader platform");

                    break;
                }
                }

                break;
            }
            case StringHash("meshposition"):
            {
                if (args.Size() != 1)
                {
                    return "Flare Shader #!meshposition require 1 argument: line " +
                        COWU8String::FromValue(currentLine, 10, a_allocator);
                }

                isMesh = true;

                switch (a_builder.Platform)
                {
                case ShaderPlatform_Vulkan:
                {
                    rStr = "gl_MeshVerticesEXT[" + args[0] + "].gl_Position";

                    break;
                }
                case ShaderPlatform_VulkanCompute:
                {
                    const COWU8String meshVertStr = COWU8String::FromValue(meshData.MaxVertices, 10, a_tempAllocator);
                    rStr = "fl_vertOut.vertices[gl_WorkGroupID.x * " + meshVertStr + " + (" + args[0] + ")].fl_position";

                    break;
                }
                default:
                {
                    IERROR("Flare Shader invalid shader platform");

                    break;
                }
                }

                break;
            }
            case StringHash("meshtri"):
            {
                if (args.Size() != 2)
                {
                    return "Flare Shader #!meshtri requires 2 arguments: line " +
                        COWU8String::FromValue(currentLine, 10, a_allocator);
                }

                if (meshData.PrimitiveType != MeshShaderPrimitive_Triangle)
                {
                    return "Flare Shader using #!meshtri but PrimitiveType is not set to triangles. "
                        "Use the #!meshprimitives define with the 1st argument as triangles: line " + 
                        COWU8String::FromValue(currentLine, 10, a_allocator);
                }

                if (meshData.MaxPrimitives <= 0)
                {
                    return "Flare Shader using #!meshtri but Max Primitives not set. "
                        "Use the #!meshprimitives define with the 3rd argument as an unsigned value above 0: line " +
                        COWU8String::FromValue(currentLine, 10, a_allocator);
                }

                isMesh = true;

                switch (a_builder.Platform)
                {
                case ShaderPlatform_Vulkan:
                {
                    rStr = "gl_PrimitiveTriangleIndicesEXT[" + args[0] + "] = " + args[1];

                    break;
                }
                case ShaderPlatform_VulkanCompute:
                {
                    const COWU8String maxPrimitivesStr = COWU8String::FromValue(meshData.MaxPrimitives, 10, a_tempAllocator);

                    const COWU8String offsetStr = COWU8String
                    (
                        "(gl_WorkGroupID.x * (" + maxPrimitivesStr + " * 3)) + ((" + args[0] + ") * 3)",
                        a_tempAllocator
                    );

                    rStr = "fl_indexOut.indices[(" + offsetStr + ") + 0] = (" + args[1] + ").x;"
                        "fl_indexOut.indices[(" + offsetStr + ") + 1] = (" + args[1] + ").y;"
                        "fl_indexOut.indices[(" + offsetStr + ") + 2] = (" + args[1] + ").z";

                    break;
                }
                default:
                {
                    IERROR("Flare Shader invalid shader platform");
                }
                }

                break;
            }
            case StringHash("meshvertex"):
            {
                if (args.Size() != 3)
                {
                    return "Flare Shader #!meshvertex requires 3 arguments: line" +
                        COWU8String::FromValue(currentLine, 10, a_allocator);
                }

                isMesh = true;

                uint16_t userSlot;
                if (!args[0].ToUint16(&userSlot))
                {
                    return "Flare Shader #!meshvertex failed to parse 1st argument, requires unsigned value greater than or equal to 0: line " +
                        COWU8String::FromValue(currentLine, 10, a_allocator);
                }

                const uint16_t currentSlot = nextSlot++;
                const COWU8String slotStr = COWU8String::FromValue(currentSlot, 10, a_tempAllocator);

                rStr = "struct Vertex " + args[2] + "; layout(std430,set=" + slotStr + ",binding=" + slotStr + ") readonly buffer MeshVertices { Vertex Vertices[]; } " + args[1] + ";";

                const ShaderBufferInput input = 
                {
                    .UserSlot = userSlot,
                    .RealSlot = currentSlot,
                    .BufferType = ShaderBufferType_MeshVertex,
                    .Count = 1,
                };

                a_builder.Inputs->Push(input);

                break;
            }
            case StringHash("meshlettriangledata"):
            {
                if (args.Size() != 2)
                {
                    return "Flare Shader #!meshlettriangledata requires 2 arguments: line " +
                        COWU8String::FromValue(currentLine, 10, a_allocator);
                }

                // GLSL does not support uint8 so need to do some bit fiddling to extract the true value
                // Yes I have noticed the extension but that requires querying device features or we can just do some bit fiddling
                rStr = "((" + args[0] + ".Indices[(" + args[1] + ") / 4] >> ((" + args[1] + ") % 4 * 8)) & 0xFF)";

                break;
            }
            case StringHash("meshletvertices"):
            {
                if (args.Size() != 2)
                {
                    return "Flare Shader #!meshletvertices requires 2 arguments: line " + 
                        COWU8String::FromValue(currentLine, 10, a_allocator);
                }

                uint16_t userSlot;
                if (!args[0].ToUint16(&userSlot))
                {
                    return "Flare Shader #!meshletvertices failed to parse 1st argument, requires unsigned value greater than or equal to 0: line " +
                        COWU8String::FromValue(currentLine, 10, a_allocator);
                }

                const uint16_t currentSlot = nextSlot++;
                const COWU8String slotStr = COWU8String::FromValue(currentSlot, 10, a_tempAllocator);

                rStr = "layout(std430,set=" + slotStr + ",binding=" + slotStr + ") readonly buffer MeshletVertices { uint Vertices[]; } " + args[1] + ";";

                const ShaderBufferInput input =
                {
                    .UserSlot = userSlot,
                    .RealSlot = currentSlot,
                    .BufferType = ShaderBufferType_MeshletVertices,
                    .Count = 1
                };

                a_builder.Inputs->Push(input);

                break;
            }
            case StringHash("meshlettriangles"):
            {
                if (args.Size() != 2)
                {
                    return "Flare Shader #!meshlettriangles requires 2 arguments: line " + 
                        COWU8String::FromValue(currentLine, 10, a_allocator);
                }

                uint16_t userSlot;
                if (!args[0].ToUint16(&userSlot))
                {
                    return "Flare Shader #!meshlettriangles failed to parse 1st argument, requires unsigned value greater than or equal to 0: line " +
                        COWU8String::FromValue(currentLine, 10, a_allocator);
                }

                const uint16_t currentSlot = nextSlot++;
                const COWU8String slotStr = COWU8String::FromValue(currentSlot, 10, a_tempAllocator);

                rStr = "layout(std430,set=" + slotStr + ",binding=" + slotStr + ") readonly buffer MeshletIndices { uint Indices[]; } " + args[1] + ";";

                const ShaderBufferInput input = 
                {
                    .UserSlot = userSlot,
                    .RealSlot = currentSlot,
                    .BufferType = ShaderBufferType_MeshletTriangles,
                    .Count = 1,
                };

                a_builder.Inputs->Push(input);

                break;
            }
            case StringHash("meshlet"):
            {
                if (args.Size() != 2)
                {
                    return "Flare Shader #!meshlet requires 2 arguments: line " +
                        COWU8String::FromValue(currentLine, 10, a_allocator);
                }

                uint16_t userSlot;
                if (!args[0].ToUint16(&userSlot))
                {
                    return "Flare Shader #!meshlet failed to parse 1st arguement, requires unsigned value greater than or equal to 0: line " +
                        COWU8String::FromValue(currentLine, 10, a_allocator);
                }

                const uint16_t currentSlot = nextSlot++;
                const COWU8String slotStr = COWU8String::FromValue(currentSlot, 10, a_tempAllocator);

                rStr = "struct MeshletData { uvec4 Data; vec4 Bounds; }; layout(std140,set=" + slotStr + ",binding=" + slotStr + ") readonly buffer Meshlets { MeshletData Meshlets[]; } " + args[1] + ";";

                const ShaderBufferInput input =
                {
                    .UserSlot = userSlot,
                    .RealSlot = currentSlot,
                    .BufferType = ShaderBufferType_Meshlet,
                    .Count = 1
                };

                a_builder.Inputs->Push(input);

                break;
            }
            case StringHash("structure"):
            {
                if (args.Size() != 3)
                {
                    return "Flare Shader #!structure requires 3 arguments: line " +
                        COWU8String::FromValue(currentLine, 10, a_allocator);
                }

                const char* str = args[0].CStr();

                switch (StringHash(str))
                {
                FSHADER_UBO
                FSHADER_SSBO
                }

                break;
            }
            case StringHash("buffertexture"):
            {
                if (args.Size() != 4)
                {
                    return "Flare Shader #!buffertexture requries 4 arguements: line " +
                        COWU8String::FromValue(currentLine, 10, a_allocator);
                }

                if (a_builder.Platform != ShaderPlatform_VulkanCompute)
                {
                    return "Flare Shader #!buffertexture not available on non compute platform: line " + 
                        COWU8String::FromValue(currentLine, 10, a_allocator);
                }

                uint16_t userSlot;
                if (!args[1].ToUint16(&userSlot))
                {
                    return "Flare Shader #!buffertexture failed to parse 2nd argument, requires an unsigned value greater than or equal to 0: line " +
                        COWU8String::FromValue(currentLine, 10, a_allocator);
                }

                uint16_t count;
                if (!args[2].ToUint16(&count))
                {
                    return "Flare Shader #!buffertexture failed to parse 3rd argument, requires an unsigned value greater than 0: line " +
                        COWU8String::FromValue(currentLine, 10, a_allocator);
                }

                const uint16_t currentSlot = ILAMBDA(
                {
                    for (const ShaderBufferInput& input : a_builder.OtherInputs)
                    {
                        if (input.BufferType == ShaderBufferType_BufferTexture && input.UserSlot == userSlot)
                        {
                            ILRETURN input.RealSlot;
                        }
                    }

                    ILRETURN nextSlot++;
                });
                const COWU8String slotStr = COWU8String::FromValue(currentSlot, 10, a_tempAllocator);

                rStr = "layout(" + args[0] + ",set=" + slotStr + ",binding=" + slotStr + ") uniform image2D " + args[3] + ";";

                const ShaderBufferInput input =
                {
                    .UserSlot = userSlot,
                    .RealSlot = currentSlot,
                    .BufferType = ShaderBufferType_BufferTexture,
                    .Count = count,
                };

                a_builder.Inputs->Push(input);

                break;
            }
            case StringHash("texture"):
            {
                if (args.Size() != 2)
                {
                    return "Flare Shader #!texture requires 2 arguments: line " +
                        COWU8String::FromValue(currentLine, 10, a_allocator);
                }

                uint16_t userSlot;
                if (!args[0].ToUint16(&userSlot))
                {
                    return "Flare Shader #!texture failed to parse 1st argument, requires an unsigned value greater than or equal to 0: line " +
                        COWU8String::FromValue(currentLine, 10, a_allocator);
                }

                const uint16_t currentSlot = ILAMBDA(
                {
                    for (const ShaderBufferInput& input : a_builder.OtherInputs)
                    {
                        if (input.BufferType == ShaderBufferType_Texture && input.UserSlot == userSlot)
                        {
                            ILRETURN input.RealSlot;
                        }
                    }

                    ILRETURN nextSlot++;
                });
                const COWU8String slotStr = COWU8String::FromValue(currentSlot, 10, a_tempAllocator);

                rStr = "layout(set=" + slotStr + ",binding=" + slotStr + ") uniform sampler2D " + args[1] + ";";

                const ShaderBufferInput input =
                {
                    .UserSlot = userSlot,
                    .RealSlot = currentSlot,
                    .BufferType = ShaderBufferType_Texture,
                    .Count = 1
                };

                a_builder.Inputs->Push(input);

                break;
            }
            case StringHash("pushtexture"):
            {
                if (args.Size() != 2)
                {
                    return "Flare Shader #!pushtexture requires 2 arguments: line " +
                        COWU8String::FromValue(currentLine, 10, a_allocator);
                }

                uint16_t userSlot;
                if (!args[0].ToUint16(&userSlot))
                {
                    return "Flare Shader #!pushtexture failed to parse 1st argument, requires an unsigned value greater than or equal to 0: line " +
                        COWU8String::FromValue(currentLine, 10, a_allocator);
                }

                const uint16_t currentSlot = ILAMBDA(
                {
                    for (const ShaderBufferInput& input : a_builder.OtherInputs)
                    {
                        if (input.BufferType == ShaderBufferType_PushTexture && input.UserSlot == userSlot)
                        {
                            ILRETURN input.RealSlot;
                        }
                    }

                    ILRETURN nextSlot++;
                });
                const COWU8String slotStr = COWU8String::FromValue(currentSlot, 10, a_tempAllocator);

                rStr = "layout(set=" + slotStr + ",binding=" + slotStr + ") uniform sampler2D " + args[1] + ";";

                const ShaderBufferInput input =
                {
                    .UserSlot = userSlot,
                    .RealSlot = currentSlot,
                    .BufferType = ShaderBufferType_PushTexture,
                    .Count = 1
                };

                a_builder.Inputs->Push(input);

                break;
            }
            case StringHash("shadowtexture"):
            {
                if (args.Size() != 2)
                {
                    return "Flare Shader #!shadowtexture requires 2 arguments: line " + 
                        COWU8String::FromValue(currentLine, 10, a_allocator);
                }


                uint16_t userSlot;
                if (!args[0].ToUint16(&userSlot))
                {
                    return "Flare Shader #!shadowtexture failed to parse 1st argument, requires an unsigned value greater than or equal to 0: line " +
                        COWU8String::FromValue(currentLine, 10, a_allocator);
                }

                const uint16_t currentSlot = ILAMBDA(
                {
                    for (const ShaderBufferInput& input : a_builder.OtherInputs)
                    {
                        if (input.BufferType == ShaderBufferType_ShadowTexture2D && input.UserSlot == userSlot)
                        {
                            ILRETURN input.RealSlot;
                        }
                    }

                    ILRETURN nextSlot++;
                });
                const COWU8String slotStr = COWU8String::FromValue(currentSlot, 10, a_tempAllocator);

                rStr = "layout(set=" + slotStr + ",binding=" + slotStr + ") uniform sampler2D " + args[1] + ";";

                const ShaderBufferInput input =
                {
                    .UserSlot = userSlot,
                    .RealSlot = currentSlot,
                    .BufferType = ShaderBufferType_ShadowTexture2D,
                    .Count = 1,
                };

                a_builder.Inputs->Push(input);

                break;
            }
            case StringHash("cubeshadowtexture"):
            {
                if (args.Size() != 2)
                {
                    return "Flare Shader #!cubeshadowtexture requires 2 arguments: line " +
                        COWU8String::FromValue(currentLine, 10, a_allocator);
                }

                uint16_t userSlot;
                if (!args[0].ToUint16(&userSlot))
                {
                    return "Flare Shader #!cubeshadowtexture failed to parse 1st argument, requires an unsigned value greater than or equal to 0: line " +
                        COWU8String::FromValue(currentLine, 10, a_allocator);
                }

                const uint16_t currentSlot = ILAMBDA(
                {
                    for (const ShaderBufferInput& input : a_builder.OtherInputs)
                    {
                        if (input.BufferType == ShaderBufferType_ShadowTextureCube && input.UserSlot == userSlot)
                        {
                            ILRETURN input.RealSlot;
                        }
                    }

                    ILRETURN nextSlot++;
                });
                const COWU8String slotStr = COWU8String::FromValue(currentSlot, 10, a_tempAllocator);

                rStr = "layout(set=" + slotStr + ",binding=" + slotStr + ") uniform samplerCube " + args[1] + ";";

                const ShaderBufferInput input =
                {
                    .UserSlot = userSlot,
                    .RealSlot = currentSlot,
                    .BufferType = ShaderBufferType_ShadowTextureCube,
                    .Count = 1,
                };

                a_builder.Inputs->Push(input);

                break;
            }
            case StringHash("shadowtexturearray"):
            {
                if (args.Size() != 3)
                {
                    return "Flare Shader #!shadowtexturearray requires 3 arguments: line " +
                        COWU8String::FromValue(currentLine, 10, a_allocator);
                }

                uint16_t userSlot;
                if (!args[0].ToUint16(&userSlot))
                {
                    return "Flare Shader #!shadowtexturearray failed to parse 1st argument, requires an unsigned value greater than or equal to 0: line " +
                        COWU8String::FromValue(currentLine, 10, a_allocator);
                }

                uint16_t count;
                if (!args[1].ToUint16(&count))
                {
                    return "Flare Shader #!shadowtexturearray failed to parse 2nd argument, requires an unsigned value greater than 0: line " + 
                        COWU8String::FromValue(currentLine, 10, a_allocator);
                }

                const uint16_t currentSlot = ILAMBDA(
                {
                    for (const ShaderBufferInput& input : a_builder.OtherInputs)
                    {
                        if (input.BufferType == ShaderBufferType_AShadowTexture2D && input.UserSlot == userSlot)
                        {
                            ILRETURN input.RealSlot;
                        }
                    }

                    ILRETURN nextSlot++;
                });
                const COWU8String slotStr = COWU8String::FromValue(currentSlot, 10, a_tempAllocator);

                rStr = "layout(set=" + slotStr + ",binding=" + slotStr + ") uniform sampler2D " + args[2] + "[" + args[1] + "];";

                const ShaderBufferInput input =
                {
                    .UserSlot = userSlot,
                    .RealSlot = currentSlot,
                    .BufferType = ShaderBufferType_AShadowTexture2D,
                    .Count = count
                };

                a_builder.Inputs->Push(input);

                break;
            }
            case StringHash("userbuffer"):
            {
                if (args.Size() != 3)
                {
                    return "Flare Shader #!userbuffer requires 3 arguments: line " +
                        COWU8String::FromValue(currentLine, 10, a_allocator);
                }

                uint16_t userSlot;
                if (!args[0].ToUint16(&userSlot))
                {
                    return "Flare Shader #!userbuffer failed to parse 1st argument, requires an unsigned value greater than or equal to 0: line " +
                        COWU8String::FromValue(currentLine, 10, a_allocator);
                }

                const uint16_t currentSlot = ILAMBDA(
                {
                    for (const ShaderBufferInput& input : a_builder.OtherInputs)
                    {
                        if (input.BufferType == ShaderBufferType_UserUBO && input.UserSlot == userSlot)
                        {
                            ILRETURN input.RealSlot;
                        }
                    }

                    ILRETURN nextSlot++;
                });
                const COWU8String slotStr = COWU8String::FromValue(currentSlot, 10, a_tempAllocator);

                rStr = "layout(std140,binding=" + slotStr + ",set=" + slotStr + ") uniform UserBuffer " + args[1] + " " + args[2] + ";";

                const ShaderBufferInput input =
                {
                    .UserSlot = userSlot,
                    .RealSlot = currentSlot,
                    .BufferType = ShaderBufferType_UserUBO,
                    .Count = 1
                };

                a_builder.Inputs->Push(input);

                break;
            }
            case StringHash("userarray"):
            {
                if (args.Size() != 3)
                {
                    return "Flare Shader #!userarray requires 3 arguments: line " +
                        COWU8String::FromValue(currentLine, 10, a_allocator);
                }

                uint16_t userSlot;
                if (!args[0].ToUint16(&userSlot))
                {
                    return "Flare Shader #!userarray failed to parse 1st argument, requires an unsigned value greater than or equal to 0: line " +
                        COWU8String::FromValue(currentLine, 10, a_allocator);
                }

                const uint16_t currentSlot = ILAMBDA(
                {
                    for (const ShaderBufferInput& input : a_builder.OtherInputs)
                    {
                        if (input.BufferType == ShaderBufferType_UserArray && input.UserSlot == userSlot)
                        {
                            ILRETURN input.RealSlot;
                        }
                    }

                    ILRETURN nextSlot++;
                });
                const COWU8String slotStr = COWU8String::FromValue(currentSlot, 10, a_tempAllocator);

                rStr = "struct UserArrayData " + args[1] + ";"
                    " layout(std140,binding=" + slotStr + ",set=" + slotStr + ") readonly buffer UserArray"
                    " { int Count; UserArrayData objects[]; } " + args[2] + ";";

                const ShaderBufferInput input =
                {
                    .UserSlot = userSlot,
                    .RealSlot = currentSlot,
                    .BufferType = ShaderBufferType_UserArray,
                    .Count = 1
                };

                a_builder.Inputs->Push(input);

                break;
            }
            case StringHash("pushbuffer"):
            {
                if (args.Size() != 2)
                {
                    return "Flare Shader #!pushbuffer requires 2 arguments: line " +
                        COWU8String::FromValue(currentLine, 10, a_allocator);
                }

                const char* str = args[0].CStr();

                switch (StringHash(str))
                {
                FSHADER_PUSHBUFFER
                }

                break;
            }
            case StringHash("instancestructure"):
            {
                if (args.Size() != 1)
                {
                    return "Flare Shader #!instancetructure requires 1 argument: line " +
                        COWU8String::FromValue(currentLine, 10, a_allocator);
                }

                switch (a_builder.Platform) 
                {
                case ShaderPlatform_VulkanCompute:
                {
                    return "Flare Shader #!instancestructure used in Compute mode: line " +
                        COWU8String::FromValue(currentLine, 10, a_allocator);
                }
                case ShaderPlatform_Vulkan:
                {
                    rStr = args[0] + ".objects[gl_InstanceIndex]";

                    break;
                }
                default:
                {
                    IERROR("Flare Shader invalid shader platform");

                    break;
                }
                }

                break;
            }
            case StringHash("preloop"):
            {
                if (args.Size() != 4)
                {
                    return "Flare Shader #!preloop requires 4 arguments: line " +
                        COWU8String::FromValue(currentLine, 10, a_allocator);
                }

                int32_t startIndex;
                if (!args[1].ToInt32(&startIndex, 10))
                {
                    return "Flare Shader #!preloop failed to parse the 1st argument, requires an integer value: line " +
                        COWU8String::FromValue(currentLine, 10, a_allocator);
                }

                int32_t endIndex;
                if (!args[2].ToInt32(&endIndex))
                {
                    return "Flare Shader #!preloop failed to parse the 2nd argument, requires an integer value: line " +
                        COWU8String::FromValue(currentLine, 10, a_allocator);
                }

                const uint32_t strSize = args[0].Length();

                rStr = COWU8String("switch(0) { default: {", a_tempAllocator);

                for (int32_t i = startIndex; i < endIndex; ++i)
                {
                    const COWU8String valStr = COWU8String::FromValue(i, 10, a_tempAllocator);

                    // We do it this way so that if the user has no instance the string does not get copied
                    COWU8String snippet = args[3];

                    while (true)
                    {
                        // TODO: Need to see use cases to see which is faster as to if we pre scan and store or scan every time
                        const uint32_t index = snippet.FindString(args[0]);
                        if (index == uint32_t(-1))
                        {
                            break;
                        }

                        snippet.Replace(index, index + strSize, valStr);
                    }

                    rStr += snippet;
                }

                rStr += "}} \n";

                // TODO: Need to count new lines at the end of the argument list aswell
                rStr += "#line " + COWU8String::FromValue(currentLine, 10, a_tempAllocator);

                break;
            }
            case StringHash("import"):
            {
                if (args.Size() != 1)
                {
                    return "Flare Shader #!import require 1 argument: line " +
                        COWU8String::FromValue(currentLine, 10, a_allocator);
                }

                const COWU8String& key = args[0];

                if (imported.Exists(key))
                {
                    break;
                }

                if (!a_builder.Imports.Exists(key))
                {
                    return "Flare Shader no import found: line " +
                        COWU8String::FromValue(currentLine, 10, a_allocator);
                }

                rStr = a_builder.Imports[key];
                imported.Push(key);

                break;
            }
            default:
            {
                return "Flare Shader invalid define: " + defName + ": line " +
                    COWU8String::FromValue(currentLine, 10, a_allocator);
            }
            }

            shader.Replace(definePos, closePos, rStr);

            // nextLine = shader.FindCharacter('\n', prevLine);

            // ++lineNumber;
            // prevLine = nextLine + 1;
        }
    }

    if (isMesh)
    {
        if (meshData.MaxVertices <= 0)
        {
            return COWU8String
            (
                "Flare Shader mesh shader but Max Vertices not set. "
                "Use the #!meshprimitives define with the 2nd argument as an unsigned value above 0",
                a_allocator
            );
        }

        if (meshData.MaxPrimitives <= 0)
        {
            return COWU8String
            (
                "Flare Shader mesh shader but Max Primitives not set. "
                "Use the #!meshprimitives define with the 3rd argument as an unsigned value above 0",
                a_allocator
            );
        }

        switch (a_builder.Platform)
        {
        case ShaderPlatform_Vulkan:
        {
            COWU8String s = COWU8String("layout(location=0) out PerVertexData {", a_tempAllocator);

            for (const MeshShaderOut& out : meshOutputs)
            {
                s += COWU8String(MeshOutString[out.Type], a_tempAllocator) + " " + out.Identifier + ";";
            }

            s += "}"
                " fl_vertOut[];\n";

            s += "#extension GL_EXT_mesh_shader : require\n";

            s += "#line " + COWU8String::FromValue(versionLine + 1, 10, a_tempAllocator) + "\n";

            shader.Insert(versionNext + 1, s);

            break;
        }
        case ShaderPlatform_VulkanCompute:
        {
            {
                const uint16_t drawOutBuffer = nextSlot++;

                const COWU8String drawOutBufferStr = COWU8String::FromValue(drawOutBuffer, 10, a_tempAllocator);

                // Refer to VkDrawIndexedIndirectCommand
                const COWU8String s = "struct fl_MeshDrawOut"
                    "{"
                    "   uint indexCount;"
                    "   uint instanceCount;"
                    "   uint firstIndex;"
                    "   int vertexOffset;"
                    "   uint firstInstance;"
                    "};"

                    "layout(std430,binding=" + drawOutBufferStr + ",set=" + drawOutBufferStr + ") buffer fl_MeshShaderDrawBuffer"
                    "{"
                        "fl_MeshDrawOut drawCalls[];"
                    "} fl_drawOut;\n"

                    "#line " + COWU8String::FromValue(versionLine + 1, 10 , a_tempAllocator) + "\n";

                    shader.Insert(versionNext + 1, s);

                    const ShaderBufferInput input =
                    {
                        .UserSlot = uint16_t(-1),
                        .RealSlot = drawOutBuffer,
                        .BufferType = ShaderBufferType_OutMeshEmulationDraw,
                        .Count = 1,
                    };

                    a_builder.Inputs->Push(input);
            }

            {
                const uint16_t vertexOutBuffer = nextSlot++;

                const COWU8String vertexOutBufferStr = COWU8String::FromValue(vertexOutBuffer, 10, a_tempAllocator);

                COWU8String s = COWU8String
                (
                    "struct fl_MeshVertexOut"
                    "{"
                    "   vec4 fl_position;",
                    a_tempAllocator
                );

                for (const MeshShaderOut& out : meshOutputs)
                {
                    s += COWU8String(MeshOutString[out.Type], a_tempAllocator) + " " + out.Identifier + ";";
                }

                s += "};"

                // So they force std140 on us even when we specify std430
                // GLSL is extra annoying as they is not 1 byte types without enabling extension nor pointer types
                // That means for tight packing either have to do bit fiddling with a uint array or take multiple inputs
                // Not going to do any of that at this point and just eat the 16 byte alignment
                " layout(std140,binding=" + vertexOutBufferStr + ",set=" + vertexOutBufferStr + ") buffer fl_MeshShaderVertexBuffer"
                "{"
                "   fl_MeshVertexOut vertices[];"
                "} fl_vertOut;\n"

                "#line " + COWU8String::FromValue(versionLine + 1, 10, a_tempAllocator) + "\n";

                shader.Insert(versionNext + 1, s);

                const ShaderBufferInput input =
                {
                    // Do not want it user facing if it is could break shit so "null" it
                    .UserSlot = uint16_t(-1),
                    .RealSlot = vertexOutBuffer,
                    .BufferType = ShaderBufferType_OutMeshEmulationVertex,
                    .Count = 1
                };

                a_builder.Inputs->Push(input);
            }

            {
                const uint16_t indexOutBuffer = nextSlot++;

                const COWU8String indexOutBufferStr = COWU8String::FromValue(indexOutBuffer, 10, a_tempAllocator);

                const COWU8String s = "layout(std430,binding=" + indexOutBufferStr + ",set=" + indexOutBufferStr + ") buffer fl_MeshShaderIndexBuffer"
                    "{"
                    "   uint indices[];"
                    "} fl_indexOut;\n"
                    "#line " + COWU8String::FromValue(versionLine + 1, 10, a_tempAllocator) + "\n";

                shader.Insert(versionNext + 1, s);

                const ShaderBufferInput input =
                {
                    .UserSlot = uint16_t(-1),
                    .RealSlot = indexOutBuffer,
                    .BufferType = ShaderBufferType_OutMeshEmulationIndex,
                    .Count = 1
                };

                a_builder.Inputs->Push(input);
            }

            break;
        }
        default:
        {
            IERROR("Invalid shader platform");

            break;
        }
        }
    }

    if (a_builder.Out != nullptr)
    {
        a_builder.Out->MeshData = meshData;
    }

    *a_shader = shader;

    return COWU8String(a_allocator);
}

COWU8String FlareShader::GenerateMeshVertexStub(const COWU8String& a_shader, Array<MeshShaderOut>* a_outputs, Allocator* a_allocator, Allocator* a_tempAllocator)
{
    // TODO: Can probably change this function to just generate the SPIR-V directly down the line
    // Is simple enough that should not need to do much and seems like a good place to start with SPIR-V
    uint32_t lastIndex = 0;

    IVERIFY(a_outputs != nullptr);

    while (true)
    {
        const uint32_t definePos = a_shader.FindString("#!", lastIndex);
        if (definePos == uint32_t(-1))
        {
            break;
        }
        IDEFER(lastIndex = definePos + 1);

        const uint32_t openPos = a_shader.FindCharacter('(', definePos);

        const COWU8String defName = a_shader.Substring
        (
            definePos + 2,
            openPos,
            a_tempAllocator
        );

        if (defName == "meshout")
        {
            Array<COWU8String> args = Array<COWU8String>(a_tempAllocator);
            {
                uint32_t offset;

                const char* shaderCStr = a_shader.CStr();
                const COWU8String splitError = SpiltArgs
                (
                    shaderCStr + openPos,
                    &args,
                    &offset,
                    nullptr,
                    a_tempAllocator
                );
                if (!splitError.Empty())
                {
                    return COWU8String(a_allocator);
                }
            }

            if (args.Size() != 3)
            {
                return COWU8String(a_allocator);
            }

            COWU8String argString = args[1];
            argString.TrimWhitespace();

            const e_MeshOutType outType = ILAMBDA(
            {
                switch (StringHash<MeshOutCaseType>(argString.CStr()))
                {
                FL_MESHOUT_TABLE(MESHOUT_CASE);
                }

                ILRETURN MeshOutType_Last;
            });

            if (outType == MeshOutType_Last)
            {
                return COWU8String(a_allocator);
            }

            uint16_t userSlot;
            if (!args[0].ToUint16(&userSlot))
            {
                return COWU8String(a_allocator);
            }

            COWU8String idenString = args[2];
            idenString.TrimWhitespace();

            const MeshShaderOut meshOut = 
            {
                .Identifier = idenString,
                .Slot = userSlot,
                .Type = outType,
            };

            a_outputs->Push(meshOut);
        }
    }

    const uint32_t count = a_outputs->Size();
    if (count <= 0)
    {
        return COWU8String(a_allocator);
    }

    COWU8String inputs = COWU8String(a_allocator);
    COWU8String outputs = COWU8String(a_allocator);
    COWU8String body = COWU8String(a_allocator);

    for (uint32_t i = 0; i < count; ++i)
    {
        const MeshShaderOut& out = (*a_outputs)[i];
        if (out.Type >= MeshOutType_Last)
        {
            return COWU8String(a_allocator);
        }

        const char* typeStr = MeshOutString[out.Type];

        const COWU8String inStr = COWU8String("fl_", a_tempAllocator) + out.Identifier;

        const COWU8String inputSlot = COWU8String::FromValue(i + 1, 10, a_tempAllocator);
        const COWU8String outputSlot = COWU8String::FromValue(out.Slot, 10, a_tempAllocator);

        inputs += "layout(location=" + inputSlot + ") in " + typeStr + " " + inStr + ";\n";
        outputs += "layout(location=" + outputSlot + ") out " + typeStr + " " + out.Identifier + ";\n";

        body += "    " + out.Identifier + "=" + inStr + ";\n";
    }

    return COWU8String("#version 450 \n"

    "layout(location=0) in vec4 flp_position;\n", a_allocator) +
    inputs +

    outputs +

    "void main()\n"
    "{\n"
    "   gl_Position = flp_position;\n" +

    body +

    "}\n";
}

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
