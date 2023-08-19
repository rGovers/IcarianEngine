#include "ObjectManager.h"

#include <glm/gtx/matrix_decompose.hpp>

#include "Flare/IcarianAssert.h"
#include "Runtime/RuntimeManager.h"
#include "Trace.h"

static ObjectManager* OManager = nullptr;

#define OBJECTMANAGER_RUNTIME_ATTACH(ret, namespace, klass, name, code, ...) a_runtime->BindFunction(RUNTIME_FUNCTION_STRING(namespace, klass, name), (void*)RUNTIME_FUNCTION_NAME(klass, name));

#define OBJECTMANAGER_BINDING_FUNCTION_TABLE(F) \
    F(uint32_t, IcarianEngine, Transform, GenerateTransformBuffer, { return OManager->CreateTransformBuffer(); }) \
    F(TransformBuffer, IcarianEngine, Transform, GetTransformBuffer, { return OManager->GetTransformBuffer(a_addr); }, uint32_t a_addr) \
    F(void, IcarianEngine, Transform, SetTransformBuffer, { OManager->SetTransformBuffer(a_addr, a_buffer); }, uint32_t a_addr, TransformBuffer a_buffer) \
    F(void, IcarianEngine, Transform, DestroyTransformBuffer, { OManager->DestroyTransformBuffer(a_addr); }, uint32_t a_addr)

RUNTIME_FUNCTION(MonoArray*, Transform, GetTransformMatrix, 
{
    MonoArray* a = mono_array_new(mono_domain_get(), mono_get_single_class(), 16);

    const TransformBuffer buffer = OManager->GetTransformBuffer(a_addr);
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

    const glm::mat4 mat = OManager->GetGlobalMatrix(a_addr);

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

    TransformBuffer buffer = OManager->GetTransformBuffer(a_addr);
    
    glm::vec3 skew;
    glm::vec4 perspective;

    glm::decompose(mat, buffer.Scale, buffer.Rotation, buffer.Translation, skew, perspective);

    OManager->SetTransformBuffer(a_addr, buffer);
}, uint32_t a_addr, MonoArray* a_mat)

OBJECTMANAGER_BINDING_FUNCTION_TABLE(RUNTIME_FUNCTION_DEFINITION);

ObjectManager::ObjectManager(RuntimeManager* a_runtime)
{
    OManager = this;

    TRACE("Binding Object functions to C#");
    OBJECTMANAGER_BINDING_FUNCTION_TABLE(OBJECTMANAGER_RUNTIME_ATTACH);

    BIND_FUNCTION(a_runtime, IcarianEngine, Transform, GetTransformMatrix);
    BIND_FUNCTION(a_runtime, IcarianEngine, Transform, GetGlobalTransformMatrix);
    BIND_FUNCTION(a_runtime, IcarianEngine, Transform, SetTransformMatrix);
}
ObjectManager::~ObjectManager()
{

}

uint32_t ObjectManager::CreateTransformBuffer()
{
    constexpr TransformBuffer Buffer;

    TRACE("Creating Transform Buffer");
    if (!m_freeTransforms.empty())
    {
        const uint32_t add = m_freeTransforms.front();
        m_freeTransforms.pop();

        m_transformBuffer.LockSet(add, Buffer);

        return add;
    }

    TRACE("Allocating Transform Buffer");
    
    return m_transformBuffer.PushVal(Buffer);
}
TransformBuffer ObjectManager::GetTransformBuffer(uint32_t a_addr)
{
    ICARIAN_ASSERT_MSG(a_addr < m_transformBuffer.Size(), "GetTransformBuffer out of bounds");

    return m_transformBuffer[a_addr];
}
void ObjectManager::SetTransformBuffer(uint32_t a_addr, const TransformBuffer& a_buffer)
{
    ICARIAN_ASSERT_MSG(a_addr < m_transformBuffer.Size(), "SetTransformBuffer out of bounds");

    m_transformBuffer.LockSet(a_addr, a_buffer);
}
void ObjectManager::DestroyTransformBuffer(uint32_t a_addr)
{
    TRACE("Destroying Transform Buffer");

    m_freeTransforms.emplace(a_addr);
}

glm::mat4 ObjectManager::GetGlobalMatrix(uint32_t a_addr)
{
    ICARIAN_ASSERT_MSG(a_addr < m_transformBuffer.Size(), "GetGlobalMatrix out of bounds");

    return m_transformBuffer[a_addr].ToGlobalMat4(this);
}