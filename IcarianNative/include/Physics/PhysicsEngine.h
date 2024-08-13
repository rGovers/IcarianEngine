// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include <Jolt/Jolt.h>

#include <cstdint>
#include <Jolt/Core/Core.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Physics/Body/Body.h>
#include <Jolt/Physics/Body/BodyID.h>
#include <Jolt/Physics/Character/CharacterVirtual.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <unordered_map>

#include "DataTypes/TNCArray.h"
#include "Physics/IcBodyActivationListener.h"
#include "Physics/IcBroadPhaseLayerInterface.h"
#include "Physics/IcCharacterListener.h"
#include "Physics/IcContactListener.h"
#include "Physics/IcObjectVsBroadPhaseLayerFilter.h"
#include "Physics/IcObjectLayerPairFilter.h"
#include "Physics/IcPhysicsJobSystem.h"

class Config;
class PhysicsEngineBindings;
class RuntimeFunction;

struct BodyBinding
{
    uint32_t TransformAddr;
    JPH::BodyID Body;
};

// Dunno if I will regret this but trying Jolt over Bullet
class PhysicsEngine
{
public:
    static constexpr JPH::BroadPhaseLayer BroadphaseNonMoving = JPH::BroadPhaseLayer(0);
    static constexpr JPH::BroadPhaseLayer BroadphaseTrigger = JPH::BroadPhaseLayer(1);
    static constexpr JPH::BroadPhaseLayer BroadphaseMoving = JPH::BroadPhaseLayer(2);
    static constexpr JPH::ObjectLayer LayerTrigger = JPH::ObjectLayer(6);
    static constexpr JPH::ObjectLayer LayerNonMoving = JPH::ObjectLayer(7);

private:
    friend class PhysicsEngineBindings;

    static constexpr uint32_t MaxBodies = 65535;
    static constexpr uint32_t MaxContactConstraints = 1024 * 10;
    static constexpr uint32_t AllocatorSize = 1024 * 1024 * 10;

    PhysicsEngineBindings*                    m_runtimeBindings;

    // Apparently Intel decided no fun allowed so array instead of uint64_t
    uint8_t                                   m_objectLayerCollisions[8];

    double                                    m_fixedTimePassed;
    double                                    m_fixedTimeStep;
    double                                    m_fixedTimeTimer;

    RuntimeFunction*                          m_fixedUpdateFunction;

    // FFS got foot gunned by RAII. Raw pointers it is then.
    IcPhysicsJobSystem*                       m_jobSystem;
    IcBroadPhaseLayerInterface*               m_broadPhase;
    IcObjectVsBroadPhaseLayerFilter*          m_objectBroad;
    IcObjectLayerPairFilter*                  m_pairFilter;
    
    IcBodyActivationListener*                 m_activationListener;
    IcContactListener*                        m_contactListener;
    IcCharacterListener*                      m_characterListener;

    JPH::TempAllocatorImpl*                   m_allocator;

    JPH::PhysicsSystem*                       m_physicsSystem;

    SharedSpinLock                            m_bodyMapLock;
    std::unordered_map<JPH::uint32, uint32_t> m_bodyMap;

    TNCArray<JPH::ShapeSettings::ShapeResult> m_collisionShapes;
    TNCArray<BodyBinding>                     m_bodyBindings;
    TNCArray<JPH::CharacterVirtual*>          m_characters;

protected:

public:
    PhysicsEngine(Config* a_config);
    ~PhysicsEngine();

    bool CanObjectLayersCollide(uint32_t a_lhs, uint32_t a_rhs) const;

    void Update(double a_delta);

    uint32_t GetBodyAddr(JPH::uint32 a_joltIndex);
};

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