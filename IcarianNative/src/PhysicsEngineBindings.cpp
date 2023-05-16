#include "Physics/PhysicsEngineBindings.h"

#include <Jolt/Jolt.h>

#include <glm/gtx/matrix_decompose.hpp>
#include <Jolt/Physics/Body/Body.h>
#include <Jolt/Physics/Body/BodyManager.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/MassProperties.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Math/Quat.h>
#include <Jolt/Math/Real.h>
#include <Jolt/Physics/Body/BodyID.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include <Jolt/Physics/Body/MotionType.h>
#include <Jolt/Physics/EActivation.h>
#include <mutex>

#include "DataTypes/TLockArray.h"
#include "Flare/IcarianAssert.h"

#include "Jolt/Core/Core.h"
#include "ObjectManager.h"
#include "Physics/PhysicsEngine.h"
#include "Runtime/RuntimeManager.h"
#include "Trace.h"

static PhysicsEngineBindings* Instance = nullptr;

#define PHYSICSENGINE_RUNTIME_ATTACH(ret, namespace, klass, name, code, ...) a_runtime->BindFunction(RUNTIME_FUNCTION_STRING(namespace, klass, name), (void*)RUNTIME_FUNCTION_NAME(klass, name));

#define PHYSICSENGINE_BINDING_FUNCTION_TABLE(F) \
    F(uint32_t, IcarianEngine.Physics.Shapes, SphereCollisionShape, CreateSphere, { return Instance->CreateSphereShape(a_radius); }, float a_radius) \
    F(void, IcarianEngine.Physics.Shapes, SphereCollisionShape, DestroySphere, { Instance->DestroySphereShape(a_addr); }, uint32_t a_addr) \
    F(float, IcarianEngine.Physics.Shapes, SphereCollisionShape, GetRadius, { return Instance->GetSphereShapeRadius(a_addr); }, uint32_t a_addr) \
    \
    F(uint32_t, IcarianEngine.Physics, PhysicsBody, CreatePhysicsBody, { return Instance->CreatePhysicsBody(a_transformAddr, a_colliderAddr); }, uint32_t a_transformAddr, uint32_t a_colliderAddr) \
    F(void, IcarianEngine.Physics, PhysicsBody, DestroyPhysicsBody, { Instance->DestroyPhysicsBody(a_addr); }, uint32_t a_addr) \
    \
    F(uint32_t, IcarianEngine.Physics, RigidBody, CreateRigidBody, { return Instance->CreateRigidBody(a_transformAddr, a_colliderAddr, a_mass); }, uint32_t a_transformAddr, uint32_t a_colliderAddr, float a_mass) \
    F(void, IcarianEngine.Physics, RigidBody, DestroyRigidBody, { Instance->DestroyPhysicsBody(a_addr); }, uint32_t a_addr) 

PHYSICSENGINE_BINDING_FUNCTION_TABLE(RUNTIME_FUNCTION_DEFINITION);

PhysicsEngineBindings::PhysicsEngineBindings(PhysicsEngine* a_engine, RuntimeManager* a_runtime)
{
    TRACE("Binding physics functions to C#");
    
    m_engine = a_engine;

    Instance = this;

    PHYSICSENGINE_BINDING_FUNCTION_TABLE(PHYSICSENGINE_RUNTIME_ATTACH);
}
PhysicsEngineBindings::~PhysicsEngineBindings()
{

}

uint32_t PhysicsEngineBindings::CreateSphereShape(float a_radius) const
{
    TRACE("Creating Physics Shape");

    const JPH::SphereShapeSettings sphereSettings = JPH::SphereShapeSettings(a_radius);
    const JPH::ShapeSettings::ShapeResult result = sphereSettings.Create();

    ICARIAN_ASSERT_MSG(result.IsValid(), "CreateSphereShape invalid sphere shape");
    {
        TLockArray<JPH::ShapeSettings::ShapeResult> a = m_engine->m_collisionShapes.ToLockArray();
        const uint32_t size = a.Size();

        for (uint32_t i = 0; i < size; ++i)
        {
            if (!a[i].IsValid())
            {
                a[i] = result;

                return i;
            }
        }
    }

    return m_engine->m_collisionShapes.PushVal(result);
}
void PhysicsEngineBindings::DestroySphereShape(uint32_t a_addr) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_engine->m_collisionShapes.Size(), "DestroySphereShape out of bounds");

    m_engine->m_collisionShapes[a_addr].Clear();
}
float PhysicsEngineBindings::GetSphereShapeRadius(uint32_t a_addr) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_engine->m_collisionShapes.Size(), "GetSphereShapeRadius out of bounds");

    const JPH::ShapeRefC shape = m_engine->m_collisionShapes[a_addr].Get();
    
    ICARIAN_ASSERT_MSG(shape->GetSubType() == JPH::EShapeSubType::Sphere, "GetSphereShapeRadius non sphere shape");

    return shape->GetInnerRadius();
}

