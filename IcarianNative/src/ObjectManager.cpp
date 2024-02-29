#include "ObjectManager.h"

#include <glm/gtx/matrix_decompose.hpp>

#include "Flare/IcarianAssert.h"
#include "Flare/IcarianDefer.h"
#include "Runtime/RuntimeManager.h"
#include "Trace.h"

#include "EngineTransformInterop.h"

static ObjectManager* Instance = nullptr;

ENGINE_TRANSFORM_EXPORT_TABLE(RUNTIME_FUNCTION_DEFINITION);

ObjectManager::ObjectManager()
{
    TRACE("Binding Object functions to C#");
    ENGINE_TRANSFORM_EXPORT_TABLE(RUNTIME_FUNCTION_ATTACH);
}
ObjectManager::~ObjectManager()
{

}

void ObjectManager::Init()
{
    if (Instance == nullptr)
    {
        Instance = new ObjectManager();
    }
}
void ObjectManager::Destroy()
{
    if (Instance != nullptr)
    {
        delete Instance;
        Instance = nullptr;
    }
}

uint32_t* ObjectManager::BatchCreateTransformBuffer(uint32_t a_count)
{
    constexpr TransformBuffer Buffer;

    uint32_t* addrs = new uint32_t[a_count];

    const uint32_t freeCount = Instance->m_freeTransforms.size();
    if (a_count > freeCount)
    {
        TLockArray<TransformBuffer> a = Instance->m_transformBuffer.ToLockArray();
        for (uint32_t i = 0; i < freeCount; ++i)
        {
            const uint32_t addr = Instance->m_freeTransforms.front();
            Instance->m_freeTransforms.pop();

            a[addr] = Buffer;

            addrs[i] = addr;
        }

        const uint32_t pushCount = a_count - freeCount;
        const uint32_t size = a.Size();

        Instance->m_transformBuffer.UPushVals(Buffer, pushCount);

        for (uint32_t i = 0; i < pushCount; ++i)
        {
            addrs[freeCount + i] = size + i;
        }
    }
    else 
    {
        TLockArray<TransformBuffer> a = Instance->m_transformBuffer.ToLockArray();
        for (uint32_t i = 0; i < a_count; ++i)
        {
            const uint32_t addr = Instance->m_freeTransforms.front();
            Instance->m_freeTransforms.pop();

            a[addr] = Buffer;

            addrs[i] = addr;
        }
    }

    return addrs;
}
uint32_t ObjectManager::CreateTransformBuffer()
{
    constexpr TransformBuffer Buffer;

    TLockArray<TransformBuffer> a = Instance->m_transformBuffer.ToLockArray();
    if (!Instance->m_freeTransforms.empty())
    {
        const uint32_t addr = Instance->m_freeTransforms.front();
        Instance->m_freeTransforms.pop();

        a[addr] = Buffer;

        return addr;
    }

    return Instance->m_transformBuffer.UPushVal(Buffer);
}
TransformBuffer ObjectManager::GetTransformBuffer(uint32_t a_addr)
{
    ICARIAN_ASSERT_MSG(a_addr < Instance->m_transformBuffer.Size(), "GetTransformBuffer out of bounds");

    return Instance->m_transformBuffer[a_addr];
}
void ObjectManager::SetTransformBuffer(uint32_t a_addr, const TransformBuffer& a_buffer)
{
    ICARIAN_ASSERT_MSG(a_addr < Instance->m_transformBuffer.Size(), "SetTransformBuffer out of bounds");

    Instance->m_transformBuffer.LockSet(a_addr, a_buffer);
}
void ObjectManager::DestroyTransformBuffer(uint32_t a_addr)
{
    const std::unique_lock l = std::unique_lock(Instance->m_transformBuffer.Lock());

    Instance->m_freeTransforms.emplace(a_addr);
}

glm::mat4 ObjectManager::GetMatrix(uint32_t a_addr)
{
    ICARIAN_ASSERT_MSG(a_addr < Instance->m_transformBuffer.Size(), "GetMatrix out of bounds");

    TLockArray<TransformBuffer> a = Instance->m_transformBuffer.ToLockArray();

    const TransformBuffer& buffer = a[a_addr];

    return buffer.ToMat4();
}
glm::mat4 ObjectManager::GetGlobalMatrix(uint32_t a_addr)
{
    ICARIAN_ASSERT_MSG(a_addr < Instance->m_transformBuffer.Size(), "GetGlobalMatrix out of bounds");

    TLockArray<TransformBuffer> a = Instance->m_transformBuffer.ToLockArray();

    TransformBuffer buffer = a[a_addr];
    glm::mat4 transform = buffer.ToMat4();

    while (buffer.ParentAddr != -1)
    {
        buffer = a[buffer.ParentAddr];

        transform = buffer.ToMat4() * transform;
    }

    return transform;
}
