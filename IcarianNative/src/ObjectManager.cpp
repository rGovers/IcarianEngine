#include "ObjectManager.h"

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

OBJECTMANAGER_BINDING_FUNCTION_TABLE(RUNTIME_FUNCTION_DEFINITION);

ObjectManager::ObjectManager(RuntimeManager* a_runtime)
{
    OManager = this;

    TRACE("Binding Object functions to C#");
    OBJECTMANAGER_BINDING_FUNCTION_TABLE(OBJECTMANAGER_RUNTIME_ATTACH);
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

    m_transformBuffer.Push(Buffer);

    return m_transformBuffer.Size() - 1;
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