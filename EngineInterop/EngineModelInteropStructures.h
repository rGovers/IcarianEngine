#pragma once

#include "InteropTypes.h"

#ifdef CUBE_LANGUAGE_CPP
#define GLM_FORCE_SWIZZLE
#include <glm/glm.hpp>
#endif

#ifdef CUBE_LANGUAGE_CSHARP
namespace IcarianEngine.Rendering {
#endif

IOP_CSPUBLIC enum IOP_ENUM_NAME(VertexType) : IOP_UINT16
{
    IOP_ENUM_VALUE(VertexType, Null) = IOP_UINT16_MAX,
    IOP_ENUM_VALUE(VertexType, Float) = 0,
    IOP_ENUM_VALUE(VertexType, Int) = 1,
    IOP_ENUM_VALUE(VertexType, UInt) = 2
};

IOP_PACKED IOP_CSPUBLIC struct VertexInputAttribute
{
    /// <summary>
    /// The location of the attribute in the shader
    /// </summary>
    IOP_CSPUBLIC IOP_UINT16 Location;
    /// <summary>
    /// The type of the attribute
    /// </summary>
    IOP_CSPUBLIC IOP_ENUM_NAME(VertexType) Type;
    /// <summary>
    /// The number of elements in the attribute
    /// </summary>
    IOP_CSPUBLIC IOP_UINT16 Count;
    /// <summary>
    /// The offset of the attribute in the vertex
    /// </summary>
    IOP_CSPUBLIC IOP_UINT16 Offset;

#ifdef CUBE_LANGUAGE_CPP
    inline bool operator ==(const VertexInputAttribute& a_other) const
    {
        return Location == a_other.Location && Type == a_other.Type && Count == a_other.Count && Offset == a_other.Offset;
    }
    inline bool operator !=(const VertexInputAttribute& a_other) const
    {
        return !(*this == a_other);
    }
#endif
};

IOP_PACKED IOP_CSPUBLIC struct Vertex
{
    /// <summary>
    /// The position of the vertex
    /// </summary>
    IOP_CSPUBLIC IOP_VEC4 Position;
    /// <summary>
    /// The normal of the vertex
    /// </summary>
    IOP_CSPUBLIC IOP_VEC3 Normal;
    /// <summary>
    /// The color of the vertex
    /// </summary>
    IOP_CSPUBLIC IOP_VEC4 Color;
    /// <summary>
    /// The texture coordinates of the vertex
    /// </summary>
    IOP_CSPUBLIC IOP_VEC2 TexCoords;

#ifdef CUBE_LANGUAGE_CSHARP
    /// <summary>
    /// Gets the attributes of the vertex
    /// </summary>
    /// <returns>The attributes of the vertex</returns>
    public static VertexInputAttribute[] GetAttributes()
    {
        return new VertexInputAttribute[]
        {
            new VertexInputAttribute() { Location = 0, Type = VertexType.Float, Count = 4, Offset = (ushort)Marshal.OffsetOf<Vertex>("Position") },
            new VertexInputAttribute() { Location = 1, Type = VertexType.Float, Count = 3, Offset = (ushort)Marshal.OffsetOf<Vertex>("Normal") },
            new VertexInputAttribute() { Location = 2, Type = VertexType.Float, Count = 4, Offset = (ushort)Marshal.OffsetOf<Vertex>("Color") },
            new VertexInputAttribute() { Location = 3, Type = VertexType.Float, Count = 2, Offset = (ushort)Marshal.OffsetOf<Vertex>("TexCoords") }
        };
    
    }
#else
    constexpr Vertex(const glm::vec4& a_pos = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), const glm::vec3& a_normal = glm::vec3(0.0f), const glm::vec4& a_color = glm::vec4(1.0f), const glm::vec2& a_texCoords = glm::vec2(0.0f)) :
            Position(a_pos),
            Normal(a_normal),
            Color(a_color),
            TexCoords(a_texCoords)
        {   

        }
#endif
};

IOP_PACKED IOP_CSPUBLIC struct SkinnedVertex
{
    /// <summary>
    /// The position of the vertex
    /// </summary>
    IOP_CSPUBLIC IOP_VEC4 Position;
    /// <summary>
    /// The normal of the vertex
    /// </summary>
    IOP_CSPUBLIC IOP_VEC3 Normal;
    /// <summary>
    /// The color of the vertex
    /// </summary>
    IOP_CSPUBLIC IOP_VEC4 Color;
    /// <summary>
    /// The texture coordinates of the vertex
    /// </summary>
    IOP_CSPUBLIC IOP_VEC2 TexCoords;
    /// <summary>
    /// The bone weights of the vertex
    /// </summary>
    IOP_CSPUBLIC IOP_VEC4 BoneWeights;
#ifdef CUBE_LANGUAGE_CSHARP
    /// <summary>
    /// The first bone index of the vertex
    /// </summary>
    public int BoneIndexA;
    /// <summary>
    /// The second bone index of the vertex
    /// </summary>
    public int BoneIndexB;
    /// <summary>
    /// The third bone index of the vertex
    /// </summary>
    public int BoneIndexC;
    /// <summary>
    /// The fourth bone index of the vertex
    /// </summary>
    public int BoneIndexD;
#else
    glm::ivec4 BoneIndices;
#endif

#ifdef CUBE_LANGUAGE_CSHARP
    /// <summary>
    /// Gets the attributes of the vertex
    /// </summary>
    /// <returns>The attributes of the vertex</returns>
    public static VertexInputAttribute[] GetAttributes()
    {
        return new VertexInputAttribute[]
        {
            new VertexInputAttribute() { Location = 0, Type = VertexType.Float, Count = 4, Offset = (ushort)Marshal.OffsetOf<SkinnedVertex>("Position") },
            new VertexInputAttribute() { Location = 1, Type = VertexType.Float, Count = 3, Offset = (ushort)Marshal.OffsetOf<SkinnedVertex>("Normal") },
            new VertexInputAttribute() { Location = 2, Type = VertexType.Float, Count = 4, Offset = (ushort)Marshal.OffsetOf<SkinnedVertex>("Color") },
            new VertexInputAttribute() { Location = 3, Type = VertexType.Float, Count = 2, Offset = (ushort)Marshal.OffsetOf<SkinnedVertex>("TexCoords") },
            new VertexInputAttribute() { Location = 4, Type = VertexType.Float, Count = 4, Offset = (ushort)Marshal.OffsetOf<SkinnedVertex>("BoneWeights") },
            new VertexInputAttribute() { Location = 5, Type = VertexType.Int, Count = 4, Offset = (ushort)Marshal.OffsetOf<SkinnedVertex>("BoneIndices") }
        };
    }
#else 
    constexpr SkinnedVertex(const glm::vec4& a_pos = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), const glm::vec3& a_normal = glm::vec3(0.0f), const glm::vec4& a_color = glm::vec4(1.0f), const glm::vec2& a_texCoords = glm::vec2(0.0f), const glm::vec4& a_boneWeights = glm::vec4(0.0f), const glm::ivec4& a_boneIndices = glm::ivec4(0)) :
            Position(a_pos),
            Normal(a_normal),
            Color(a_color),
            TexCoords(a_texCoords),
            BoneWeights(a_boneWeights),
            BoneIndices(a_boneIndices)
        {   

        }
#endif
};

#ifdef CUBE_LANGUAGE_CSHARP
}
#endif