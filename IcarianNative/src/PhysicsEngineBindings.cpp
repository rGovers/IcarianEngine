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

#include "Core/Bitfield.h"
#include "Core/IcarianAssert.h"
#include "Core/IcarianDefer.h"
#include "IcarianError.h"
#include "ObjectManager.h"
#include "Physics/InterfaceLock.h"
#include "Physics/PhysicsEngine.h"
#include "Runtime/RuntimeManager.h"
#include "Trace.h"

#include "EngineBoxCollisionShapeInterop.h"
#include "EngineCapsuleCollisionShapeInterop.h"
#include "EngineCollisionShapeInterop.h"
#include "EngineCylinderCollisionShapeInterop.h"
#include "EnginePhysicsBodyInterop.h"
#include "EnginePhysicsInterop.h"
#include "EngineRigidBodyInterop.h"
#include "EngineSphereCollisionShapeInterop.h"
#include "EngineTriggerBodyInterop.h"

static PhysicsEngineBindings* Instance = nullptr;

ENGINE_COLLISIONSHAPE_EXPORT_TABLE(RUNTIME_FUNCTION_DEFINITION);

ENGINE_BOXCOLLISIONSHAPE_EXPORT_TABLE(RUNTIME_FUNCTION_DEFINITION);
ENGINE_CAPSULECOLLISIONSHAPE_EXPORT_TABLE(RUNTIME_FUNCTION_DEFINITION);
ENGINE_CYLINDERCOLLISIONSHAPE_EXPORT_TABLE(RUNTIME_FUNCTION_DEFINITION);
ENGINE_SPHERECOLLISIONSHAPE_EXPORT_TABLE(RUNTIME_FUNCTION_DEFINITION);

ENGINE_PHYSICSBODY_EXPORT_TABLE(RUNTIME_FUNCTION_DEFINITION);
ENGINE_RIGIDBODY_EXPORT_TABLE(RUNTIME_FUNCTION_DEFINITION);
ENGINE_TRIGGERBODY_EXPORT_TABLE(RUNTIME_FUNCTION_DEFINITION);

ENGINE_PHYSICS_EXPORT_TABLE(RUNTIME_FUNCTION_DEFINITION);

PhysicsEngineBindings::PhysicsEngineBindings(PhysicsEngine* a_engine)
{
    TRACE("Binding physics functions to C#");
    
    m_engine = a_engine;

    Instance = this;

    ENGINE_COLLISIONSHAPE_EXPORT_TABLE(RUNTIME_FUNCTION_ATTACH);

    ENGINE_BOXCOLLISIONSHAPE_EXPORT_TABLE(RUNTIME_FUNCTION_ATTACH);
    ENGINE_CAPSULECOLLISIONSHAPE_EXPORT_TABLE(RUNTIME_FUNCTION_ATTACH);
    ENGINE_CYLINDERCOLLISIONSHAPE_EXPORT_TABLE(RUNTIME_FUNCTION_ATTACH);
    ENGINE_SPHERECOLLISIONSHAPE_EXPORT_TABLE(RUNTIME_FUNCTION_ATTACH);

    ENGINE_PHYSICSBODY_EXPORT_TABLE(RUNTIME_FUNCTION_ATTACH);
    ENGINE_RIGIDBODY_EXPORT_TABLE(RUNTIME_FUNCTION_ATTACH);
    ENGINE_TRIGGERBODY_EXPORT_TABLE(RUNTIME_FUNCTION_ATTACH);

    ENGINE_PHYSICS_EXPORT_TABLE(RUNTIME_FUNCTION_ATTACH);
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
    const ThreadGuard g = ThreadGuard(m_engine->m_mapLock);

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

    const glm::mat4 globalTransform = ObjectManager::GetGlobalMatrix(a_transformAddr);

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
    interface.DestroyBody(binding.Body);

    binding.TransformAddr = -1;
}
void PhysicsEngineBindings::SetPhysicsBodyPosition(uint32_t a_addr, const glm::vec3& a_pos) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_engine->m_bodyBindings.Size(), "SetPhysicsBodyPosition out of bounds");

    const BodyBinding binding = m_engine->m_bodyBindings[a_addr];

    JPH::BodyInterface& interface = m_engine->m_physicsSystem->GetBodyInterface();

    interface.SetPosition(binding.Body, JPH::Vec3(a_pos.x, a_pos.y, a_pos.z), JPH::EActivation::Activate);

    TransformBuffer buffer = ObjectManager::GetTransformBuffer(binding.TransformAddr);

    glm::mat4 invMat = glm::identity<glm::mat4>();
    if (buffer.ParentAddr != -1)
    {
        const glm::mat4 transformMat = ObjectManager::GetGlobalMatrix(buffer.ParentAddr);
        invMat = glm::inverse(transformMat);
    }

    buffer.Translation = (invMat * glm::vec4(a_pos, 1.0f)).xyz();

    ObjectManager::SetTransformBuffer(binding.TransformAddr, buffer);
}
void PhysicsEngineBindings::SetPhysicsBodyRotation(uint32_t a_addr, const glm::quat& a_rot) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_engine->m_bodyBindings.Size(), "SetPhysicsBodyRotation out of bounds");

    const BodyBinding binding = m_engine->m_bodyBindings[a_addr];

    JPH::BodyInterface& interface = m_engine->m_physicsSystem->GetBodyInterface();

    interface.SetRotation(binding.Body, JPH::Quat(a_rot.x, a_rot.y, a_rot.z, a_rot.w), JPH::EActivation::Activate);

    TransformBuffer buffer = ObjectManager::GetTransformBuffer(binding.TransformAddr);

    glm::quat invQuat = glm::identity<glm::quat>();
    if (buffer.ParentAddr != -1)
    {
        const glm::mat4 transformMat = ObjectManager::GetGlobalMatrix(buffer.ParentAddr);
        const glm::mat4 inv = glm::inverse(transformMat);

        glm::vec3 trans;
        glm::vec3 scale;
        glm::vec3 skew;
        glm::vec4 per;
        glm::decompose(inv, scale, invQuat, trans, skew, per);
    }

    buffer.Rotation = a_rot * invQuat;

    ObjectManager::SetTransformBuffer(binding.TransformAddr, buffer);
}

