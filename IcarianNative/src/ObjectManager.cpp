#include "ObjectManager.h"

#include <glm/gtx/matrix_decompose.hpp>

#include "Flare/IcarianAssert.h"
#include "Runtime/RuntimeManager.h"
#include "Trace.h"

static ObjectManager* Instance = nullptr;

#define OBJECTMANAGER_BINDING_FUNCTION_TABLE(F) \
    F(uint32_t, IcarianEngine, Transform, GenerateTransformBuffer, { return Instance->CreateTransformBuffer(); }) \
    F(TransformBuffer, IcarianEngine, Transform, GetTransformBuffer, { return Instance->GetTransformBuffer(a_addr); }, uint32_t a_addr) \
    F(void, IcarianEngine, Transform, SetTransformBuffer, { Instance->SetTransformBuffer(a_addr, a_buffer); }, uint32_t a_addr, TransformBuffer a_buffer) \
    F(void, IcarianEngine, Transform, DestroyTransformBuffer, { Instance->DestroyTransformBuffer(a_addr); }, uint32_t a_addr)

RUNTIME_FUNCTION(MonoArray*, Transform, GetTransformMatrix, 
{
    MonoArray* a = mono_array_new(mono_domain_get(), mono_get_single_class(), 16);

    const TransformBuffer buffer = ObjectManager::GetTransformBuffer(a_addr);
    const glm::mat4 mat = buffer.ToMat4();

    for (int i = 0; i < 16; ++i)
    {
        mono_array_set(a, float, i, mat[i / 4][i % 4]);
    }

    return a;
}, uint32_t a_addr)
RUNTIME_FUNCTION(MonoArray*, Transform, GetGlobalTransformMatrix,
{
    MonoArray* a = mono_array_new(mono_domain_get(), mono_get_single_class(), 16);

    const glm::mat4 mat = ObjectManager::GetGlobalMatrix(a_addr);

    for (int i = 0; i < 16; ++i)
    {
        mono_array_set(a, float, i, mat[i / 4][i % 4]);
    }

    return a;
}, uint32_t a_addr)

RUNTIME_FUNCTION(void, Transform, SetTransformMatrix,
{
    glm::mat4 mat;
    float* dat = (float*)&mat;

    for (uint32_t i = 0; i < 16; ++i)
    {
        dat[i] = mono_array_get(a_mat, float, i);
    }

    TransformBuffer buffer = ObjectManager::GetTransformBuffer(a_addr);
    
    glm::vec3 skew;
    glm::vec4 perspective;

    glm::decompose(mat, buffer.Scale, buffer.Rotation, buffer.Translation, skew, perspective);

    ObjectManager::SetTransformBuffer(a_addr, buffer);
}, uint32_t a_addr, MonoArray* a_mat)

OBJECTMANAGER_BINDING_FUNCTION_TABLE(RUNTIME_FUNCTION_DEFINITION);

ObjectManager::ObjectManager()
{
    TRACE("Binding Object functions to C#");
    OBJECTMANAGER_BINDING_FUNCTION_TABLE(RUNTIME_FUNCTION_ATTACH);

    BIND_FUNCTION(IcarianEngine, Transform, GetTransformMatrix);
    BIND_FUNCTION(IcarianEngine, Transform, GetGlobalTransformMatrix);
    BIND_FUNCTION(IcarianEngine, Transform, SetTransformMatrix);
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

uint32_t ObjectManager::CreateTransformBuffer()
{
    constexpr TransformBuffer Buffer;

    TLockArray<TransformBuffer> a = Instance->m_transformBuffer.ToLockArray();

    TRACE("Creating Transform Buffer");
    if (!Instance->m_freeTransforms.empty())
    {
        const uint32_t addr = Instance->m_freeTransforms.front();
        Instance->m_freeTransforms.pop();

        a[addr] = Buffer;

        return addr;
    }

    TRACE("Allocating Transform Buffer");
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
    TRACE("Destroying Transform Buffer");

    const std::unique_lock l = std::unique_lock(Instance->m_transformBuffer.Lock());

    Instance->m_freeTransforms.emplace(a_addr);
}

glm::mat4 ObjectManager::GetGlobalMatrix(uint32_t a_addr)
{
    ICARIAN_ASSERT_MSG(a_addr < Instance->m_transformBuffer.Size(), "GetGlobalMatrix out of bounds");

    return Instance->m_transformBuffer[a_addr].ToGlobalMat4();
}

glm::mat4 TransformBuffer::ToMat4() const
{
    constexpr glm::mat4 Iden = glm::identity<glm::mat4>();

    const glm::mat4 translation = glm::translate(Iden, Translation);
    const glm::mat4 rotation = glm::toMat4(Rotation);
    const glm::mat4 scale = glm::scale(Iden, Scale);

    return translation * rotation * scale;
}

glm::mat4 TransformBuffer::ToGlobalMat4() const
{
    if (Parent != -1)
    {
        const TransformBuffer buffer = ObjectManager::GetTransformBuffer(Parent);

        return buffer.ToGlobalMat4() * ToMat4();
    }

    return ToMat4();
}