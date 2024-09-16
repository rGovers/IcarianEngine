// Icarian Engine - C# Game Engine
// 
// License at end of file.

#include "Physics/PhysicsEngineBindings.h"

#include <Jolt/Jolt.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
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
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
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
#include "Core/IcarianDefer.h"
#include "Core/IcarianError.h"
#include "Core/StringUtils.h"
#include "FileCache.h"
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
#include "EngineCharacterControllerInterop.h"
#include "EngineMeshCollisionShapeInterop.h"
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
ENGINE_MESHCOLLISIONSHAPE_EXPORT_TABLE(RUNTIME_FUNCTION_DEFINITION);
ENGINE_SPHERECOLLISIONSHAPE_EXPORT_TABLE(RUNTIME_FUNCTION_DEFINITION);

ENGINE_CHARACTERCONTROLLER_EXPORT_TABLE(RUNTIME_FUNCTION_DEFINITION);

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
    ENGINE_MESHCOLLISIONSHAPE_EXPORT_TABLE(RUNTIME_FUNCTION_ATTACH);
    ENGINE_SPHERECOLLISIONSHAPE_EXPORT_TABLE(RUNTIME_FUNCTION_ATTACH);

    ENGINE_CHARACTERCONTROLLER_EXPORT_TABLE(RUNTIME_FUNCTION_ATTACH);

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

    if (result.HasError())
    {
        const JPH::String str = result.GetError();
        IERROR(std::string("Jolt: ") + str.c_str());

        return -1;
    }
    IVERIFY(result.IsValid());

    return m_engine->m_collisionShapes.PushVal(result);
}
float PhysicsEngineBindings::GetSphereShapeRadius(uint32_t a_addr) const
{
    IVERIFY(m_engine->m_collisionShapes.Exists(a_addr));

    const JPH::ShapeRefC shape = m_engine->m_collisionShapes[a_addr].Get();
    IVERIFY(shape->GetSubType() == JPH::EShapeSubType::Sphere);

    return shape->GetInnerRadius();
}

uint32_t PhysicsEngineBindings::CreateBoxShape(const glm::vec3& a_extents) const
{
    TRACE("Creating Plane Shape");
    const glm::vec3 halfExtents = a_extents * 0.5f;

    const JPH::BoxShapeSettings boxSettings = JPH::BoxShapeSettings(JPH::RVec3(halfExtents.x, halfExtents.y, halfExtents.z));
    const JPH::ShapeSettings::ShapeResult result = boxSettings.Create();

    if (result.HasError())
    {
        const JPH::String str = result.GetError();
        IERROR(std::string("Jolt: ") + str.c_str());

        return -1;
    }
    IVERIFY(result.IsValid());

    return m_engine->m_collisionShapes.PushVal(result);
}
glm::vec3 PhysicsEngineBindings::GetBoxShapeExtents(uint32_t a_addr) const
{
    IVERIFY(m_engine->m_collisionShapes.Exists(a_addr));

    const JPH::ShapeRefC shape = m_engine->m_collisionShapes[a_addr].Get();
    IVERIFY(shape->GetSubType() == JPH::EShapeSubType::Box);
    
    const JPH::BoxShape* bShape = (JPH::BoxShape*)shape.GetPtr();

    const JPH::RVec3 val = bShape->GetHalfExtent();

    return glm::vec3(val.GetX(), val.GetY(), val.GetZ()) * 2.0f;
}

uint32_t PhysicsEngineBindings::CreateCapsuleShape(float a_height, float a_radius) const
{
    TRACE("Creating Capsule Shape");
    const JPH::CapsuleShapeSettings capsuleSettings = JPH::CapsuleShapeSettings(a_height * 0.5f, a_radius);
    const JPH::ShapeSettings::ShapeResult result = capsuleSettings.Create();

    if (result.HasError())
    {
        const JPH::String str = result.GetError();
        IERROR(std::string("Jolt: ") + str.c_str());

        return -1;
    }
    IVERIFY(result.IsValid());

    return m_engine->m_collisionShapes.PushVal(result);
}
float PhysicsEngineBindings::GetCapsuleShapeHeight(uint32_t a_addr) const
{
    IVERIFY(m_engine->m_collisionShapes.Exists(a_addr));

    const JPH::ShapeRefC shape = m_engine->m_collisionShapes[a_addr].Get();
    IVERIFY(shape->GetSubType() == JPH::EShapeSubType::Capsule);

    const JPH::CapsuleShape* cShape = (JPH::CapsuleShape*)shape.GetPtr();

    return cShape->GetHalfHeightOfCylinder() * 2.0f;
}
float PhysicsEngineBindings::GetCasuleShapeRadius(uint32_t a_addr) const
{
    IVERIFY(m_engine->m_collisionShapes.Exists(a_addr));

    const JPH::ShapeRefC shape = m_engine->m_collisionShapes[a_addr].Get();
    IVERIFY(shape->GetSubType() == JPH::EShapeSubType::Capsule);

    const JPH::CapsuleShape* cShape = (JPH::CapsuleShape*)shape.GetPtr();

    return cShape->GetRadius();
}

