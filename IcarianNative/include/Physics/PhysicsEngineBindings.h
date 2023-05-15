#pragma once

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
    void DestroySphereShape(uint32_t a_addr) const;
    float GetSphereShapeRadius(uint32_t a_addr) const;

    uint32_t CreatePhysicsBody(uint32_t a_transformAddr, uint32_t a_colliderAddr) const;
    void DestroyPhysicsBody(uint32_t a_addr) const;

    uint32_t CreateRigidBody(uint32_t a_transformAddr, uint32_t a_colliderAddr, float a_mass) const;
    void DestroyRigidBody(uint32_t a_addr);
};