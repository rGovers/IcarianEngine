#pragma once

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include <Jolt/Jolt.h>

#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>
#include <Jolt/Physics/PhysicsSystem.h>

// Dunno if I will regret this but trying Jolt over Bullet
class PhysicsEngine
{
private:
    static constexpr uint32_t MaxBodies = 65535;
    static constexpr uint32_t MaxContactConstraints = 1024 * 10;

    JPH::BroadPhaseLayerInterface*      m_broadPhase;
    JPH::ObjectVsBroadPhaseLayerFilter* m_objectBroad;
    JPH::ObjectLayerPairFilter*         m_pairFilter;

    JPH::PhysicsSystem                  m_physicsSystem;

protected:

public:
    PhysicsEngine();
    ~PhysicsEngine();
};