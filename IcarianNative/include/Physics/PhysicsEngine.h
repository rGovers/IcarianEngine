#pragma once

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include <Jolt/Jolt.h>

#include <cstdint>
#include <Jolt/Core/Core.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Physics/Body/Body.h>
#include <Jolt/Physics/Body/BodyID.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <shared_mutex>
#include <unordered_map>

#include "DataTypes/TArray.h"
#include "Physics/IcBodyActivationListener.h"
#include "Physics/IcBroadPhaseLayerInterface.h"
#include "Physics/IcContactListener.h"
#include "Physics/IcObjectVsBroadPhaseLayerFilter.h"
#include "Physics/IcObjectLayerPairFilter.h"
#include "Physics/IcPhysicsJobSystem.h"

class Config;
class ObjectManager;
class PhysicsEngineBindings;
class RuntimeFunction;
class RuntimeManager;

struct BodyBinding
{
    uint32_t TransformAddr;

    JPH::BodyID Body;

    BodyBinding(uint32_t a_transform = -1, JPH::BodyID a_body = JPH::BodyID())
    {
        TransformAddr = a_transform;
        Body = a_body;
    }  
};

// Dunno if I will regret this but trying Jolt over Bullet
class PhysicsEngine
{
private:
    friend class PhysicsEngineBindings;

    static constexpr uint32_t MaxBodies = 65535;
    static constexpr uint32_t MaxContactConstraints = 1024 * 10;
    static constexpr uint32_t AllocatorSize = 1024 * 1024 * 10;

    double                                    m_fixedTimePassed;
    double                                    m_fixedTimeStep;
    double                                    m_fixedTimeTimer;

    ObjectManager*                            m_objectManager;

    RuntimeFunction*                          m_fixedUpdateFunction;

    // FFS got foot gunned by RAII. Raw pointers it is then.
    IcPhysicsJobSystem*                       m_jobSystem;
    IcBroadPhaseLayerInterface*               m_broadPhase;
    IcObjectVsBroadPhaseLayerFilter*          m_objectBroad;
    IcObjectLayerPairFilter*                  m_pairFilter;
    
    IcBodyActivationListener*                 m_activationListener;
    IcContactListener*                        m_contactListener;

    JPH::TempAllocatorImpl*                   m_allocator;

    JPH::PhysicsSystem*                       m_physicsSystem;

    PhysicsEngineBindings*                    m_runtimeBindings;

    std::shared_mutex                         m_mapMutex;
    std::unordered_map<JPH::uint32, uint32_t> m_bodyMap;

    TArray<JPH::ShapeSettings::ShapeResult>   m_collisionShapes;
    TArray<BodyBinding>                       m_bodyBindings;

protected:

public:
    PhysicsEngine(Config* a_config, RuntimeManager* a_runtime, ObjectManager* a_objectManager);
    ~PhysicsEngine();

    void Update(double a_delta);

    uint32_t GetBodyAddr(JPH::uint32 a_joltIndex);

    static constexpr JPH::BroadPhaseLayer BroadphaseNonMoving = JPH::BroadPhaseLayer(0);
    static constexpr JPH::BroadPhaseLayer BroadphaseMoving = JPH::BroadPhaseLayer(1);
    static constexpr JPH::BroadPhaseLayer BroadphaseTrigger = JPH::BroadPhaseLayer(2);
    static constexpr JPH::ObjectLayer LayerNonMoving = JPH::ObjectLayer(0);
    static constexpr JPH::ObjectLayer LayerMoving = JPH::ObjectLayer(1);
    static constexpr JPH::ObjectLayer LayerTrigger = JPH::ObjectLayer(2);
};