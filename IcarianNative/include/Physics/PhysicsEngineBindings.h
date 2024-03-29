#pragma once

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include <cstdint>
#include <Jolt/Core/Core.h>

class PhysicsEngine;

#include "Runtime/RuntimeManager.h"

struct RaycastResult
{
    glm::vec3 Position;
    uint32_t BodyAddr;
};

enum e_ForceMode : uint16_t
{
    ForceMode_Force = 0,
    ForceMode_Acceleration = 1,
    ForceMode_Impulse = 2
};

class PhysicsEngineBindings
{
private:
    PhysicsEngine*  m_engine;

    void AddBody(JPH::uint32 a_id, uint32_t a_index) const;
    
protected:

public:
    PhysicsEngineBindings(PhysicsEngine* a_engine);
    ~PhysicsEngineBindings();

    uint32_t CreateSphereShape(float a_radius) const;
    float GetSphereShapeRadius(uint32_t a_addr) const;

    uint32_t CreateBoxShape(const glm::vec3& a_extents) const;
    glm::vec3 GetBoxShapeExtents(uint32_t a_addr) const;

    uint32_t CreateCapsuleShape(float a_height, float a_radius) const;
    float GetCapsuleShapeHeight(uint32_t a_addr) const;
    float GetCasuleShapeRadius(uint32_t a_addr) const;

    uint32_t CreateCylinderShape(float a_height, float a_radius) const;
    float GetCylinderShapeHeight(uint32_t a_addr) const;
    float GetCylinderShapeRadius(uint32_t a_addr) const;

    void DestroyCollisionShape(uint32_t a_addr) const;

    uint32_t CreatePhysicsBody(uint32_t a_transformAddr, uint32_t a_colliderAddr) const;
    void DestroyPhysicsBody(uint32_t a_addr) const;

    void SetPhysicsBodyPosition(uint32_t a_addr, const glm::vec3& a_pos) const;
    void SetPhysicsBodyRotation(uint32_t a_addr, const glm::quat& a_rot) const;

    uint32_t CreateRigidBody(uint32_t a_transformAddr, uint32_t a_colliderAddr, float a_mass) const;
    glm::vec3 GetRigidBodyVelocity(uint32_t a_addr) const;

    void SetRigidBodyVelocity(uint32_t a_addr, const glm::vec3& a_velocity) const;
    glm::vec3 GetRigidBodyAngularVelocity(uint32_t a_addr) const;
    void SetRigidBodyAngularVelocity(uint32_t a_addr, const glm::vec3& a_velocity) const;
    void RigidBodyAddForce(uint32_t a_addr, const glm::vec3& a_force, e_ForceMode a_mode) const;
    void RigidBodyAddTorque(uint32_t a_addr, const glm::vec3& a_torque, e_ForceMode a_mode) const;

    uint32_t CreateTriggerBody(uint32_t a_transformAddr, uint32_t a_colliderAddr) const;

    void SetGravity(const glm::vec3& a_gravity) const;
    glm::vec3 GetGravity() const;

    RaycastResult* Raycast(const glm::vec3& a_pos, const glm::vec3& a_dir, uint32_t* a_resultCount) const;
    MonoArray* Raycast(const glm::vec3& a_pos, const glm::vec3& a_dir) const;

    uint32_t* SphereCollision(const glm::vec3& a_pos, float a_radius, uint32_t* a_resultCount) const;
    MonoArray* SphereCollision(const glm::vec3& a_pos, float a_radius) const;

    uint32_t* BoxCollision(const glm::mat4& a_transform, const glm::vec3& a_extents, uint32_t* a_resultCount) const;
    MonoArray* BoxCollision(MonoArray* a_transform, const glm::vec3& a_extents) const;

    uint32_t* AABBCollision(const glm::vec3& a_min, const glm::vec3& a_max, uint32_t* a_resultCount) const;
    MonoArray* AABBCollision(const glm::vec3& a_min, const glm::vec3& a_max) const;
};