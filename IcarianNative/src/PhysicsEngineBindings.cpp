#include "Physics/PhysicsEngineBindings.h"

#include <Jolt/Jolt.h>

#include <glm/gtx/matrix_decompose.hpp>
#include <Jolt/Core/Core.h>
#include <Jolt/Geometry/OrientedBox.h>
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
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/CollisionCollectorImpl.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/EActivation.h>
#include <mutex>

#include "DataTypes/TLockArray.h"
#include "Flare/IcarianAssert.h"
#include "Flare/IcarianDefer.h"
#include "ObjectManager.h"
#include "Physics/PhysicsEngine.h"
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
    F(void, IcarianEngine.Physics, RigidBody, DestroyRigidBody, { Instance->DestroyPhysicsBody(a_addr); }, uint32_t a_addr) \
    \
    F(uint32_t, IcarianEngine.Physics, TriggerBody, CreateTriggerBody, { return Instance->CreateTriggerBody(a_transformAddr, a_colliderAddr); }, uint32_t a_transformAddr, uint32_t a_colliderAddr) \
    F(void, IcarianEngine.Physics, TriggerBody, DestroyTriggerBody, { Instance->DestroyPhysicsBody(a_addr); }, uint32_t a_addr) \
    \
    F(void, IcarianEngine.Physics, Physics, SetGravity, { Instance->SetGravity(a_gravity); }, glm::vec3 a_gravity) \
    F(glm::vec3, IcarianEngine.Physics, Physics, GetGravity, { return Instance->GetGravity(); }) \
    \
    F(MonoArray*, IcarianEngine.Physics, Physics, RaycastS, { return Instance->Raycast(a_pos, a_dir); }, glm::vec3 a_pos, glm::vec3 a_dir) \
    F(MonoArray*, IcarianEngine.Physics, Physics, SphereCollisionS, { return Instance->SphereCollision(a_pos, a_radius); }, glm::vec3 a_pos, float a_radius) \
    F(MonoArray*, IcarianEngine.Physics, Physics, BoxCollisionS, { return Instance->BoxCollision(a_transform, a_extents); }, MonoArray* a_transform, glm::vec3 a_extents) \
    F(MonoArray*, IcarianEngine.Physics, Physics, AABBCollisionS, { return Instance->AABBCollision(a_min, a_max); }, glm::vec3 a_min, glm::vec3 a_max)

PHYSICSENGINE_BINDING_FUNCTION_TABLE(RUNTIME_FUNCTION_DEFINITION);

PhysicsEngineBindings::PhysicsEngineBindings(PhysicsEngine* a_engine, RuntimeManager* a_runtime)
{
    TRACE("Binding physics functions to C#");
    
    m_engine = a_engine;
    m_runtime = a_runtime;

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

void PhysicsEngineBindings::AddBody(JPH::uint32 a_id, uint32_t a_index) const
{
    const std::unique_lock g = std::unique_lock(m_engine->m_mapMutex);

    auto iter = m_engine->m_bodyMap.find(a_id);
    if (iter != m_engine->m_bodyMap.end()) 
    {
        iter->second = a_index;
    } 
    else 
    {
        m_engine->m_bodyMap.emplace(a_id, a_index);
    }
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
                AddBody(id.GetIndex(), i);

                a[i] = binding;

                return i;
            }
        }
    }

    const uint32_t index = m_engine->m_bodyBindings.PushVal(binding);

    AddBody(id.GetIndex(), index);

    return index;
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
    ICARIAN_ASSERT_MSG(a_colliderAddr < m_engine->m_collisionShapes.Size(), "CreateRigidBody out of bounds");

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
                AddBody(id.GetIndex(), i);

                a[i] = binding;

                return i;
            }
        }
    }

    const uint32_t index = m_engine->m_bodyBindings.PushVal(binding);

    AddBody(id.GetIndex(), index);

    return index;
}

uint32_t PhysicsEngineBindings::CreateTriggerBody(uint32_t a_transformAddr, uint32_t a_colliderAddr) const
{
    ICARIAN_ASSERT_MSG(a_colliderAddr < m_engine->m_collisionShapes.Size(), "CreateTriggerBody out of bounds");

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
        JPH::EMotionType::Kinematic,
        PhysicsEngine::LayerTrigger
    );
    bodySettings.mIsSensor = true;

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
                AddBody(id.GetIndex(), i);

                a[i] = binding;

                return i;
            }
        }
    }

    const uint32_t index = m_engine->m_bodyBindings.PushVal(binding);

    AddBody(id.GetIndex(), index);

    return index;
}