uint32_t PhysicsEngineBindings::CreateCylinderShape(float a_height, float a_radius) const
{
    TRACE("Creating Cylinder Shape");
    const JPH::CylinderShapeSettings cylinderSettings = JPH::CylinderShapeSettings(a_height * 0.5f, a_radius);
    const JPH::ShapeSettings::ShapeResult result = cylinderSettings.Create();

    if (result.HasError())
    {
        const JPH::String str = result.GetError();
        IERROR(std::string("Jolt: ") + str.c_str());

        return -1;
    }
    IVERIFY(result.IsValid());

    return m_engine->m_collisionShapes.PushVal(result);
}
float PhysicsEngineBindings::GetCylinderShapeHeight(uint32_t a_addr) const
{
    IVERIFY(m_engine->m_collisionShapes.Exists(a_addr));

    const JPH::ShapeRefC shape = m_engine->m_collisionShapes[a_addr].Get();
    IVERIFY(shape->GetSubType() == JPH::EShapeSubType::Cylinder);

    const JPH::CylinderShape* cShape = (JPH::CylinderShape*)shape.GetPtr();

    return cShape->GetHalfHeight() * 2.0f;
}
float PhysicsEngineBindings::GetCylinderShapeRadius(uint32_t a_addr) const
{
    IVERIFY(m_engine->m_collisionShapes.Exists(a_addr));

    const JPH::ShapeRefC shape = m_engine->m_collisionShapes[a_addr].Get();
    IVERIFY(shape->GetSubType() == JPH::EShapeSubType::Cylinder);

    const JPH::CylinderShape* cShape = (JPH::CylinderShape*)shape.GetPtr();

    return cShape->GetRadius();
}

uint32_t PhysicsEngineBindings::CreateMeshShape(const std::filesystem::path& a_path) const
{
    IERRBLOCK;

    TRACE("Creating Mesh Shape");

    const std::filesystem::path ext = a_path.extension();
    const std::string extStr = ext.string();

    switch (StringHash<uint32_t>(extStr.c_str()))
    {
    case StringHash<uint32_t>(".obj"):
    case StringHash<uint32_t>(".dae"):
    case StringHash<uint32_t>(".fbx"):
    case StringHash<uint32_t>(".glb"):
    case StringHash<uint32_t>(".gltf"):
    {
        FileHandle* handle = FileCache::LoadFile(a_path);
        IERRCHECKRET(handle != nullptr, -1);
        IDEFER(delete handle);

        const uint64_t size = handle->GetSize();
        uint8_t* dat = new uint8_t[size];
        IDEFER(delete[] dat);
        IERRCHECKRET(handle->Read(dat, size) == size, -1);

        Assimp::Importer importer;

        const aiScene* scene = importer.ReadFileFromMemory(dat, (size_t)size, aiProcess_Triangulate | aiProcess_PreTransformVertices, extStr.c_str() + 1);
        IERRCHECKRET(scene != nullptr, -1);

        JPH::IndexedTriangleList faces;
        JPH::VertexList vertices;
        for (uint32_t i = 0; i < scene->mNumMeshes; ++i)
        {
            const aiMesh* mesh = scene->mMeshes[i];

            const uint32_t vertexCount = (uint32_t)mesh->mNumVertices;
            vertices.reserve(vertices.size() + vertexCount);

            for (uint32_t i = 0; i < vertexCount; ++i)
            {
                const aiVector3D& pos = mesh->mVertices[i];

                const JPH::Float3 vert = JPH::Float3(pos.x, -pos.y, pos.z);
                vertices.push_back(vert);
            }

            const uint32_t faceCount = (uint32_t)mesh->mNumFaces;
            faces.reserve(faces.size() + faceCount);

            for (uint32_t i = 0; i < faceCount; ++i)
            {
                const aiFace& face = mesh->mFaces[i];

                const JPH::IndexedTriangle tri = JPH::IndexedTriangle(face.mIndices[0], face.mIndices[2], face.mIndices[1]);
                faces.emplace_back(tri);
            }
        }

        const JPH::MeshShapeSettings meshSettings = JPH::MeshShapeSettings(vertices, faces);
        const JPH::ShapeSettings::ShapeResult result = meshSettings.Create();

        if (result.HasError())
        {
            const JPH::String str = result.GetError();
            IERROR(std::string("Jolt: ") + str.c_str());

            return -1;
        }
        IVERIFY(result.IsValid());

        return m_engine->m_collisionShapes.PushVal(result);
    }
    default:
    {
        break;
    }
    }

    return -1;
}

