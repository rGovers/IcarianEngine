#pragma once

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <queue>
#include <vector>

#include "DataTypes/TArray.h"

#include "EngineTransformInteropStructures.h"

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

    static uint32_t* BatchCreateTransformBuffer(uint32_t a_count);
    static uint32_t CreateTransformBuffer();
    static TransformBuffer GetTransformBuffer(uint32_t a_addr);
    static void SetTransformBuffer(uint32_t a_addr, const TransformBuffer& a_buffer);
    static void DestroyTransformBuffer(uint32_t a_addr);

    static glm::mat4 GetMatrix(uint32_t a_addr);
    static glm::mat4 GetGlobalMatrix(uint32_t a_addr);
};
