#include "Physics/PhysicsEngineBindings.h"

#include <Jolt/Jolt.h>

#include <glm/gtx/matrix_decompose.hpp>
#include <Jolt/Core/Core.h>
#include <Jolt/Physics/Body/Body.h>
#include <Jolt/Physics/Body/BodyManager.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/MassProperties.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/CylinderShape.h>
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
#include "Flare/IcarianDefer.h"
#include "ObjectManager.h"
#include "Physics/PhysicsEngine.h"
#include "Runtime/RuntimeManager.h"
#include "Trace.h"

static PhysicsEngineBindings* Instance = nullptr;

#define PHYSICSENGINE_RUNTIME_ATTACH(ret, namespace, klass, name, code, ...) a_runtime->BindFunction(RUNTIME_FUNCTION_STRING(namespace, klass, name), (void*)RUNTIME_FUNCTION_NAME(klass, name));

#define PHYSICSENGINE_BINDING_FUNCTION_TABLE(F) \
    F(uint32_t, IcarianEngine.Physics.Shapes, SphereCollisionShape, CreateSphere, { return Instance->CreateSphereShape(a_radius); }, float a_radius) \
    F(float, IcarianEngine.Physics.Shapes, SphereCollisionShape, GetRadius, { return Instance->GetSphereShapeRadius(a_addr); }, uint32_t a_addr) \
    \
    F(uint32_t, IcarianEngine.Physics.Shapes, BoxCollisionShape, CreateBox, { return Instance->CreateBoxShape(a_extents); }, glm::vec3 a_extents) \
    F(glm::vec3, IcarianEngine.Physics.Shapes, BoxCollisionShape, GetExtents, { return Instance->GetBoxShapeExtents(a_addr); }, uint32_t a_addr) \
    \
    F(uint32_t, IcarianEngine.Physics.Shapes, CapsuleCollisionShape, CreateCapsule, { return Instance->CreateCapsuleShape(a_height, a_radius); }, float a_height, float a_radius) \
    F(float, IcarianEngine.Physics.Shapes, CapsuleCollisionShape, GetHeight, { return Instance->GetCapsuleShapeHeight(a_addr); }, uint32_t a_addr) \
    F(float, IcarianEngine.Physics.Shapes, CapsuleCollisionShape, GetRadius, { return Instance->GetCasuleShapeRadius(a_addr); }, uint32_t a_addr) \
    \
    F(uint32_t, IcarianEngine.Physics.Shapes, CylinderCollisionShape, CreateCylinder, { return Instance->CreateCylinderShape(a_height, a_radius); }, float a_height, float a_radius) \
    F(float, IcarianEngine.Physics.Shapes, CylinderCollisionShape, GetHeight, { return Instance->GetCylinderShapeHeight(a_addr); }, uint32_t a_addr) \
    F(float, IcarainEngine.Physics.Shapes, CylinderCollisionShape, GetRadius, { return Instance->GetCylinderShapeRadius(a_addr); }, uint32_t a_addr) \
    \
    F(void, IcarianEngine.Physics.Shapes, CollisionShape, DestroyShape, { Instance->DestroyCollisionShape(a_addr); }, uint32_t a_addr) \
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
    TRACE("Creating Sphere Shape");

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
float PhysicsEngineBindings::GetSphereShapeRadius(uint32_t a_addr) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_engine->m_collisionShapes.Size(), "GetSphereShapeRadius out of bounds");

    const JPH::ShapeRefC shape = m_engine->m_collisionShapes[a_addr].Get();
    
    ICARIAN_ASSERT_MSG(shape->GetSubType() == JPH::EShapeSubType::Sphere, "GetSphereShapeRadius non sphere shape");

    return shape->GetInnerRadius();
}

uint32_t PhysicsEngineBindings::CreateBoxShape(const glm::vec3& a_extents) const
{
    TRACE("Creating Plane Shape");

    const glm::vec3 halfExtents = a_extents * 0.5f;

    const JPH::BoxShapeSettings boxSettings = JPH::BoxShapeSettings(JPH::RVec3(halfExtents.x, halfExtents.y, halfExtents.z));
    const JPH::ShapeSettings::ShapeResult result = boxSettings.Create();
    
    ICARIAN_ASSERT_MSG(result.IsValid(), "CreateBoxShape invalid box shape");
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
glm::vec3 PhysicsEngineBindings::GetBoxShapeExtents(uint32_t a_addr) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_engine->m_collisionShapes.Size(), "GetBoxShapeExtents out of bounds");

    const JPH::ShapeRefC shape = m_engine->m_collisionShapes[a_addr].Get();

    ICARIAN_ASSERT_MSG(shape->GetSubType() == JPH::EShapeSubType::Box, "GetBoxShapeExtents non box shape");
    
    const JPH::BoxShape* bShape = (JPH::BoxShape*)shape.GetPtr();

    const JPH::RVec3 val = bShape->GetHalfExtent();

    return glm::vec3(val.GetX(), val.GetY(), val.GetZ()) * 2.0f;
}