void PhysicsEngineBindings::DestroyCollisionShape(uint32_t a_addr) const
{
    IVERIFY(m_engine->m_collisionShapes.Exists(a_addr));

    m_engine->m_collisionShapes[a_addr].Clear();
}

uint32_t PhysicsEngineBindings::CreateCharacterController(uint32_t a_transformAddr, uint32_t a_colliderAddr, const glm::vec3& a_up, float a_slopeAngle, float a_mass) const
{
    TRACE("Creating Character Controller");
    IVERIFY(m_engine->m_collisionShapes.Exists(a_colliderAddr));

    const glm::mat4 globalTransform = ObjectManager::GetGlobalMatrix(a_transformAddr);

    glm::vec3 translation;
    glm::quat rotation;
    glm::vec3 scale;
    glm::vec3 s;
    glm::vec4 p;
    glm::decompose(globalTransform, scale, rotation, translation, s, p);

    JPH::CharacterVirtualSettings settings = JPH::CharacterVirtualSettings();
    settings.mMass = a_mass;
    settings.mMaxSlopeAngle = a_slopeAngle;
    settings.mUp = JPH::Vec3(a_up.x, a_up.y, a_up.z);
    settings.mShape = m_engine->m_collisionShapes[a_colliderAddr].Get();

    JPH::CharacterVirtual* character = new JPH::CharacterVirtual
    (
        &settings, 
        JPH::RVec3Arg(translation.x, translation.y, translation.z), 
        JPH::QuatArg(rotation.x, rotation.y, rotation.z, rotation.w), 
        m_engine->m_physicsSystem
    );

    const uint32_t index = m_engine->m_characters.PushVal(character);

    character->SetListener(m_engine->m_characterListener);
    character->SetUserData((JPH::uint64)index << 32 | (JPH::uint64)a_transformAddr);

    return index;
}
void PhysicsEngineBindings::DestroyCharacterController(uint32_t a_addr) const
{
    IVERIFY(m_engine->m_characters.Exists(a_addr));

    const JPH::CharacterVirtual* character = m_engine->m_characters[a_addr];
    IDEFER(delete character);
    m_engine->m_characters.Erase(a_addr);
}
glm::vec3 PhysicsEngineBindings::GetCharacterControllerVelocity(uint32_t a_addr) const
{
    IVERIFY(m_engine->m_characters.Exists(a_addr));

    const JPH::CharacterVirtual* character = m_engine->m_characters[a_addr];
    const JPH::Vec3 vel = character->GetLinearVelocity();

    return glm::vec3(vel.GetX(), vel.GetY(), vel.GetZ());
}
void PhysicsEngineBindings::SetCharacterControllerVelocity(uint32_t a_addr, const glm::vec3& a_velocity) const
{
    IVERIFY(m_engine->m_characters.Exists(a_addr));

    JPH::CharacterVirtual* character = m_engine->m_characters[a_addr];
    character->SetLinearVelocity(JPH::Vec3Arg(a_velocity.x, a_velocity.y, a_velocity.z));
}