uint32_t PhysicsEngineBindings::CreateRigidBody(uint32_t a_transformAddr, uint32_t a_colliderAddr, uint32_t a_layer, float a_mass) const
{
    IVERIFY(a_colliderAddr < m_engine->m_collisionShapes.Size());
    IVERIFY(a_layer < 8);

    const glm::mat4 globalTransform = ObjectManager::GetGlobalMatrix(a_transformAddr);

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
        (JPH::ObjectLayer)a_layer
    );
    bodySettings.mMassPropertiesOverride.mMass = a_mass;
    bodySettings.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateInertia;

    JPH::BodyInterface& interface = m_engine->m_physicsSystem->GetBodyInterface();
    const JPH::BodyID id = interface.CreateAndAddBody(bodySettings, JPH::EActivation::Activate);

    IDEFER(m_engine->m_activationListener->OnBodyActivated(id, 0));

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
void PhysicsEngineBindings::SetRigidBodyGravityFactor(uint32_t a_addr, float a_factor) const
{
    IVERIFY(a_addr < m_engine->m_bodyBindings.Size());

    const BodyBinding binding = m_engine->m_bodyBindings[a_addr];

    const JPH::BodyLockInterfaceLocking& interface = m_engine->m_physicsSystem->GetBodyLockInterface();
    const PhysicsInterfaceWriteLock lock = PhysicsInterfaceWriteLock(binding.Body, interface);

    JPH::Body* body = interface.TryGetBody(binding.Body);

    JPH::MotionProperties* properties = body->GetMotionProperties();
    properties->SetGravityFactor(a_factor);
}
float PhysicsEngineBindings::GetRigidBodyGravityFactor(uint32_t a_addr) const
{
    IVERIFY(a_addr < m_engine->m_bodyBindings.Size());

    const BodyBinding binding = m_engine->m_bodyBindings[a_addr];

    const JPH::BodyLockInterfaceLocking& interface = m_engine->m_physicsSystem->GetBodyLockInterface();
    const PhysicsInterfaceReadLock lock = PhysicsInterfaceReadLock(binding.Body, interface);

    const JPH::Body* body = interface.TryGetBody(binding.Body);

    const JPH::MotionProperties* properties = body->GetMotionProperties();
    return properties->GetGravityFactor();
}
glm::vec3 PhysicsEngineBindings::GetRigidBodyVelocity(uint32_t a_addr) const
{
    IVERIFY(a_addr < m_engine->m_bodyBindings.Size());

    const BodyBinding binding = m_engine->m_bodyBindings[a_addr];

    const JPH::BodyLockInterfaceLocking& interface = m_engine->m_physicsSystem->GetBodyLockInterface();
    const PhysicsInterfaceReadLock lock = PhysicsInterfaceReadLock(binding.Body, interface);

    const JPH::Body* body = interface.TryGetBody(binding.Body);

    const JPH::Vec3 vel = body->GetLinearVelocity();

    return glm::vec3(vel.GetX(), vel.GetY(), vel.GetZ());
}
void PhysicsEngineBindings::SetRigidBodyVelocity(uint32_t a_addr, const glm::vec3& a_velocity) const
{
    IVERIFY(a_addr < m_engine->m_bodyBindings.Size());

    const BodyBinding binding = m_engine->m_bodyBindings[a_addr];

    const JPH::BodyLockInterfaceLocking& interface = m_engine->m_physicsSystem->GetBodyLockInterface();
    const PhysicsInterfaceWriteLock lock = PhysicsInterfaceWriteLock(binding.Body, interface);

    JPH::Body* body = interface.TryGetBody(binding.Body);
    body->SetLinearVelocity(JPH::Vec3(a_velocity.x, a_velocity.y, a_velocity.z));
}
glm::vec3 PhysicsEngineBindings::GetRigidBodyAngularVelocity(uint32_t a_addr) const
{
    IVERIFY(a_addr < m_engine->m_bodyBindings.Size());

    const BodyBinding binding = m_engine->m_bodyBindings[a_addr];

    const JPH::BodyLockInterfaceLocking& interface = m_engine->m_physicsSystem->GetBodyLockInterface();
    const PhysicsInterfaceReadLock lock = PhysicsInterfaceReadLock(binding.Body, interface);

    const JPH::Body* body = interface.TryGetBody(binding.Body);

    const JPH::Vec3 vel = body->GetAngularVelocity();

    return glm::vec3(vel.GetX(), vel.GetY(), vel.GetZ());
}
void PhysicsEngineBindings::SetRigidBodyAngularVelocity(uint32_t a_addr, const glm::vec3& a_velocity) const
{
    IVERIFY(a_addr < m_engine->m_bodyBindings.Size());

    const BodyBinding binding = m_engine->m_bodyBindings[a_addr];

    const JPH::BodyLockInterfaceLocking& interface = m_engine->m_physicsSystem->GetBodyLockInterface();
    const PhysicsInterfaceWriteLock lock = PhysicsInterfaceWriteLock(binding.Body, interface);

    JPH::Body* body = interface.TryGetBody(binding.Body);
    body->SetAngularVelocity(JPH::Vec3(a_velocity.x, a_velocity.y, a_velocity.z));
}
void PhysicsEngineBindings::RigidBodyAddForce(uint32_t a_addr, const glm::vec3& a_force, e_ForceMode a_mode) const
{
    IVERIFY(a_addr < m_engine->m_bodyBindings.Size());

    const BodyBinding binding = m_engine->m_bodyBindings[a_addr];

    const JPH::BodyLockInterfaceLocking& interface = m_engine->m_physicsSystem->GetBodyLockInterface();
    const PhysicsInterfaceWriteLock lock = PhysicsInterfaceWriteLock(binding.Body, interface);

    JPH::Body* body = interface.TryGetBody(binding.Body);
    switch (a_mode) 
    {
    case ForceMode_Impulse:
    {
        body->AddImpulse(JPH::Vec3(a_force.x, a_force.y, a_force.z));

        break;
    }
    default:
    {
        body->AddForce(JPH::Vec3(a_force.x, a_force.y, a_force.z));

        break;
    }
    }
}
void PhysicsEngineBindings::RigidBodyAddTorque(uint32_t a_addr, const glm::vec3& a_torque, e_ForceMode a_mode) const
{
    IVERIFY(a_addr < m_engine->m_bodyBindings.Size());

    const BodyBinding binding = m_engine->m_bodyBindings[a_addr];

    const JPH::BodyLockInterfaceLocking& interface = m_engine->m_physicsSystem->GetBodyLockInterface();
    const PhysicsInterfaceWriteLock lock = PhysicsInterfaceWriteLock(binding.Body, interface);

    JPH::Body* body = interface.TryGetBody(binding.Body);
    switch (a_mode) 
    {
    case ForceMode_Impulse:
    {
        body->AddAngularImpulse(JPH::Vec3(a_torque.x, a_torque.y, a_torque.z));

        break;
    }
    default:
    {
        body->AddTorque(JPH::Vec3(a_torque.x, a_torque.y, a_torque.z));
        
        break;
    }
    }
}