uint32_t PhysicsEngineBindings::CreateCapsuleShape(float a_height, float a_radius) const
{
    TRACE("Creating Capsule Shape");

    const JPH::CapsuleShapeSettings capsuleSettings = JPH::CapsuleShapeSettings(a_height * 0.5f, a_radius);
    const JPH::ShapeSettings::ShapeResult result = capsuleSettings.Create();

    ICARIAN_ASSERT_MSG(result.IsValid(), "CreateCapsuleShape invalid capsule shape");
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
float PhysicsEngineBindings::GetCapsuleShapeHeight(uint32_t a_addr) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_engine->m_collisionShapes.Size(), "GetCapsuleShapeHeight out of bounds");

    const JPH::ShapeRefC shape = m_engine->m_collisionShapes[a_addr].Get();

    ICARIAN_ASSERT_MSG(shape->GetSubType() == JPH::EShapeSubType::Capsule, "GetCapsuleShapeHeight non capsule shape");

    const JPH::CapsuleShape* cShape = (JPH::CapsuleShape*)shape.GetPtr();

    return cShape->GetHalfHeightOfCylinder() * 2.0f;
}
float PhysicsEngineBindings::GetCasuleShapeRadius(uint32_t a_addr) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_engine->m_collisionShapes.Size(), "GetCasuleShapeRadius out of bounds");

    const JPH::ShapeRefC shape = m_engine->m_collisionShapes[a_addr].Get();

    ICARIAN_ASSERT_MSG(shape->GetSubType() == JPH::EShapeSubType::Capsule, "GetCasuleShapeRadius non capsule shape");

    const JPH::CapsuleShape* cShape = (JPH::CapsuleShape*)shape.GetPtr();

    return cShape->GetRadius();
}

uint32_t PhysicsEngineBindings::CreateCylinderShape(float a_height, float a_radius) const
{
    TRACE("Creating Cylinder Shape");

    const JPH::CylinderShapeSettings cylinderSettings = JPH::CylinderShapeSettings(a_height * 0.5f, a_radius);
    const JPH::ShapeSettings::ShapeResult result = cylinderSettings.Create();

    ICARIAN_ASSERT_MSG(result.IsValid(), "CreateCylinderShape invalid cylinder shape");
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
float PhysicsEngineBindings::GetCylinderShapeHeight(uint32_t a_addr) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_engine->m_collisionShapes.Size(), "GetCylinderShapeHeight out of bounds");

    const JPH::ShapeRefC shape = m_engine->m_collisionShapes[a_addr].Get();

    ICARIAN_ASSERT_MSG(shape->GetSubType() == JPH::EShapeSubType::Cylinder, "GetCylinderShapeHeight non cylinder shape");

    const JPH::CylinderShape* cShape = (JPH::CylinderShape*)shape.GetPtr();

    return cShape->GetHalfHeight() * 2.0f;
}
float PhysicsEngineBindings::GetCylinderShapeRadius(uint32_t a_addr) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_engine->m_collisionShapes.Size(), "GetCylinderShapeRadius out of bounds");

    const JPH::ShapeRefC shape = m_engine->m_collisionShapes[a_addr].Get();

    ICARIAN_ASSERT_MSG(shape->GetSubType() == JPH::EShapeSubType::Cylinder, "GetCylinderShapeRadius non cylinder shape");

    const JPH::CylinderShape* cShape = (JPH::CylinderShape*)shape.GetPtr();

    return cShape->GetRadius();
}

void PhysicsEngineBindings::DestroyCollisionShape(uint32_t a_addr) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_engine->m_collisionShapes.Size(), "DestroyCollisionShape out of bounds");

    m_engine->m_collisionShapes[a_addr].Clear();
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
    
    Logger::Message("t addr" + std::to_string(a_transformAddr));

    const struct 
    {
        IcBodyActivationListener* Listener;
        JPH::BodyID ID;
    } activationVal = { m_engine->m_activationListener, id };

    ICARIAN_DEFER(activationVal, activationVal.Listener->OnBodyActivated(activationVal.ID, 0));

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