void PhysicsEngineBindings::AddBody(JPH::uint32 a_id, uint32_t a_index) const
{
    const ThreadGuard g = ThreadGuard(m_engine->m_bodyMapLock);

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
    IVERIFY(m_engine->m_collisionShapes.Exists(a_colliderAddr));

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

    const BodyBinding binding = 
    {
        .TransformAddr = a_transformAddr,
        .Body = id
    };
    const uint32_t index = m_engine->m_bodyBindings.PushVal(binding);

    AddBody(id.GetIndex(), index);

    return index;
}
void PhysicsEngineBindings::DestroyPhysicsBody(uint32_t a_addr) const
{
    IVERIFY(m_engine->m_bodyBindings.Exists(a_addr));

    const BodyBinding binding = m_engine->m_bodyBindings[a_addr];
    m_engine->m_bodyBindings.Erase(a_addr);

    JPH::BodyInterface& interface = m_engine->m_physicsSystem->GetBodyInterface();
    interface.RemoveBody(binding.Body);
    interface.DestroyBody(binding.Body);
}
void PhysicsEngineBindings::SetPhysicsBodyPosition(uint32_t a_addr, const glm::vec3& a_pos) const
{
    IVERIFY(m_engine->m_bodyBindings.Exists(a_addr));

    const BodyBinding binding = m_engine->m_bodyBindings[a_addr];

    JPH::BodyInterface& interface = m_engine->m_physicsSystem->GetBodyInterface();

    interface.SetPosition(binding.Body, JPH::Vec3(a_pos.x, a_pos.y, a_pos.z), JPH::EActivation::Activate);
}
glm::vec3 PhysicsEngineBindings::GetPhysicsBodyPosition(uint32_t a_addr) const
{
    IVERIFY(m_engine->m_bodyBindings.Exists(a_addr));

    const BodyBinding binding = m_engine->m_bodyBindings[a_addr];

    const JPH::BodyInterface& interface = m_engine->m_physicsSystem->GetBodyInterface();

    const JPH::RVec3 pos = interface.GetPosition(binding.Body);

    const TransformBuffer buffer = ObjectManager::GetTransformBuffer(binding.TransformAddr);

    glm::mat4 invMat = glm::identity<glm::mat4>();
    if (buffer.ParentAddr != -1)
    {
        const glm::mat4 transformMat = ObjectManager::GetGlobalMatrix(buffer.ParentAddr);
        invMat = glm::inverse(transformMat);
    }

    return (invMat * glm::vec4(pos.GetX(), pos.GetY(), pos.GetZ(), 1.0f)).xyz();
}
void PhysicsEngineBindings::SetPhysicsBodyRotation(uint32_t a_addr, const glm::quat& a_rot) const
{
    IVERIFY(m_engine->m_bodyBindings.Exists(a_addr));

    const BodyBinding binding = m_engine->m_bodyBindings[a_addr];

    JPH::BodyInterface& interface = m_engine->m_physicsSystem->GetBodyInterface();

    interface.SetRotation(binding.Body, JPH::Quat(a_rot.x, a_rot.y, a_rot.z, a_rot.w), JPH::EActivation::Activate);
}
glm::quat PhysicsEngineBindings::GetPhysicsBodyRotation(uint32_t a_addr) const
{
    IVERIFY(m_engine->m_bodyBindings.Exists(a_addr));

    const BodyBinding binding = m_engine->m_bodyBindings[a_addr];

    const JPH::BodyInterface& interface = m_engine->m_physicsSystem->GetBodyInterface();

    const JPH::Quat rot = interface.GetRotation(binding.Body);

    const TransformBuffer buffer = ObjectManager::GetTransformBuffer(binding.TransformAddr);

    glm::quat invQuat = glm::identity<glm::quat>();
    if (buffer.ParentAddr != -1)
    {
        const glm::mat4 transformMat = ObjectManager::GetGlobalMatrix(buffer.ParentAddr);
        const glm::mat4 inv = glm::inverse(transformMat);

        glm::vec3 t;
        glm::vec3 s;
        glm::vec3 sk;
        glm::vec4 p;
        glm::decompose(inv, s, invQuat, t, sk, p);
    }

    return glm::quat(rot.GetX(), rot.GetY(), rot.GetZ(), rot.GetW()) * invQuat;
}

