#pragma once

#include "InteropTypes.h"

#ifdef CUBE_LANGUAGE_CSHARP
namespace IcarianEngine {
#endif

/// @file EngineTransformInteropStructures.h

/// @cond INTERNAL

IOP_PACKED IOP_CSINTERNAL struct TransformBuffer
{
    IOP_CSPUBLIC IOP_UINT32 ParentAddr;
    IOP_CSPUBLIC IOP_VEC3 Translation;
    IOP_CSPUBLIC IOP_QUAT Rotation;
    IOP_CSPUBLIC IOP_VEC3 Scale;

#ifdef CUBE_LANGUAGE_CPP
    constexpr TransformBuffer(uint32_t a_parent = -1, const glm::vec3& a_translation = glm::vec3(0.0f), const glm::quat& a_quat = glm::identity<glm::quat>(), const glm::vec3& a_scale = glm::vec3(1.0f)) :
        ParentAddr(a_parent),
        Translation(a_translation),
        Rotation(a_quat),
        Scale(a_scale)
    {

    }

    glm::mat4 ToMat4() const
    {
        constexpr glm::mat4 Iden = glm::identity<glm::mat4>();

        return glm::scale(glm::translate(Iden, Translation) * glm::toMat4(Rotation), Scale);
    }
#endif
};

/// @endcond

#ifdef CUBE_LANGUAGE_CSHARP
}
#endif