void PhysicsEngineBindings::SetGravity(const glm::vec3 &a_gravity) const
{
    m_engine->m_physicsSystem->SetGravity(JPH::Vec3(a_gravity.x, a_gravity.y, a_gravity.z));
}
glm::vec3 PhysicsEngineBindings::GetGravity() const
{
    const JPH::Vec3 gravity = m_engine->m_physicsSystem->GetGravity();

    return glm::vec3(gravity.GetX(), gravity.GetY(), gravity.GetZ());
}

RaycastResult* PhysicsEngineBindings::Raycast(const glm::vec3& a_pos, const glm::vec3& a_dir, uint32_t* a_resultCount) const
{
    *a_resultCount = 0;

    const JPH::BroadPhaseQuery& broad = m_engine->m_physicsSystem->GetBroadPhaseQuery();

    JPH::RayCast ray;
    ray.mOrigin = JPH::Vec3(a_pos.x, a_pos.y, a_pos.z);
    ray.mDirection = JPH::Vec3(a_dir.x, a_dir.y, a_dir.z);

    JPH::AllHitCollisionCollector<JPH::RayCastBodyCollector> collector;
    broad.CastRay(ray, collector);

    if (collector.HadHit())
    {
        *a_resultCount = (uint32_t)collector.mHits.size();

        const JPH::BroadPhaseCastResult* rayResults = collector.mHits.data();

        RaycastResult* results = new RaycastResult[*a_resultCount];
        for (uint32_t i = 0; i < *a_resultCount; ++i)
        {
            const JPH::Vec3 pos = ray.GetPointOnRay(rayResults[i].mFraction);

            results[i].Position = glm::vec3(pos.GetX(), pos.GetY(), pos.GetZ());
            results[i].BodyAddr = m_engine->GetBodyAddr(rayResults[i].mBodyID.GetIndex());
        }

        return results;
    }

    return nullptr;
}
MonoArray* PhysicsEngineBindings::Raycast(const glm::vec3& a_pos, const glm::vec3& a_dir) const
{
    uint32_t resultCount;
    RaycastResult* results = Raycast(a_pos, a_dir, &resultCount);
    if (results != nullptr)
    {
        ICARIAN_DEFER_delA(results);

        MonoClass* klass = m_runtime->GetClass("IcarianEngine.Physics", "RaycastResultS");
        MonoArray* arr = mono_array_new(m_runtime->GetDomain(), klass, (uintptr_t)resultCount);

        for (uint32_t i = 0; i < resultCount; ++i)
        {
            mono_array_set(arr, RaycastResult, i, results[i]);
        }

        return arr;
    }

    return nullptr;
}

uint32_t* PhysicsEngineBindings::SphereCollision(const glm::vec3& a_pos, float a_radius, uint32_t* a_resultCount) const
{
    *a_resultCount = 0;

    const JPH::BroadPhaseQuery& broad = m_engine->m_physicsSystem->GetBroadPhaseQuery();

    JPH::AllHitCollisionCollector<JPH::CollideShapeBodyCollector> collector;
    broad.CollideSphere(JPH::Vec3(a_pos.x, a_pos.y, a_pos.z), a_radius, collector);

    if (collector.HadHit())
    {
        *a_resultCount = (uint32_t)collector.mHits.size();

        const JPH::BodyID* ids = collector.mHits.data();
        uint32_t* results = new uint32_t[*a_resultCount];

        for (uint32_t i = 0; i < *a_resultCount; ++i)
        {
            results[i] = m_engine->GetBodyAddr(ids[i].GetIndex());
        }

        return results;
    }

    return nullptr;
}
MonoArray* PhysicsEngineBindings::SphereCollision(const glm::vec3& a_pos, float a_radius) const
{
    uint32_t resultCount;
    uint32_t* data = SphereCollision(a_pos, a_radius, &resultCount);
    if (data != nullptr)
    {
        ICARIAN_DEFER_delA(data);

        MonoArray* arr = mono_array_new(m_runtime->GetDomain(), mono_get_uint32_class(), (uintptr_t)resultCount);

        for (uint32_t i = 0; i < resultCount; ++i)
        {
            mono_array_set(arr, uint32_t, i, data[i]);
        }

        return arr;
    }

    return nullptr;
}

