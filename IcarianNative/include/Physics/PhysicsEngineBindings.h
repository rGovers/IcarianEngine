#pragma once

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include <cstdint>

class PhysicsEngine;
class RuntimeManager;

class PhysicsEngineBindings
{
private:
    PhysicsEngine* m_engine;

protected:

public:
    PhysicsEngineBindings(PhysicsEngine* a_engine, RuntimeManager* a_runtime);
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

    uint32_t CreateRigidBody(uint32_t a_transformAddr, uint32_t a_colliderAddr, float a_mass) const;
    void DestroyRigidBody(uint32_t a_addr);
};