uint32_t PhysicsEngineBindings::CreateTriggerBody(uint32_t a_transformAddr, uint32_t a_colliderAddr) const
{
    ICARIAN_ASSERT_MSG(a_colliderAddr < m_engine->m_collisionShapes.Size(), "CreateTriggerBody out of bounds");

    const glm::mat4 globalTransform = ObjectManager::GetGlobalMatrix(a_transformAddr);

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

bool PhysicsEngineBindings::GetObjectLayerCollision(uint32_t a_lhs, uint32_t a_rhs) const
{
    return m_engine->CanObjectLayersCollide(a_lhs, a_rhs);
}
void PhysicsEngineBindings::SetObjectLayerCollision(uint32_t a_lhs, uint32_t a_rhs, bool a_state) const
{
    IVERIFY(a_lhs < 8);
    IVERIFY(a_rhs < 8);

    ITOGGLEBIT(a_state, m_engine->m_objectLayerCollisions[a_lhs], a_rhs);
    // Not sure if I should toggle on both sides may change later just doing it for now
    ITOGGLEBIT(a_state, m_engine->m_objectLayerCollisions[a_rhs], a_lhs);
}

RaycastResultBuffer* PhysicsEngineBindings::Raycast(const glm::vec3& a_pos, const glm::vec3& a_dir, uint32_t* a_resultCount) const
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

        RaycastResultBuffer* results = new RaycastResultBuffer[*a_resultCount];
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

    return NULL;
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