uint32_t PhysicsEngineBindings::CreatePhysicsBody(uint32_t a_transformAddr, uint32_t a_colliderAddr) const
{
    TRACE("Creating Physics Body");

    ICARIAN_ASSERT_MSG(a_colliderAddr < m_engine->m_collisionShapes.Size(), "CreatePhysicsBody out of bounds");

    const glm::mat4 globalTransform = m_engine->m_objectManager->GetGlobalMatrix(a_transformAddr);

    glm::vec3 translation;
    glm::quat rotation;
    glm::vec3 scale;
    glm::vec3 skew;
    glm::vec4 perspective;
    glm::decompose(globalTransform, scale, rotation, translation, skew, perspective);

    const JPH::BodyCreationSettings bodySettings = JPH::BodyCreationSettings
    (
        m_engine->m_collisionShapes[a_colliderAddr].Get(), 
        JPH::RVec3(translation.x, translation.y, translation.z), 
        JPH::Quat(rotation.x, rotation.y, rotation.z, rotation.w), 
        JPH::EMotionType::Static, 
        PhysicsEngine::LayerNonMoving
    );

    JPH::BodyInterface& interface = m_engine->m_physicsSystem->GetBodyInterface();
    const JPH::BodyID id = interface.CreateAndAddBody(bodySettings, JPH::EActivation::DontActivate);

    const BodyBinding binding = BodyBinding(a_transformAddr, id);

    {
        TLockArray<BodyBinding> a = m_engine->m_bodyBindings.ToLockArray();
        const uint32_t size = a.Size();

        for (uint32_t i = 0; i < size; ++i)
        {
            if (a[i].TransformAddr == -1)
            {
                a[i] = binding;

                return i;
            }
        }
    }

    return m_engine->m_bodyBindings.PushVal(binding);
}
void PhysicsEngineBindings::DestroyPhysicsBody(uint32_t a_addr) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_engine->m_bodyBindings.Size(), "DestroyPhysicsBody out of bounds");

    BodyBinding& binding = m_engine->m_bodyBindings[a_addr];

    JPH::BodyInterface& interface = m_engine->m_physicsSystem->GetBodyInterface();
    interface.RemoveBody(binding.Body);

    binding.TransformAddr = -1;
}

uint32_t PhysicsEngineBindings::CreateRigidBody(uint32_t a_transformAddr, uint32_t a_colliderAddr, float a_mass) const
{
    ICARIAN_ASSERT_MSG_R(a_colliderAddr < m_engine->m_collisionShapes.Size(), "CreateRigidBody out of bounds");

    const glm::mat4 globalTransform = m_engine->m_objectManager->GetGlobalMatrix(a_transformAddr);

    glm::vec3 translation;
    glm::quat rotation;
    glm::vec3 scale;
    glm::vec3 skew;
    glm::vec4 perspective;
    glm::decompose(globalTransform, scale, rotation, translation, skew, perspective);

    JPH::BodyCreationSettings bodySettings = JPH::BodyCreationSettings
    (
        m_engine->m_collisionShapes[a_colliderAddr].Get(),
        JPH::RVec3(translation.x, translation.y, translation.z),
        JPH::Quat(rotation.x, rotation.y, rotation.z, rotation.w),
        JPH::EMotionType::Dynamic,
        PhysicsEngine::LayerMoving
    );
    bodySettings.mMassPropertiesOverride.mMass = a_mass;

    JPH::BodyInterface& interface = m_engine->m_physicsSystem->GetBodyInterface();
    const JPH::BodyID id = interface.CreateAndAddBody(bodySettings, JPH::EActivation::Activate);
    
    const BodyBinding binding = BodyBinding(a_transformAddr, id);
    {
        TLockArray<BodyBinding> a = m_engine->m_bodyBindings.ToLockArray();
        const uint32_t size = a.Size();
        
        for (uint32_t i = 0; i < size; ++i)
        {
            if (a[i].TransformAddr == -1)
            {
                const std::unique_lock g = std::unique_lock(m_engine->m_mapMutex);
                const JPH::uint32 bIndex = id.GetIndex();

                auto iter = m_engine->m_bodyMap.find(bIndex);
                if (iter != m_engine->m_bodyMap.end())
                {
                    iter->second = i;
                }
                else 
                {
                    m_engine->m_bodyMap.emplace(bIndex, i);
                }

                a[i] = binding;

                return i;
            }
        }
    }

    const std::unique_lock g = std::unique_lock(m_engine->m_mapMutex);
    const JPH::uint32 bIndex = id.GetIndex();

    const uint32_t index = m_engine->m_bodyBindings.PushVal(binding);

    auto iter = m_engine->m_bodyMap.find(bIndex);
    if (iter != m_engine->m_bodyMap.end()) 
    {
        iter->second = index;
    } 
    else 
    {
        m_engine->m_bodyMap.emplace(bIndex, index);
    }

    return index;
}