uint32_t PhysicsEngineBindings::CreateRigidBody(uint32_t a_transformAddr, uint32_t a_colliderAddr, uint32_t a_layer, float a_mass) const
{
    IVERIFY(m_engine->m_collisionShapes.Exists(a_colliderAddr));
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
    // Should probably make this a setting and default to discrete but for now cast
    bodySettings.mMotionQuality = JPH::EMotionQuality::LinearCast;
    bodySettings.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateInertia;

    JPH::BodyInterface& interface = m_engine->m_physicsSystem->GetBodyInterface();
    const JPH::BodyID id = interface.CreateAndAddBody(bodySettings, JPH::EActivation::Activate);
    IDEFER(m_engine->m_activationListener->OnBodyActivated(id, 0));

    const BodyBinding binding = 
    {
        .TransformAddr = a_transformAddr, 
        .Body = id
    };
    const uint32_t index = m_engine->m_bodyBindings.PushVal(binding);

    AddBody(id.GetIndex(), index);

    return index;
}
void PhysicsEngineBindings::SetRigidBodyGravityFactor(uint32_t a_addr, float a_factor) const
{
    IVERIFY(m_engine->m_bodyBindings.Exists(a_addr));

    const BodyBinding binding = m_engine->m_bodyBindings[a_addr];

    const JPH::BodyLockInterfaceLocking& interface = m_engine->m_physicsSystem->GetBodyLockInterface();
    const PhysicsInterfaceWriteLock lock = PhysicsInterfaceWriteLock(binding.Body, interface);

    JPH::Body* body = interface.TryGetBody(binding.Body);

    JPH::MotionProperties* properties = body->GetMotionProperties();
    properties->SetGravityFactor(a_factor);
}
float PhysicsEngineBindings::GetRigidBodyGravityFactor(uint32_t a_addr) const
{
    IVERIFY(m_engine->m_bodyBindings.Exists(a_addr));

    const BodyBinding binding = m_engine->m_bodyBindings[a_addr];

    const JPH::BodyLockInterfaceLocking& interface = m_engine->m_physicsSystem->GetBodyLockInterface();
    const PhysicsInterfaceReadLock lock = PhysicsInterfaceReadLock(binding.Body, interface);

    const JPH::Body* body = interface.TryGetBody(binding.Body);

    const JPH::MotionProperties* properties = body->GetMotionProperties();
    return properties->GetGravityFactor();
}
glm::vec3 PhysicsEngineBindings::GetRigidBodyVelocity(uint32_t a_addr) const
{
    IVERIFY(m_engine->m_bodyBindings.Exists(a_addr));

    const BodyBinding binding = m_engine->m_bodyBindings[a_addr];

    const JPH::BodyLockInterfaceLocking& interface = m_engine->m_physicsSystem->GetBodyLockInterface();
    const PhysicsInterfaceReadLock lock = PhysicsInterfaceReadLock(binding.Body, interface);

    const JPH::Body* body = interface.TryGetBody(binding.Body);

    const JPH::Vec3 vel = body->GetLinearVelocity();

    return glm::vec3(vel.GetX(), vel.GetY(), vel.GetZ());
}
void PhysicsEngineBindings::SetRigidBodyVelocity(uint32_t a_addr, const glm::vec3& a_velocity) const
{
    IVERIFY(m_engine->m_bodyBindings.Exists(a_addr));

    const BodyBinding binding = m_engine->m_bodyBindings[a_addr];

    const JPH::BodyLockInterfaceLocking& interface = m_engine->m_physicsSystem->GetBodyLockInterface();
    const PhysicsInterfaceWriteLock lock = PhysicsInterfaceWriteLock(binding.Body, interface);

    JPH::Body* body = interface.TryGetBody(binding.Body);
    body->SetLinearVelocity(JPH::Vec3(a_velocity.x, a_velocity.y, a_velocity.z));
}
glm::vec3 PhysicsEngineBindings::GetRigidBodyAngularVelocity(uint32_t a_addr) const
{
    IVERIFY(m_engine->m_bodyBindings.Exists(a_addr));

    const BodyBinding binding = m_engine->m_bodyBindings[a_addr];

    const JPH::BodyLockInterfaceLocking& interface = m_engine->m_physicsSystem->GetBodyLockInterface();
    const PhysicsInterfaceReadLock lock = PhysicsInterfaceReadLock(binding.Body, interface);

    const JPH::Body* body = interface.TryGetBody(binding.Body);

    const JPH::Vec3 vel = body->GetAngularVelocity();

    return glm::vec3(vel.GetX(), vel.GetY(), vel.GetZ());
}
void PhysicsEngineBindings::SetRigidBodyAngularVelocity(uint32_t a_addr, const glm::vec3& a_velocity) const
{
    IVERIFY(m_engine->m_bodyBindings.Exists(a_addr));

    const BodyBinding binding = m_engine->m_bodyBindings[a_addr];

    const JPH::BodyLockInterfaceLocking& interface = m_engine->m_physicsSystem->GetBodyLockInterface();
    const PhysicsInterfaceWriteLock lock = PhysicsInterfaceWriteLock(binding.Body, interface);

    JPH::Body* body = interface.TryGetBody(binding.Body);
    body->SetAngularVelocity(JPH::Vec3(a_velocity.x, a_velocity.y, a_velocity.z));
}
void PhysicsEngineBindings::RigidBodyAddForce(uint32_t a_addr, const glm::vec3& a_force, e_ForceMode a_mode) const
{
    IVERIFY(m_engine->m_bodyBindings.Exists(a_addr));

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
    IVERIFY(m_engine->m_bodyBindings.Exists(a_addr));

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
    IVERIFY(m_engine->m_collisionShapes.Exists(a_colliderAddr));

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
        JPH::EMotionType::Static,
        PhysicsEngine::LayerTrigger
    );
    bodySettings.mIsSensor = true;

    JPH::BodyInterface& interface = m_engine->m_physicsSystem->GetBodyInterface();
    const JPH::BodyID id = interface.CreateAndAddBody(bodySettings, JPH::EActivation::DontActivate);

    const BodyBinding binding = 
    {
        .TransformAddr = a_transformAddr,
        .Body = id
    };
    const uint32_t index = m_engine->m_bodyBindings.PushVal(binding);

    AddBody(id.GetIndex(), index);

    return index;
}