uint32_t* PhysicsEngineBindings::BoxCollision(const glm::mat4& a_transform, const glm::vec3& a_extents, uint32_t* a_resultCount) const
{
    *a_resultCount = 0;

    const JPH::BroadPhaseQuery& broad = m_engine->m_physicsSystem->GetBroadPhaseQuery();

    const glm::vec3 halfExtents = a_extents * 0.5f;

    JPH::AllHitCollisionCollector<JPH::CollideShapeBodyCollector> collector;

    JPH::OrientedBox box;
    box.mHalfExtents = JPH::Vec3(halfExtents.x, halfExtents.y, halfExtents.z);
    box.mOrientation = JPH::Mat44
    (
        JPH::Vec4(a_transform[0][0], a_transform[0][1], a_transform[0][2], a_transform[0][3]),
        JPH::Vec4(a_transform[1][0], a_transform[1][1], a_transform[1][2], a_transform[1][3]),
        JPH::Vec4(a_transform[2][0], a_transform[2][1], a_transform[2][2], a_transform[2][3]),
        JPH::Vec4(a_transform[3][0], a_transform[3][1], a_transform[3][2], a_transform[3][3])
    );

    broad.CollideOrientedBox(box, collector);

    if (collector.HadHit())
    {
        *a_resultCount = (uint32_t)collector.mHits.size();

        const JPH::BodyID* ids = collector.mHits.data();
        uint32_t* results = new uint32_t[*a_resultCount];

        for (uint32_t i = 0; i < *a_resultCount; ++i)
        {
            results[i] = m_engine->GetBodyAddr(ids[i].GetIndex());
        }

        return results;
    }

    return nullptr;
}
MonoArray* PhysicsEngineBindings::BoxCollision(MonoArray* a_transform, const glm::vec3& a_extents) const
{
    glm::mat4 t;
    float* tDat = (float*)&t;

    for (int i = 0; i < 16; ++i)
    {
        tDat[i] = mono_array_get(a_transform, float, i);
    }

    uint32_t resultCount;
    uint32_t* data = BoxCollision(t, a_extents, &resultCount);

    if (data != nullptr)
    {
        ICARIAN_DEFER_delA(data);

        MonoArray* arr = mono_array_new(m_runtime->GetDomain(), mono_get_uint32_class(), (uintptr_t)resultCount);

        for (uint32_t i = 0; i < resultCount; ++i)
        {
            mono_array_set(arr, uint32_t, i, data[i]);
        }

        return arr;
    }
    
    return nullptr;
}

uint32_t* PhysicsEngineBindings::AABBCollision(const glm::vec3& a_min, const glm::vec3& a_max, uint32_t* a_resultCount) const
{
    *a_resultCount = 0;

    const JPH::BroadPhaseQuery& broad = m_engine->m_physicsSystem->GetBroadPhaseQuery();

    JPH::AllHitCollisionCollector<JPH::CollideShapeBodyCollector> collector;

    JPH::AABox box;
    box.mMin = JPH::Vec3(a_min.x, a_min.y, a_min.z);
    box.mMax = JPH::Vec3(a_max.x, a_max.y, a_max.z);

    broad.CollideAABox(box, collector);

    if (collector.HadHit())
    {
        *a_resultCount = (uint32_t)collector.mHits.size();

        const JPH::BodyID* ids = collector.mHits.data();    
        uint32_t* results = new uint32_t[*a_resultCount];

        for (uint32_t i = 0; i < *a_resultCount; ++i)
        {
            results[i] = m_engine->GetBodyAddr(ids[i].GetIndex());
        }

        return results;
    }

    return nullptr;
}
MonoArray* PhysicsEngineBindings::AABBCollision(const glm::vec3& a_min, const glm::vec3& a_max) const
{
    uint32_t resultCount;
    uint32_t* data = AABBCollision(a_min, a_max, &resultCount);

    if (data != nullptr)
    {
        ICARIAN_DEFER_delA(data);

        MonoArray* arr = mono_array_new(m_runtime->GetDomain(), mono_get_uint32_class(), (uintptr_t)resultCount);

        for (uint32_t i = 0; i < resultCount; ++i)
        {
            mono_array_set(arr, uint32_t, i, data[i]);
        }

        return arr;
    }

    return nullptr;
}