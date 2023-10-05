#pragma once

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <queue>
#include <vector>

#include "DataTypes/TArray.h"

struct TransformBuffer;

class ObjectManager
{
private:
    std::queue<uint32_t>    m_freeTransforms;
    TArray<TransformBuffer> m_transformBuffer;

    ObjectManager();
protected:

public:
    ~ObjectManager();

    static void Init();
    static void Destroy();

    static uint32_t CreateTransformBuffer();
    static TransformBuffer GetTransformBuffer(uint32_t a_addr);
    static void SetTransformBuffer(uint32_t a_addr, const TransformBuffer& a_buffer);
    static void DestroyTransformBuffer(uint32_t a_addr);

    static glm::mat4 GetGlobalMatrix(uint32_t a_addr);
};

struct TransformBuffer
{
    uint32_t  Parent;

    glm::vec3 Translation;
    glm::quat Rotation;
    glm::vec3 Scale;

    // Cannot seem to be able to use copy constructors because of C#
    // Gonna guess weird stuff with GC not sure as to exactly why however
    constexpr TransformBuffer(uint32_t a_parent = -1, const glm::vec3& a_translation = glm::vec3(0.0f), const glm::quat& a_quat = glm::identity<glm::quat>(), const glm::vec3& a_scale = glm::vec3(1.0f)) :
        Parent(a_parent),
        Translation(a_translation),
        Rotation(a_quat),
        Scale(a_scale)
    {

    }

    glm::mat4 ToMat4() const;
    glm::mat4 ToGlobalMat4() const;
};