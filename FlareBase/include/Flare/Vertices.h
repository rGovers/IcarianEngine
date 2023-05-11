#pragma once

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include <cstdint>

namespace FlareBase
{
    enum e_VertexType : uint16_t
    {
        VertexType_Null = UINT16_MAX,
        VertexType_Float = 0,
        VertexType_Int = 1,
        VertexType_UInt = 2
    };  

    struct VertexInputAttrib
    {
        uint16_t Location;
        e_VertexType Type;
        uint16_t Count;
        uint16_t Offset;    

        constexpr VertexInputAttrib(uint16_t a_location = -1, e_VertexType a_type = VertexType_Null, uint16_t a_count = 0, uint16_t a_offset = 0) :
            Location(a_location),
            Type(a_type),
            Count(a_count),
            Offset(a_offset)
        {   

        }   

        constexpr bool operator ==(const VertexInputAttrib& a_other) const
        {
            return Location == a_other.Location && Type == a_other.Type && Count == a_other.Count && Offset == a_other.Offset;
        }
        constexpr bool operator !=(const VertexInputAttrib& a_other) const
        {
            return !(*this == a_other);
        }
    };  

    struct Vertex
    {
        glm::vec4 Position;
        glm::vec3 Normal;
        glm::vec4 Color;
        glm::vec2 TexCoords;    

        constexpr Vertex(const glm::vec4& a_pos = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), const glm::vec3& a_normal = glm::vec3(0.0f), const glm::vec4& a_color = glm::vec4(1.0f), const glm::vec2& a_texCoords = glm::vec2(0.0f)) :
            Position(a_pos),
            Normal(a_normal),
            Color(a_color),
            TexCoords(a_texCoords)
        {   

        }
    };
}