void PhysicsEngineBindings::SetGravity(const glm::vec3& a_gravity) const
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

    const JPH::NarrowPhaseQuery& narrow = m_engine->m_physicsSystem->GetNarrowPhaseQuery();

    JPH::RRayCast ray;
    ray.mOrigin = JPH::Vec3(a_pos.x, a_pos.y, a_pos.z);
    ray.mDirection = JPH::Vec3(a_dir.x, a_dir.y, a_dir.z);

    constexpr JPH::RayCastSettings Settings;

    class RayCollector : public JPH::CastRayCollector
    {
    public:
        Array<JPH::RayCastResult> Results;

        virtual void AddHit(const JPH::RayCastResult& a_result)
        {
            // Had a air jump bug so all trust in Jolt is gone
            if (a_result.mFraction < 0.0f || a_result.mFraction > 1.0f)
            {
                return;
            }

            Results.Push(a_result);
        }
    } collector{ };

    narrow.CastRay(ray, Settings, collector);

    if (!collector.Results.Empty())
    {
        *a_resultCount = collector.Results.Size();
        RaycastResultBuffer* results = new RaycastResultBuffer[*a_resultCount];

        const JPH::BodyInterface& interface = m_engine->m_physicsSystem->GetBodyInterface();

        for (uint32_t i = 0; i < *a_resultCount; ++i)
        {
            const JPH::RayCastResult& res = collector.Results[i];

            const JPH::Vec3 pos = ray.GetPointOnRay(res.mFraction);

            JPH::RefConst<JPH::Shape> shape = interface.GetShape(res.mBodyID);

            const JPH::RMat44 mat = interface.GetWorldTransform(res.mBodyID);
            const JPH::RMat44 invMat = mat.Inversed();
            const JPH::RMat44 rot = mat.GetRotation();
            const JPH::Vec3 normal = rot * shape->GetSurfaceNormal(res.mSubShapeID2, invMat * pos);

            results[i].Fraction = res.mFraction;
            results[i].Position = glm::vec3(pos.GetX(), pos.GetY(), pos.GetZ());
            results[i].Normal = glm::vec3(normal.GetX(), normal.GetY(), normal.GetZ());
            results[i].BodyAddr = m_engine->GetBodyAddr(res.mBodyID.GetIndex());
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

// MIT License
// 
// Copyright (c) 2024 River Govers
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.